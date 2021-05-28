#include "api.h"
#include "hashTable.h"
#include "anthony.h"
#include <stdio.h>
#include <string.h>

#define DEBUG 1
#define TOKEN_MAX_LEN 1000
#define MAX_ANSWER_LEN 10050

#include "anthony.h"


int n_mails, n_queries;
mail *mails;
query *queries;

// initialization
int answer[MAX_ANSWER_LEN], answerLen;

int main (void) {
	// debug config
	char verbose = DEBUG;
	int test_queries_n = 5;
	int test_queries[5] = {5, 7, 24, 28, 30};
	int test_queryType = find_similar;
	
	api.init(&n_mails, &n_queries, &mails, &queries);
	
	HashTable* hashTables[n_mails];
	hashTables_init(hashTables, n_mails);
	for (int mail_ctr; mail_ctr<n_mails; mail_ctr++){
		hashTable_hashmail(hashTables[mail_ctr], mails[mail_ctr]);
	}

	initTokensFromMails();
	
	if (verbose){
		fprintf(stderr, "==== DEBUG MODE INFO ====\n");
		fprintf(stderr, "test query type: %d\n", test_queryType); 
		fprintf(stderr, "test qid:"); 
		for (int i=0; i<test_queries_n; i++)
			fprintf(stderr, " %d", test_queries[i]);
		fprintf(stderr, "\ntotal received: n_mails: %d, n_queries: %d\n", n_mails, n_queries);
		fprintf(stderr, "==== DEBUG MODE INFO END ====\n\n");
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
			if (verbose && test_queryType == expression_match)
				fprintf(stderr, "\tparam: expression: %s\n", queries[i].data.expression_match_data.expression);
			answerLen = 0;
            expressionMatch(queries[i].data.expression_match_data.expression);
	        api.answer(queries[i].id, NULL, 0);
		}

		if (queries[i].type == find_similar){
			if (verbose && test_queryType == find_similar)
				fprintf(stderr, "\tparam: mid: %d, thres: %lf\n", queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
			int *ans_arr = (int *) malloc(sizeof(int)*n_mails);
			int ans_len; // = findSimilar_solve(ans_arr, mails, hashTables, queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold, verbose);
			api.answer(queries[i].id, ans_arr, ans_len);
			free(ans_arr);
		}
			
		if (queries[i].type == group_analyse){
			if (verbose && test_queryType == group_analyse){
				fprintf(stderr, "\tparam: len: %d\n\t       mids: ", queries[i].data.group_analyse_data.len);
				char indentPrefix[20] = "\t             ";
				int newlineSize = 10;
				for (int midctr=0; midctr<queries[i].data.group_analyse_data.len; midctr++){
					if (midctr%10 == 0 && midctr != 0)
						fprintf(stderr, "%s", indentPrefix);
					fprintf(stderr, "%d ", queries[i].data.group_analyse_data.mids[midctr]);
					if (midctr%10 == 9 || midctr == queries[i].data.group_analyse_data.len-1)
						fprintf(stderr, "\n");
				}
			}
		}

		if (verbose && queries[i].type == test_queryType)
			fprintf(stderr, "\n");
	}

	return 0;
}
