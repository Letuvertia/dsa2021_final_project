#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define HASH_M 100
#define HASH_S 1
#define TOKENS_MAX_N 3500 // mid 9771 (start from 0) has the most tokens of 3416.
#define S_ENLARGE_RATIO 3

#define Q_RabinKarp 59650000
#define D_RabinKarp 36

// I am from the future to save you from RE.
// debug config
#define DEBUG 1

int n_mails, n_queries;
mail *mails;
query *queries;


int toNumberArray[1<<7];
void toNumber_init() {
	// 0-9 48-57
	for (int i=48; i<=57; i++)
		toNumberArray[i] = i-48;
	// A-Z 65-90
	for (int i=65; i<=90; i++)
		toNumberArray[i] = i-55;
	// a-z 97-122
	for (int i=97; i<=122; i++)
		toNumberArray[i] = i-87;
}


/*
int hashString(char *s, int len) {
    int ans = 0;
    for (int i = 0; i < len; i++) {
        ans = (ans * D_RabinKarp + toNumberArray[s[i]]) % Q_RabinKarp;
    }
    return ans;
}*/

int hashString(char *s_begin, int s_len) {
	int hash = 0;
	for (int s_ctr=0; s_ctr<s_len; s_ctr++){
		hash = s_begin[s_ctr] + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}


bool isDelimiter(char c) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
        return false;
    }
    return true;
}


// (Jun): I found hash function samples from 
// 		  http://www.cse.yorku.ca/~oz/hash.html
unsigned int hashString_inChain (char *s_begin, int s_len){
	unsigned long hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
        hash = ((hash << 5) + hash) + s_begin[s_ctr]; /* hash * 33 + c */
    }
	
	unsigned int return_hash = hash % ((unsigned int)INT_MAX*2+1); // return int
	return return_hash;
}


typedef struct HashTable {
    // chaining hash table
    // please note that this is a hashTable for ONE mail.
	unsigned char chainCapacity[HASH_M]; // using char just for saving memory
	unsigned char chainElementsN[HASH_M];
	int tokenN;
	int s_hashes[TOKENS_MAX_N]; 
	unsigned int s_hashes_inChain[TOKENS_MAX_N];
    unsigned int *chains[HASH_M];
} HashTable;


HashTable *hashTable_init(mail mail) {
	//fprintf(stderr, "hashing mail: %d", mail.id);
    HashTable *hashTable = (HashTable *) malloc(sizeof(HashTable));
    memset(hashTable->chainElementsN, 0, sizeof(unsigned char)*HASH_M);
    hashTable->tokenN = 0;
    //hashTables[mail_ctr]->chains = (unsigned int **) malloc(sizeof(unsigned int*)*HASH_M);
    /*
    for (int m_ctr=0; m_ctr<HASH_M; m_ctr++) {
        hashTables[mail_ctr]->chains[m_ctr] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
        hashTables[mail_ctr]->chainCapacity[m_ctr] = HASH_S;
        hashTables[mail_ctr]->chainElementsN[m_ctr] = 0;
    }*/
    hashTable_hashParagraph(hashTable, mail.subject);
	hashTable_hashParagraph(hashTable, mail.content);

    return hashTable;
}


void hashTable_hashParagraph(HashTable *hashTable, char *para) {
	for (int s = 0; para[s] != '\0'; s++) {
		if ((s == 0 && isDelimiter(para[s])) || (s != 0 && !(isDelimiter(para[s-1]) && !isDelimiter(para[s]))))
			continue;

		int sLen = 0;
		while (!isDelimiter(para[s+sLen])) {
			sLen++;
		}

		hashTable_pushToken(hashTable, para+s, sLen);
		s += sLen;
	}
}


void hashTable_pushToken(HashTable *hashTable, char *s_begin, int s_len) {
	//fprintf(stderr, "push Token: ");
	//for (int i=0; i<s_len; i++)
	//	fprintf(stderr, "%c", s_begin[i]);
	//fprintf(stderr, "\n");

	// check whether the token repeated
	int s_hash = hashString(s_begin, s_len) % HASH_M;
	//int s_hash = hashString(s_begin, s_len) & mod; fprintf(stderr, "%d, %d\n", hashString(s_begin, s_len), s_hash);
	unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
    fprintf(stderr, "s_hash: %d, s_hash_inChain: %u, chain N:%d\n\n", s_hash, s_hash_inChain, hashTable->chainElementsN[s_hash]);

    if (hashTable->chainElementsN[s_hash] == 0){
		hashTable->chains[s_hash] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
        //fprintf(stderr, "push Token: \n");
		hashTable->chainCapacity[s_hash] = HASH_S;
        //fprintf(stderr, "push Token: \n");
	}

    else{
        //fprintf(stderr, "s_hash: %d, s_hash_inChain: %u\n", s_hash, s_hash_inChain);
        for (int i=0; i<hashTable->chainElementsN[s_hash]; i++){
            //fprintf(stderr, "%d\n", sizeof(unsigned int));
            //fprintf(stderr, "%d %u\n", i, hashTable->chains[s_hash][i]);
            if (hashTable->chains[s_hash][i] == s_hash_inChain) {
                //fprintf(stderr, "%d %u\n", i, hashTable->chains[s_hash][i]);
                return;
            }
        }
        
        // enlarge
        if (hashTable->chainCapacity[s_hash] == hashTable->chainElementsN[s_hash]) {
            unsigned char newCapacity = (hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO > (CHAR_MAX*2+1))? 
                (CHAR_MAX*2+1) : hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO;
            hashTable->chains[s_hash] 
                = (unsigned int *) realloc(hashTable->chains[s_hash], sizeof(unsigned int)*newCapacity);
            //fprintf(stderr, "enlarge chain from %d to %d\n", hashTable->chainCapacity[s_hash], newCapacity);
            hashTable->chainCapacity[s_hash] = newCapacity;
        }
    }
        
    //fprintf(stderr, "aaapush Token: \n");
	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]] = s_hash_inChain;
	hashTable->chainElementsN[s_hash]++;
    //fprintf(stderr, "bbbpush Token: \n");
	hashTable->s_hashes[hashTable->tokenN] = s_hash;
	hashTable->s_hashes_inChain[hashTable->tokenN] = s_hash_inChain;
	hashTable->tokenN++;
    //fprintf(stderr, "cccpush Token: \n");
}


bool hashTable_findToken_inputHash(HashTable *hashTable, int s_hash, unsigned int s_hash_inChain) {
    if (hashTable->chainElementsN[s_hash] == 0)
        return 0;
    for (int s_ctr=0; s_ctr < hashTable->chainElementsN[s_hash]; s_ctr++) {
        if (hashTable->chains[s_hash][s_ctr] == s_hash_inChain)
            return 1;
    }
    return 0;
}


bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len) {
    int s_hash = hashString(s_begin, s_len) % HASH_M; 
	//int s_hash = hashString(s_begin, s_len) & (HASH_M-1);
    unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
    return hashTable_findToken_inputHash(hashTable, s_hash, s_hash_inChain);
}


int findSimilar_solve (int *ans_arr, HashTable* hashTables[], int mid, double thres){
	int ans_len = 0;
	for (int mail_ctr=0; mail_ctr < n_mails; mail_ctr++){
        fprintf(stderr, "mail: %d\n", mail_ctr);
		if (mail_ctr == mid)
			continue;
		
		/*
		fprintf(stderr, "=== MAIL %d ===\n", mid);
		fprintf(stderr, "subject: %s\n", mails[mid].subject);
		fprintf(stderr, "content: %s\n\n", mails[mid].content);
		fprintf(stderr, "s_hash_inChain: (tokenN: %d) \n", hashTables[mid]->tokenN);
		for(int i=0; i<hashTables[mid]->tokenN; i++)
			fprintf(stderr, "%u ", hashTables[mid]->s_hashes_inChain[i]);
		fprintf(stderr, "\n\n");
		fprintf(stderr, "=== MAIL %d ===\n", mail_ctr);
		fprintf(stderr, "subject: %s\n", mails[mail_ctr].subject);
		fprintf(stderr, "content: %s\n", mails[mail_ctr].content);
		fprintf(stderr, "s_hash_inChain: (tokenN: %d) \n", hashTables[mail_ctr]->tokenN);
		for(int i=0; i<hashTables[mail_ctr]->tokenN; i++)
			fprintf(stderr, "%u ", hashTables[mail_ctr]->s_hashes_inChain[i]);
		fprintf(stderr, "\n\n");
		*/
		

		// compare the less token mail with the more token one
		// 「我要叫他啟發式比較。」
		//			 - 2021, 半夜在寫扣的人
		HashTable *hashTable_lessToken;
		HashTable *hashTable_moreToken;
        //fprintf(stderr, "mail: %d\n", mail_ctr);
		if (hashTables[mid]->tokenN > hashTables[mail_ctr]){
			hashTable_lessToken = hashTables[mail_ctr];
			hashTable_moreToken = hashTables[mid];
		}
		else{
			hashTable_lessToken = hashTables[mid];
			hashTable_moreToken = hashTables[mail_ctr];
		}
    
        //fprintf(stderr, "tokenN: %d\n", hashTable_moreToken->tokenN);
		double union_count = hashTable_moreToken->tokenN;
		double intersec_count = 0;
		for (int token_ctr=0; token_ctr < hashTable_lessToken->tokenN; token_ctr++){
            //fprintf(stderr, "token: %d\n", token_ctr);
			int find = hashTable_findToken_inputHash(hashTable_moreToken, hashTable_lessToken->s_hashes[token_ctr], hashTable_lessToken->s_hashes_inChain[token_ctr]);
			//fprintf(stderr, "check token s_hash_inChain: %u. Find: %d\n", hashTable_lessToken->s_hashes_inChain[token_ctr], find);

			if (find)
				intersec_count++;
			else
				union_count++;
		}

		double similarity = intersec_count / union_count;
		//fprintf(stderr, "compare mail %d and mail %d. u: %lf, i:%lf, similarity: %lf\n", mid, mail_ctr, union_count, intersec_count, similarity);
		if (similarity > thres)
			ans_arr[ans_len++] = mail_ctr;
	}

	return ans_len;
}


int main (void) {
	// Initialization
	toNumber_init();
	api.init(&n_mails, &n_queries, &mails, &queries);
    HashTable* hashTables[n_mails];
    for (int mail_ctr = 0; mail_ctr < n_mails; mail_ctr++) {
        fprintf(stderr, "mail id: %d\n", mail_ctr);
        hashTables[mail_ctr] = hashTable_init(mails[mail_ctr]);
    }
        
    /*
	hashTables_init(hashTables, n_mails);
	for (int mail_ctr = 0; mail_ctr < n_mails; mail_ctr++){
		fprintf(stderr, "mail id: %d\n", mail_ctr);
        hashTable_hashmail(hashTables[mail_ctr], mails[mail_ctr]);
	}*/

	// DEBUG MODE OFF
	if (!DEBUG){
		for(int i = 0; i < n_mails; i++){
			if (queries[i].type == find_similar){
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = findSimilar_solve(ans_arr, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				api.answer(queries[i].id, ans_arr, ans_len);
				free(ans_arr);
			}
		}
	}
	
	
	// DEBUG MODE ON
	else{
		char verbose = DEBUG;
		
		// 1.1 Test specified queries by QID
		// test queries: find_similar = {5, 7, 24, 28, 30}
		//               expression_match = {1, 4, 6, 8, 10}
		int testedQueries_n = 1; // the # of the tested queries
		int testedQueries[] = {702}; // the qid of the tested queries
		
		// 1.2 Test specified queries by type
		// set -1 if you wanna test all
		// set -2 if you wanna test by qid
		int testAllQueries = find_similar;
		if (testAllQueries >= -1)
			testedQueries_n = n_mails;
		
		
		// 2) Output the answer arr and check answer with test_env/testdata/test.ans
		bool outputAns = 1;
		FILE *outFile = NULL;
		if (outputAns)
			outFile = fopen("output.txt", "w");

		
		if (verbose){
			fprintf(stderr, "==== DEBUG MODE INFO ====\n");
			fprintf(stderr, "total received: n_mails: %d, n_queries: %d\n", n_mails, n_queries);
			if (testAllQueries >= -1){
				switch (testAllQueries) {
					case expression_match:
						fprintf(stderr, "tested query type: expression_match\n"); 
						break;
					case find_similar:
						fprintf(stderr, "tested query type: find_similar\n"); 
						break;
					case group_analyse:
						fprintf(stderr, "tested query type: group_analyse\n"); 
						break;
					case -1:
						fprintf(stderr, "tested query type: All\n"); 
						break;
					default:
						break;
				}
			}
			
			else{
				fprintf(stderr, "tested QIDs:");
				for (int i=0; i<testedQueries_n; i++)
					fprintf(stderr, " %d", testedQueries[i]);
				fprintf(stderr, "\n"); 
			}
			fprintf(stderr, "==== DEBUG MODE INFO END ====\n\n");
		}


		// start processing queries
		for(int ctr = 0; ctr < testedQueries_n; ctr++){
			int i;
			if (testAllQueries == -2) // test by QID
				i = testedQueries[ctr];
			else if (testAllQueries == -1) // test All
				i = ctr;
			else if (testAllQueries == queries[ctr].type)
				i = ctr;
			else
				continue;

            //fprintf(stderr, "qid:%d", i);
			
			if (verbose){
				switch (queries[i].type){
					case expression_match:
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "expression_match");
						break;
					case find_similar:
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "find_similar");
						break;
					case group_analyse:
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "group_analyse");
						break;
					default:
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "none of the three types. *BUGS ALERT*");
						break;
				}	
			}

			if (queries[i].type == find_similar){
				if (verbose)
					fprintf(stderr, "\tparam: mid: %d, thres: %lf\n", queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = findSimilar_solve(ans_arr, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				// api.answer(queries[i].id, ans_arr, ans_len);
				if (outputAns){
					fprintf(outFile, "%d:", i);
					for (int ans_ctr=0; ans_ctr<ans_len; ans_ctr++)
						fprintf(outFile, " %d", ans_arr[ans_ctr]);
					fprintf(outFile, "\n");
				}
				free(ans_arr);
			}

			if (verbose)
				fprintf(stderr, "\n");
		}

		if (outputAns)
			fclose(outFile);
	}
	
	return 0;
}