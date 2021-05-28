#include "api.h"
#include "tools.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define HASH_M 50000 // unsigned long (4 bytes) * HASH_M= memory used for one mail
#define HASH_S 1
#define S_ENLARGE_RATIO 3


typedef struct HashTable {
    // chaining hash table
    // please note that this is a hashTable for ONE mail.
	unsigned int **chains;
	unsigned char chainCapacity[HASH_M]; // using char just for saving memory
	unsigned char chainElementsN[HASH_M];
} HashTable;


void hashTables_init(HashTable* hashTable[], int n_mails);
void hashTable_hashParagraph(HashTable *hashTable, char *para);
void hashTable_pushToken(HashTable *hashTable, char *s_begin, int s_len);
bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len);
bool hashTable_findToken_inputHash(HashTable *hashTable, int s_hash, unsigned int s_hash_inChain);


void hashTables_init(HashTable *hashTable[], int n_mails) {
	// this function initialize a HashTable pointer of n mails.
	for (int mail_ctr=0; mail_ctr<n_mails; mail_ctr++){
		hashTable[mail_ctr] = (HashTable *) malloc(sizeof(HashTable));
		hashTable[mail_ctr]->chains = (unsigned int **) malloc(sizeof(unsigned int*)*HASH_M);
		for (int m_ctr=0; m_ctr<HASH_M; m_ctr++) {
			hashTable[mail_ctr]->chains[m_ctr] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
			hashTable[mail_ctr]->chainCapacity[m_ctr] = HASH_S;
			hashTable[mail_ctr]->chainElementsN[m_ctr] = 0;
		}
	}
}


void hashTable_hashmail(HashTable* hashTable, mail mail) {
	hashTable_hashParagraph(hashTable, mail.subject);
	hashTable_hashParagraph(hashTable, mail.content);
}


void hashTable_hashParagraph(HashTable *hashTable, char *para) {
	// copied from "void initTokensFromString(TokenList* tokenList, char *T)" in
	// anthony.h
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
	int s_hash = hashString(s_begin, s_len) % HASH_M;
	if (hashTable->chainCapacity[s_hash] == hashTable->chainElementsN[s_hash]) {
		// enlarge
		unsigned char newCapacity = (hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO > (CHAR_MAX*2+1))? 
			(CHAR_MAX*2+1) : hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO;
		hashTable->chains[s_hash] 
			= (unsigned int *) realloc(hashTable->chains[s_hash], sizeof(unsigned int)*newCapacity);
		hashTable->chainCapacity[s_hash] = newCapacity;
	}

	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]] = hashString_inChain(s_begin, s_len);
	hashTable->chainElementsN[s_hash]++;
}


bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len) {
    int s_hash = hashString(s_begin, s_len) % HASH_M;
    unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
    hashTable_findToken_inputHash(hashTable, s_hash, s_hash_inChain);
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
