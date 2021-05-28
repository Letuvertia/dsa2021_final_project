#include "api.h"
#include <stdio.h>
#define TOKEN_MAX_LEN 1000
#define MAX_ANSWER_LEN 10050

#include "anthony.h"

// The testdata only contains the first 100 mails (mail1 ~ mail100)
// and 2000 queries for you to debug.

int n_mails, n_queries;
mail *mails;
query *queries;

int answer[MAX_ANSWER_LEN], answerLen;

int main(void) {
	api.init(&n_mails, &n_queries, &mails, &queries);

	initTokensFromMails();

	for(int i = 0; i < n_queries; i++)
	    if(queries[i].type == expression_match) {
			answerLen = 0;
            expressionMatch(queries[i].data.expression_match_data.expression);
	        api.answer(queries[i].id, NULL, 0);
		}

  return 0;
}
