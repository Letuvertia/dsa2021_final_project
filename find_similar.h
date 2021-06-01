#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Mark2: Functions definitions

int findSimilar_solve (int *ans_arr, HashTable* hashTables[], int mid, double thres){
	int ans_len = 0;
	for (int mail_ctr=0; mail_ctr < n_mails; mail_ctr++){
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
		for (int token_ctr=0; token_ctr < hashTable_lessToken->tokenN; token_ctr++){
			char find = hashTable_findToken_inputHash(hashTable_moreToken, hashTable_lessToken->s_hashes[token_ctr], hashTable_lessToken->s_hashes_inChain[token_ctr]);
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
		//fprintf(stderr, "if over thres: %d", (similarity>thres)?1:0);
	}

	return ans_len;
}