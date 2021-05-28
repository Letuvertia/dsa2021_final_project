#include "api.h"
#include <stdio.h>
#include <string.h>

int n_mails, n_queries;
mail *mails;
query *queries;

#include "expression_match.h"
#include "group_analyze.h"
#include "find_similar.h"

#define DEBUG 1


int main (void) {
	// debug config
	char verbose = DEBUG;
	int test_queries_n = 1;
	// test queries: find_similar = {5, 7, 24, 28, 30}
	//               expression_match = {1, 4, 6, 8, 10}
	int test_queries[1] = {1};
	int test_queryType = expression_match;
	
	if (verbose)
		fprintf(stderr, "fetch mails......\n");
	api.init(&n_mails, &n_queries, &mails, &queries);
	if (verbose){
		fprintf(stderr, "==== DEBUG MODE INFO ====\n");
		switch (test_queryType) {
		case expression_match:
			fprintf(stderr, "test query type: expression_match\n"); 
			break;
		case find_similar:
			fprintf(stderr, "test query type: find_similar\n"); 
			break;
		case group_analyse:
			fprintf(stderr, "test query type: group_analyse\n"); 
			break;
		default:
			break;
		}

		fprintf(stderr, "test qid:"); 
		for (int i=0; i<test_queries_n; i++)
			fprintf(stderr, " %d", test_queries[i]);
		fprintf(stderr, "\ntotal received: n_mails: %d, n_queries: %d\n", n_mails, n_queries);
		fprintf(stderr, "==== DEBUG MODE INFO END ====\n\n");
	}
	

	if (verbose)
		fprintf(stderr, "hash mails into their own hashTable......\n");
	HashTable* hashTables[n_mails];
	hashTables_init(hashTables, n_mails);
	for (int mail_ctr; mail_ctr<1; mail_ctr++){
		hashTable_hashmail(hashTables[mail_ctr], mails[mail_ctr]);
	}
		

	// start processing queries
	for(int ctr = 0; ctr < test_queries_n; ctr++){
		int i = test_queries[ctr];
		
		if (verbose){
			char queryType[20];
			switch (queries[i].type){
				case expression_match:
					if (test_queryType == expression_match)
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "expression_match");
					break;
				case find_similar:
					if (test_queryType == find_similar)
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "find_similar");
					break;
				case group_analyse:
					if (test_queryType == group_analyse)
						fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
							queries[i].id, queries[i].reward, "group_analyse");
					break;
				default:
					fprintf(stderr, "QID: %d, Reward: %lf, Type: %s\n", 
						queries[i].id, queries[i].reward, "none of the three types. *BUGS ALERT*");
					break;
			}	
		}
		
		
		if (queries[i].type == expression_match){
			fprintf(stderr, "fdsafsa\n");
			int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
			int ans_len = expressionMatch(queries[i].data.expression_match_data.expression, ans_arr, hashTables);
			for (int i=0; i<ans_len; i++)
				fprintf(stderr, "%d ", ans_arr[i]);
			api.answer(queries[i].id, ans_arr, ans_len);
			free(ans_arr);
		}

		if (queries[i].type == find_similar){
			/*
			if (verbose && test_queryType == find_similar)
				fprintf(stderr, "\tparam: mid: %d, thres: %lf\n", queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
			*/
			int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
			int ans_len = 1; // = findSimilar_solve(ans_arr, mails, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold, verbose);
			api.answer(queries[i].id, ans_arr, ans_len);
			free(ans_arr);
		}
			
		if (queries[i].type == group_analyse){
			int *GA_ans = GA(queries[i].data.group_analyse_data.len,queries[i].data.group_analyse_data.mids,mails);
			api.answer(queries[i].id, GA_ans, 2);
		}

	}

	return 0;
}
