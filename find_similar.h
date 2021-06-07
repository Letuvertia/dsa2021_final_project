#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Mark1: Functions declarations 

int findSimilar_solve (int *ans_arr, HashTable* hashTables[], int mid, double thres);

// Mark2: Functions definitions

int findSimilar_solve (int *ans_arr, HashTable* hashTables[], int mid, double thres){
	int ans_len = 0;
	unsigned short int s_ctr;
	int token_ctr;
	for (int mail_ctr=0; mail_ctr < n_mails; mail_ctr++){
		if (mail_ctr == mid)
			continue;
		
		if (mail_ctr == 4) {
			fprintf(stderr, "=== MAIL %d ===\n", mid);
			fprintf(stderr, "subject: %s\n", mails[mid].subject);
			fprintf(stderr, "content: %s\n\n", mails[mid].content);
			fprintf(stderr, "s_hash_inChain: (tokenN: %d) \n", hashTables[mid]->tokenN);
			for(int i=0; i<hashTables[mid]->tokenN; i++)
				fprintf(stderr, "\"%s\", ", hashTables[mid]->str[i]);
			fprintf(stderr, "\n\n");
			fprintf(stderr, "=== MAIL %d ===\n", mail_ctr);
			fprintf(stderr, "subject: %s\n", mails[mail_ctr].subject);
			fprintf(stderr, "content: %s\n", mails[mail_ctr].content);
			fprintf(stderr, "s_hash_inChain: (tokenN: %d) \n", hashTables[mail_ctr]->tokenN);
			for(int i=0; i<hashTables[mail_ctr]->tokenN; i++)
				fprintf(stderr, "\"%s\", ", hashTables[mail_ctr]->str[i]);
			fprintf(stderr, "\n\n");
		}
		
		
		

		// compare the less token mail with the more token one
		// 「我要叫他啟發式比較。」
		//			 - 2021, 半夜在寫扣的人
		HashTable *hashTable_lessToken;
		HashTable *hashTable_moreToken;
		if (hashTables[mid]->tokenN > hashTables[mail_ctr]->tokenN){
			hashTable_lessToken = hashTables[mail_ctr];
			hashTable_moreToken = hashTables[mid];
		}
		else{
			hashTable_lessToken = hashTables[mid];
			hashTable_moreToken = hashTables[mail_ctr];
		}
	
		double union_count = hashTable_moreToken->tokenN;
		double intersec_count = 0;
		for (token_ctr=0; token_ctr < hashTable_lessToken->tokenN; token_ctr++){
			if (mail_ctr == 4){
				unsigned short s_hash = hashTable_lessToken->s_hashes[token_ctr];
				unsigned int s_hash_inChain = hashTable_lessToken->s_hashes_inChain[token_ctr];
				fprintf(stderr, "check s_hash: %hu, s_hash_inChain: %u; ChainN: %hu; capa: %hu\n", s_hash, s_hash_inChain, hashTable_moreToken->chainElementsN[s_hash], hashTable_moreToken->chainCapacity[s_hash]);
				for (s_ctr=0; s_ctr < hashTable_moreToken->chainElementsN[s_hash]; s_ctr++) {
					fprintf(stderr, "check %s with ", hashTable_lessToken->str[token_ctr]);
					for (int tmp=0; tmp<hashTable_moreToken->tokenN; tmp++)
						if (hashTable_moreToken->s_hashes[tmp] == s_hash && hashTable_moreToken->s_hashes_inChain[tmp] == hashTable_moreToken->chains[s_hash][s_ctr])
							fprintf(stderr, "%s", hashTable_moreToken->str[tmp]);
					fprintf(stderr, "; ");
					fprintf(stderr, "check %u with %u\n", s_hash_inChain, hashTable_moreToken->chains[s_hash][s_ctr]);
					if (hashTable_moreToken->chains[s_hash][s_ctr] == s_hash_inChain)
						fprintf(stderr, "MATCH!\n");
					s_ctr++;
				}
			}
			int find = hashTable_findToken_inputHash(hashTable_moreToken, hashTable_lessToken->s_hashes[token_ctr], hashTable_lessToken->s_hashes_inChain[token_ctr]);
			
			if (find)
				intersec_count++;
			else
				union_count++;
		}

		double similarity = intersec_count / union_count;
		if (mail_ctr == 4)
			fprintf(stderr, "compare mail %d and mail %d. u: %lf, i:%lf, similarity: %lf\n", mid, mail_ctr, union_count, intersec_count, similarity);
		if (similarity > thres)
			ans_arr[ans_len++] = mail_ctr;
	}

	return ans_len;
}