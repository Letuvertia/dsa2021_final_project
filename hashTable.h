#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define HASH_M 100 // unsigned long (4 bytes) * HASH_M= memory used for one mail
#define HASH_S 1
#define S_ENLARGE_RATIO 3

#define Q_RabinKarp 59650000
#define D_RabinKarp 36

int toNumberArray[1<<7];

// ======= tools =======

void toNumber_init() {
	toNumberArray['0'] = 0;
	toNumberArray['1'] = 1;
	toNumberArray['2'] = 2;
	toNumberArray['3'] = 3;
	toNumberArray['4'] = 4;
	toNumberArray['5'] = 5;
	toNumberArray['6'] = 6;
	toNumberArray['7'] = 7;
	toNumberArray['8'] = 8;
	toNumberArray['9'] = 9;

	toNumberArray['a'] = 10;
	toNumberArray['b'] = 11;
	toNumberArray['c'] = 12;
	toNumberArray['d'] = 13;
	toNumberArray['e'] = 14;
	toNumberArray['f'] = 15;
	toNumberArray['g'] = 16;
	toNumberArray['h'] = 17;
	toNumberArray['i'] = 18;
	toNumberArray['j'] = 19;
	toNumberArray['k'] = 20;
	toNumberArray['l'] = 21;
	toNumberArray['m'] = 22;
	toNumberArray['n'] = 23;
	toNumberArray['o'] = 24;
	toNumberArray['p'] = 25;
	toNumberArray['q'] = 26;
	toNumberArray['r'] = 27;
	toNumberArray['s'] = 28;
	toNumberArray['t'] = 29;
	toNumberArray['u'] = 30;
	toNumberArray['v'] = 31;
	toNumberArray['w'] = 32;
	toNumberArray['x'] = 33;
	toNumberArray['y'] = 34;
	toNumberArray['z'] = 35;
	
	toNumberArray['A'] = 10;
	toNumberArray['B'] = 11;
	toNumberArray['C'] = 12;
	toNumberArray['D'] = 13;
	toNumberArray['E'] = 14;
	toNumberArray['F'] = 15;
	toNumberArray['G'] = 16;
	toNumberArray['H'] = 17;
	toNumberArray['I'] = 18;
	toNumberArray['J'] = 19;
	toNumberArray['K'] = 20;
	toNumberArray['L'] = 21;
	toNumberArray['M'] = 22;
	toNumberArray['N'] = 23;
	toNumberArray['O'] = 24;
	toNumberArray['P'] = 25;
	toNumberArray['Q'] = 26;
	toNumberArray['R'] = 27;
	toNumberArray['S'] = 28;
	toNumberArray['T'] = 29;
	toNumberArray['U'] = 30;
	toNumberArray['V'] = 31;
	toNumberArray['W'] = 32;
	toNumberArray['X'] = 33;
	toNumberArray['Y'] = 34;
	toNumberArray['Z'] = 35;
}

char toNumber(char c) {
	return toNumberArray[c];
    // if ('0' <= c && c <= '9') {
    //     return c - '0';
    // }
    // if ('A' <= c && c <= 'Z') {
    //     return c - 'A' + 10;
    // }
    // return c - 'a' + 10;
}


int isAlphaNumeric (char c){
	// 0-9 48-57
	if (48 <= c && c <= 57)
		return 1;
	// A-Z 65-90
	if (65 <= c && c <= 90)
		return 1;
	// a-z 97-122
	if (97 <= c && c <= 122)
		return 1;
	return 0;
}


bool isDelimiter(char c) {
    if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')) {
        return false;
    }
    return true;
}


int hashString(char *s, int len) {
    int ans = 0;
    for (int i = 0; i < len; i++) {
        ans = (ans * D_RabinKarp + toNumber(s[i])) % Q_RabinKarp;
    }
    return ans;
}


// (Jun): I found hash function samples from 
// 		  http://www.cse.yorku.ca/~oz/hash.html
unsigned int hashString_inChain (char *s_begin, int s_len){
	// used "within a chain". That is, used only when different string 
	// is hashed into the same value by stringHash (written in anthony.h) 
	unsigned long hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
        hash = ((hash << 5) + hash) + toNumber(s_begin[s_ctr]); /* hash * 33 + c */
    }
	
	unsigned int return_hash = hash % ((unsigned int)INT_MAX*2+1); // return int
	return return_hash;
}




// ======= hash table functions =======

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
	//fprintf(stderr, "push Token: ");
	//for (int i=0; i<s_len; i++)
	//	fprintf(stderr, "%c", s_begin[i]);
	//fprintf(stderr, "\n");

	// check whether the token repeated
	int s_hash = hashString(s_begin, s_len) % HASH_M;
	unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
	for (int i=0; i<hashTable->chainElementsN[s_hash]; i++)
		if (hashTable->chains[s_hash][i] == s_hash_inChain){
			//fprintf(stderr, "repeated\n");
			return;
		}

	// enlarge
	if (hashTable->chainCapacity[s_hash] == hashTable->chainElementsN[s_hash]) {
		unsigned char newCapacity = (hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO > (CHAR_MAX*2+1))? 
			(CHAR_MAX*2+1) : hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO;
		hashTable->chains[s_hash] 
			= (unsigned int *) realloc(hashTable->chains[s_hash], sizeof(unsigned int)*newCapacity);
		//fprintf(stderr, "enlarge chain from %d to %d\n", hashTable->chainCapacity[s_hash], newCapacity);
		hashTable->chainCapacity[s_hash] = newCapacity;
	}

	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]] = s_hash_inChain;
	hashTable->chainElementsN[s_hash]++;
	//fprintf(stderr, "s_hash: %d, s_hash_inChain: %u\n\n", s_hash, s_hash_inChain);
}


bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len) {
    int s_hash = hashString(s_begin, s_len) % HASH_M;
    unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
	//fprintf(stderr, "push Token: ");
	//for (int i=0; i<s_len; i++)
	//	fprintf(stderr, "%c", s_begin[i]);
	//fprintf(stderr, "\n");
	//fprintf(stderr, "s_hash: %d, s_hash_inChain: %u\n", s_hash, s_hash_inChain);
    return hashTable_findToken_inputHash(hashTable, s_hash, s_hash_inChain);
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
