#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define MOD_BIT 15
#define HASH_M 1<<MOD_BIT
#define TOKENS_MAX_N 3500 // mid 9771 (start from 0) has the most tokens of 3416.

// debug config
#define DEBUG 1

int n_mails, n_queries;
mail *mails;
query *queries;

int toNumberArray[1<<7];
void toNumber_init() {
	for (int i = 0; i < 26; i++) {
		toNumberArray['a'+i] = i;
		toNumberArray['A'+i] = i;
	}
	for (int i = 0; i < 10; i++) {
		toNumberArray['0'+i] = i+26;
	}
}

bool isDelimiter(char c) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
        return false;
    }
    return true;
}

// ref: http://www.cse.yorku.ca/~oz/hash.html
// SDBM
short hashString(char *s_begin, int s_len) {
	int hash = 0;
	for (int s_ctr=0; s_ctr<s_len; s_ctr++){
		hash = toNumberArray[s_begin[s_ctr]] + (hash << 6) + (hash << 16) - hash;
	}
	
	return hash & ((1<<MOD_BIT) - 1);
}

// quadratic; c1, c2
#define C1 3
#define C2 6
short hashString_quaprobe(short s_hash, int k) {
	// wiki: for m = 2^n, good choice for c_1 and c_2 are 1/2
	return (s_hash + (k + k*k)/2) & ((1<<MOD_BIT) - 1);
	//return (s_hash + C1*k + C2*k*k) & ((1<<MOD_BIT) - 1);
}

short hashString_doubleprobe(short s_hash, unsigned int s_hash_inbox, unsigned int k) {
	// wiki: for m = 2^n, good choice for c_1 and c_2 are 1/2
	//fprintf(stderr, "second hash: %u\n", s_hash_inbox&((1<<10)-1));
	short probe_shift = (short) k*(s_hash_inbox&((1<<10)-1));
	//fprintf(stderr, "second hash shift: %d\n", (s_hash + probe_shift));
	return (s_hash + probe_shift) & ((1<<MOD_BIT) - 1);
}


// ref: http://www.cse.yorku.ca/~oz/hash.html
// djb2
unsigned int hashString_inbox (char *s_begin, int s_len){
	unsigned int hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
		// hash * 33 + c
        hash = ((hash << 5) + hash) + toNumberArray[s_begin[s_ctr]];
    }
	return hash;
}

/*
// ref: https://byvoid.com/zht/blog/string-hash-compare/
// BKDR Hash
unsigned int hashString_inbox (char *s_begin, int s_len){
	unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
	unsigned int hash = 0;
	for (int s_ctr=0; s_ctr<s_len; s_ctr++){
		// hash * 131 + c
        hash = hash * seed + toNumberArray[s_begin[s_ctr]];
    }

	return hash;
}*/

typedef struct HashTable {
	int tokenN;
	unsigned int boxes[HASH_M];
	//char boxes_ver[HASH_M][2];
	char boxes_string[HASH_M][50];
	char boxes_occupied[HASH_M]; // check if the box is occupied
	
	short s_hashes[TOKENS_MAX_N]; 
	unsigned int s_hashes_inbox[TOKENS_MAX_N];
	char *str_ptr[TOKENS_MAX_N];
	//char s_ver[TOKENS_MAX_N][2];
} HashTable;

HashTable hashTables[300];
int token_pushctr;

//bool hashTable_findToken_inputHash(int tid, short s_hash, unsigned int s_hash_inbox, char ori_s[], char s_ver[2]) {
bool hashTable_findToken_inputHash(int tid, short s_hash, unsigned int s_hash_inbox, char *ori_s) {
	//fprintf(stderr, "first probe: %u\n", s_hash);
	if (hashTables[tid].boxes_occupied[s_hash] == 0)
		return 0;
	unsigned int k=0; short tmp_hash = s_hash, probe_shift = (short) (s_hash_inbox&((1<<10)-1))+1;
	//fprintf(stderr, "s:%d, s_inbox:%u, shift:%d\n", s_hash, s_hash_inbox, probe_shift);
	while (hashTables[tid].boxes_occupied[tmp_hash] == 1 && hashTables[tid].boxes[tmp_hash] != s_hash_inbox) {
		//fprintf(stderr, "k=%d, probe: %d\n", k, tmp_hash);
		//k++; tmp_hash = hashString_doubleprobe(s_hash, s_hash_inbox, k);
		k++; tmp_hash += probe_shift & ((1<<MOD_BIT) - 1);
		//if (k>=10)
		//	exit(-1);
	}
	if (hashTables[tid].boxes_occupied[tmp_hash] == 0)
		return 0;
	
	fprintf(stderr, "match token \"%s\" with \"%s\"\n", ori_s, hashTables[tid].boxes_string[tmp_hash]);
	return 1;
	/*
	if (hashTables[tid].boxes_ver[tmp_hash][0] == s_ver[0] && hashTables[tid].boxes_ver[tmp_hash][1] == s_ver[1]){
		fprintf(stderr, "match token \"%s\" with \"%s\"\n", ori_s, hashTables[tid].boxes_string[tmp_hash]);
		return 1;
	}*/	
}

bool hashTable_findToken_inputString(int tid, char *s_begin, int s_len) {
    short s_hash = hashString(s_begin, s_len); 
    unsigned int s_hash_inbox = hashString_inbox(s_begin, s_len);
	char tmp[50], ver[2] = {s_begin[s_len-2], s_begin[s_len-1]};
    return hashTable_findToken_inputHash(tid, s_hash, s_hash_inbox, tmp);
}

void hashTable_pushToken(int tid, char *s_begin, int s_len) {
	char tmp[50];
	for (int i=0; i<s_len; i++)
		tmp[i] = s_begin[i];
	tmp[s_len] = '\0';
	//fprintf(stderr, "push Token: %s\n", tmp);

	short s_hash = hashString(s_begin, s_len); 
    unsigned int s_hash_inbox = hashString_inbox(s_begin, s_len);
	if (hashTables[tid].boxes_occupied[s_hash] == 0) {
		hashTables[tid].boxes[s_hash] = s_hash_inbox;
		hashTables[tid].boxes_occupied[s_hash] = 1;
		//hashTables[tid].boxes_ver[s_hash][0] = (s_len==1)? -1: s_begin[s_len-2];
		//hashTables[tid].boxes_ver[s_hash][1] = s_begin[s_len-1];

		hashTables[tid].s_hashes[hashTables[tid].tokenN] = s_hash;
		//hashTables[tid].s_ver[hashTables[tid].tokenN][0] = (s_len==1)? -1: s_begin[s_len-2];
		//hashTables[tid].s_ver[hashTables[tid].tokenN][1] = s_begin[s_len-1];
		hashTables[tid].str_ptr[hashTables[tid].tokenN] = hashTables[tid].boxes_string[s_hash];
		hashTables[tid].s_hashes_inbox[hashTables[tid].tokenN++] = s_hash_inbox;
		
		
		for (int i=0; i<s_len; i++)
			hashTables[tid].boxes_string[s_hash][i] = s_begin[i];
		hashTables[tid].boxes_string[s_hash][s_len] = '\0';
		//fprintf(stderr, "\tstored in box no.%d, not occupied; token_ctr: %d\n", s_hash, token_pushctr); token_pushctr++;
		//fprintf(outputfile, "%s\n", tmp);
		return;
	}
	
	unsigned int k=0; short tmp_hash = s_hash, probe_shift = (short) (s_hash_inbox&((1<<10)-1))+1;
	//fprintf(stderr, "\ts:%d, s_inbox:%u, shift:%d\n", s_hash, s_hash_inbox, probe_shift);
	while (hashTables[tid].boxes_occupied[tmp_hash] == 1 && hashTables[tid].boxes[tmp_hash] != s_hash_inbox) {
		//fprintf(stderr, "\tk=%d, probe: %d\n", k, tmp_hash);
		//k++; tmp_hash = hashString_doubleprobe(s_hash, s_hash_inbox, k);
		k++; tmp_hash = (tmp_hash + probe_shift) & ((1<<MOD_BIT) - 1);
		//if (k>=10)
		//	exit(-1);
	}
	if (hashTables[tid].boxes_occupied[tmp_hash] == 0) {
		hashTables[tid].boxes[tmp_hash] = s_hash_inbox;
		hashTables[tid].boxes_occupied[tmp_hash] = 1;
		//hashTables[tid].boxes_ver[tmp_hash][0] = (s_len==1)? -1: s_begin[s_len-2];
		//hashTables[tid].boxes_ver[tmp_hash][1] = s_begin[s_len-1];

		hashTables[tid].s_hashes[hashTables[tid].tokenN] = s_hash; // original hash, not tmp_hash
		//hashTables[tid].s_ver[hashTables[tid].tokenN][0] = (s_len==1)? -1: s_begin[s_len-2];
		//hashTables[tid].s_ver[hashTables[tid].tokenN][1] = s_begin[s_len-1];
		hashTables[tid].str_ptr[hashTables[tid].tokenN] = hashTables[tid].boxes_string[tmp_hash];
		hashTables[tid].s_hashes_inbox[hashTables[tid].tokenN++] = s_hash_inbox;
		
		
		for (int i=0; i<s_len; i++)
			hashTables[tid].boxes_string[tmp_hash][i] = s_begin[i];
		hashTables[tid].boxes_string[tmp_hash][s_len] = '\0';
		//fprintf(stderr, "\tstored in box no.%d, not occupied; token_ctr: %d\n", tmp_hash, token_pushctr); token_pushctr++;
		//fprintf(outputfile, "%s\n", tmp);
		return;
	}
	
	//fprintf(stderr, "token %s repeat with %s, whose s_inbox: %u\n", tmp, hashTables[tid].boxes_string[tmp_hash], hashTables[tid].boxes[tmp_hash]);
}

void hashTable_hashParagraph(int tid, char *para) {
	for (int s = 0; para[s] != '\0'; s++) {
		if ((s == 0 && isDelimiter(para[s])) || (s != 0 && !(isDelimiter(para[s-1]) && !isDelimiter(para[s]))))
			continue;

		int sLen = 0;
		while (!isDelimiter(para[s+sLen])) {
			sLen++;
		}

		hashTable_pushToken(tid, para+s, sLen);
		s += sLen;
	}
}

void hashTables_init() {
	// (Jun): initialize all mails
	for (int mail_ctr = 0; mail_ctr < 110; mail_ctr++) {
		//fprintf(stderr, "init mail: %d\n", mail_ctr);
		hashTables[mail_ctr].tokenN = 0;
		memset(hashTables[mail_ctr].boxes_occupied, 0, sizeof(char)*HASH_M);
		//token_pushctr=0;
		//fprintf(outputfile, "&mail %d\n", mail_ctr);
		hashTable_hashParagraph(mail_ctr, mails[mail_ctr].subject);
		hashTable_hashParagraph(mail_ctr, mails[mail_ctr].content);
		//fprintf(stderr, "init mail: %d, token: %d\n", mail_ctr, hashTables[mail_ctr].tokenN);
	}
}

int findSimilar_solve (int *ans_arr, int mid, double thres){
	int ans_len = 0;
	for (int mail_ctr=34; mail_ctr < n_mails; mail_ctr++){
        //fprintf(stderr, "mail: %d\n", mail_ctr);
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
		int mid_less = mid, mid_more = mail_ctr;
		if (hashTables[mid].tokenN > hashTables[mail_ctr].tokenN){
			mid_less = mail_ctr;
			mid_more = mid;
		}

		double union_count = hashTables[mid_more].tokenN;
		double intersec_count = 0;
		int find;
		
		for (int token_ctr=0; token_ctr < hashTables[mid_less].tokenN; token_ctr++){
			//fprintf(stderr, "token ctr: %d\n", token_ctr);
			//fprintf(stderr, "token hashes: %d, %u\n", hashTables[mid_less].s_hashes[token_ctr], hashTables[mid_less].s_hashes_inbox[token_ctr]);
			//find = hashTable_findToken_inputHash(mid_more, hashTables[mid_less].s_hashes[token_ctr], hashTables[mid_less].s_hashes_inbox[token_ctr], 
			//			hashTables[mid_less].boxes_string[hashTables[mid_less].s_hashes[token_ctr]], hashTables[mid_less].boxes_ver[hashTables[mid_less].s_hashes[token_ctr]]);
			find = hashTable_findToken_inputHash(mid_more, hashTables[mid_less].s_hashes[token_ctr], hashTables[mid_less].s_hashes_inbox[token_ctr], hashTables[mid_less].str_ptr[token_ctr]);
			//find = hashTable_findToken_inputHash(mid_more, hashTables[mid_less].s_hashes[token_ctr], hashTables[mid_less].s_hashes_inbox[token_ctr]);

			if (find)
				intersec_count++;
			else
				union_count++;
		}
		fprintf(stderr, "less token id:%d, N: %d; more token id:%d, N: %d\n", mid_less, hashTables[mid_less].tokenN, mid_more, hashTables[mid_more].tokenN);

		double similarity = intersec_count / union_count;
		fprintf(stderr, "u:%lf, i:%lf, s:%lf\n", union_count, intersec_count, similarity);
		if (similarity > thres)
			ans_arr[ans_len++] = mail_ctr;
		break;
	}

	return ans_len;
}


// wrong query id
//short wrong_qid[] = {694, 1333, 1695, 2227, 3073, 3107, 3146, 3413, 3585, 3835, 4634, 4715, 4998, 5124, 5450, 6819, 6837, 7059, 7648, 7755, 8955, 9015, 9358, 9611, 9636, 9927, 9971};

int main (void) {
	// Initialization
	toNumber_init();
	// int wrongQ_mid[27] = {1035, 8292, 9576, 9326, 102, 4088, 5345, 6045, 3927, 6780, 9878, 7961, 9513, 7078, 4036, 1317, 2932, 1490, 9558, 8446, 1813, 9687, 8363, 8457, 7912, 9328, 8363};
	// short skip_mid[10010] = {0};
	// for (int i=0; i<27; i++)
	// 	skip_mid[wrongQ_mid[i]] = 1;
	int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
	
	api.init(&n_mails, &n_queries, &mails, &queries);
	//FILE *outFile = fopen("token.txt", "w");
	hashTables_init();
	//fclose(outFile); exit(-1);
        
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
				// if (skip_mid[queries[i].data.find_similar_data.mid])
				// 	continue;
				
				int ans_len = findSimilar_solve(ans_arr, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				api.answer(queries[i].id, ans_arr, ans_len);
			}
		}
		free(ans_arr);
	}
	
	
	// DEBUG MODE ON
	else{
		char verbose = DEBUG;
		
		// 1.1 Test specified queries by QID
		// test queries: find_similar = {5, 7, 24, 28, 30}
		//               expression_match = {1, 4, 6, 8, 10}
		// int testedQueries_n = 27; // the # of the tested queries
		// int testedQueries[] = {694, 1333, 1695, 2227, 3073, 3107, 3146, 3413, 3585, 3835, 4634, 4715, 4998, 5124, 5450, 6819, 6837, 7059, 7648, 7755, 8955, 9015, 9358, 9611, 9636, 9927, 9971}; // the qid of the tested queries
		int testedQueries_n = 1; // the # of the tested queries
		int testedQueries[] = {3073};


		// 1.2 Test specified queries by type
		// set -1 if you wanna test all
		// set -2 if you wanna test by qid
		int testAllQueries = -2;
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
				// wrongQ_mid[ctr] = queries[i].data.find_similar_data.mid;
				// if (skip_mid[queries[i].data.find_similar_data.mid])
				// 	continue;

				if (verbose)
					fprintf(stderr, "\tparam: mid: %d, thres: %lf\n", queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = findSimilar_solve(ans_arr, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
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

		//for (int ctr=0; ctr < testedQueries_n; ctr++) {
		//	fprintf(stderr, "%d, ", wrongQ_mid[ctr]);
		//}

		if (outputAns)
			fclose(outFile);
	}
	
	return 0;
}