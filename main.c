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


// Mark2: Functions definitions
int main (void) {
	// Initialization
	toNumber_init();
	api.init(&n_mails, &n_queries, &mails, &queries);
	HashTable* hashTables[n_mails];
	hashTables_init(hashTables, n_mails);
	//hashTable_hashmail(hashTables[3320], mails[3320]); 
	for (int mail_ctr = 0; mail_ctr < n_mails; mail_ctr++){
		fprintf(stderr, "%d\n", mail_ctr);
		hashTable_hashmail(hashTables[mail_ctr], mails[mail_ctr]);
	}

	// DEBUG MODE OFF
	if (!DEBUG){
		for(int i = 0; i < n_queries; i++){  // (anthony): n_queries insteads of n_mails.
			// Mark3-0: call expression_match
			if (queries[i].type == expression_match){
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = expressionMatch(queries[i].data.expression_match_data.expression, ans_arr, hashTables);
				api.answer(queries[i].id, ans_arr, ans_len);
				free(ans_arr);
			}
			
			// Mark3-1: call find_similar
			if (queries[i].type == find_similar){
				int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
				int ans_len = findSimilar_solve(ans_arr, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
				api.answer(queries[i].id, ans_arr, ans_len);
				free(ans_arr);
			}
			
			// Mark3-2: call group_analyse
			if (queries[i].type == group_analyse){
				int *ans_arr = (int *) malloc(sizeof(int)*2);
				ans_arr = GA(queries[i].data.group_analyse_data.len,queries[i].data.group_analyse_data.mids,mails);
				api.answer(queries[i].id, ans_arr, 2);
			}

			// Mark3-4: end call
		}
	}
	
	
	// DEBUG MODE ON
	else{
		char verbose = DEBUG;
		
		// 1.1 Test specified queries by QID
		// test queries: find_similar = {5, 7, 24, 28, 30}
		//               expression_match = {1, 4, 6, 8, 10}
		int testedQueries_n = 1; // the # of the tested queries
		int testedQueries[] = {28}; // the qid of the tested queries
		
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
				
			if (queries[i].type == group_analyse){
				// print your debug info in your functions if needed
				int *ans_arr = (int *) malloc(sizeof(int)*2);
				ans_arr = GA(queries[i].data.group_analyse_data.len,queries[i].data.group_analyse_data.mids,mails);
				api.answer(queries[i].id, ans_arr, 2);
				if (outputAns){
					fprintf(outFile, "%d:", i);
					for (int ans_ctr=0; ans_ctr<2; ans_ctr++)
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
