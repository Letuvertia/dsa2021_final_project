#include "hashTable.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>


int findSimilar_solve (int *ans_arr, mail *mails, HashTable* hashTable, int mid, double thres, char verbose){
	if (verbose){
		fprintf(stderr, "mid: %d\n", mails[mid].id);
		fprintf(stderr, "from: %s\n", mails[mid].from);
		fprintf(stderr, "to: %s\n", mails[mid].to);
		fprintf(stderr, "subject: %s\n", mails[mid].subject);
		fprintf(stderr, "content: %s\n", mails[mid].content);
	}

	// hash subject
	return 0;
}