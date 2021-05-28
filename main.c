#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int n_mails, n_queries;
mail *mails;
query *queries;

#include "expression_match.h"
#include "group_analyze.h"
#include "find_similar.h"

// debug config
#define DEBUG 1


int main (void) {
	
	// Initialization
	api.init(&n_mails, &n_queries, &mails, &queries);
	HashTable* hashTables[n_mails];
	hashTables_init(hashTables, n_mails);
	for (int mail_ctr; mail_ctr<1; mail_ctr++){
		hashTable_hashmail(hashTables[mail_ctr], mails[mail_ctr]);
	}
	

	// DEBUG MODE OFF
	if (!DEBUG){
		for(int i = 0; i < n_mails; i++){
			if (queries[i].type == expression_match){
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = expressionMatch(queries[i].data.expression_match_data.expression, ans_arr, hashTables);
				api.answer(queries[i].id, ans_arr, ans_len);
				free(ans_arr);
			}

			if (queries[i].type == find_similar){
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
	}
	
	
	// DEBUG MODE ON
	else{
		char verbose = DEBUG;
		
		// 1.1 Test specified queries by QID
		// test queries: find_similar = {5, 7, 24, 28, 30}
		//               expression_match = {1, 4, 6, 8, 10}
		int testedQueries_n = 5; // the # of the tested queries
		int testedQueries[] = {1, 4, 6, 8, 10}; // the qid of the tested queries
		
		// 1.2 Test specified queries by type
		// set -1 if you wanna test all
		// set -2 if you wanna test by qid
		int testAllQueries = group_analyse;
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
			
			
			if (queries[i].type == expression_match){
				// print your debug info in your functions if needed
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = expressionMatch(queries[i].data.expression_match_data.expression, ans_arr, hashTables);
				api.answer(queries[i].id, ans_arr, ans_len);
				if (outputAns){
					fprintf(outFile, "%d:", i);
					for (int ans_ctr=0; ans_ctr<ans_len; ans_ctr++)
						fprintf(outFile, " %d", ans_arr[ans_ctr]);
					fprintf(outFile, "\n");
				}
				free(ans_arr);
			}

			if (queries[i].type == find_similar){
				if (verbose)
					fprintf(stderr, "\tparam: mid: %d, thres: %lf\n", queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = 0; // = findSimilar_solve(ans_arr, mails, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold, verbose);
				api.answer(queries[i].id, ans_arr, ans_len);
				if (outputAns){
					fprintf(outFile, "%d:", i);
					for (int ans_ctr=0; ans_ctr<ans_len; ans_ctr++)
						fprintf(outFile, " %d", ans_arr[ans_ctr]);
					fprintf(outFile, "\n");
				}
				free(ans_arr);
			}
				
			if (queries[i].type == group_analyse){
				// print your debug info in your functions if needed
				int *GA_ans = GA(queries[i].data.group_analyse_data.len,queries[i].data.group_analyse_data.mids,mails);
				api.answer(queries[i].id, GA_ans, 2);
				if (outputAns){
					fprintf(outFile, "%d:", i);
					for (int ans_ctr=0; ans_ctr<2; ans_ctr++)
						fprintf(outFile, " %d", GA_ans[ans_ctr]);
					fprintf(outFile, "\n");
				}
			}

			if (verbose)
				fprintf(stderr, "\n");
		}

		if (outputAns)
			fclose(outFile);
	}
	
	return 0;
}
