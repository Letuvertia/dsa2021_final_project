#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

#define HASH_M 10000 // unsigned long (4 bytes) * HASH_M = memory used for one mail
#define HASH_S 2
#define TOKENS_MAX_N 3500 // mid 9771 (start from 0) has the most tokens of 3416.
#define S_ENLARGE_RATIO 4

#define Q_RabinKarp 59650000
#define D_RabinKarp 36

// #define INITIAL_MAIL_N 10000


// Mark1: Functions declarations
struct HashTable;
typedef struct HashTable HashTable;
bool isDelimiter(char c);
int hashString(char *s_begin, int s_len);
unsigned int hashString_inChain (char *s_begin, int s_len);
void hashTables_init(HashTable* hashTable[], int table_n);
void hashTable_hashmail(HashTable* hashTable, mail mail);  // (anthony): I add this missing declarations
void hashTable_hashParagraph(HashTable *hashTable, char *para);
void hashTable_pushToken(HashTable *hashTable, char *s_begin, int s_len); 
bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len);
bool hashTable_findToken_inputHash(HashTable *hashTable, unsigned short int s_hash, unsigned int s_hash_inChain);


// Mark2: Functions definitions

// ==== tool functions====


//(Jun): I think that there is no need to transform char to number
//       in hashString, because what hash funcitons do is to scamble
//       the bits to make it uniformly distributed. There is no diff
//       between 0 and 48 as a shift during hashing.
int toNumberArray[1<<7];
void toNumber_init() {
	// 0-9 48-57
	for (int i=48; i<=57; i++)
		toNumberArray[i] = i-48;
	// A-Z 65-90
	for (int i=65; i<=90; i++)
		toNumberArray[i] = i-55;
	// a-z 97-122
	for (int i=97; i<=122; i++)
		toNumberArray[i] = i-87;
}

char toNumber(char c) {
	return toNumberArray[c];
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


/*
// (Jun): hash functions below are from 
// 		  http://www.cse.yorku.ca/~oz/hash.html
// (Jun): just testing out some other hash functions with bit arithmetic
int hashString(char *s_begin, int s_len) {
	unsigned short int hash = 0;
	for (int s_ctr=0; s_ctr<s_len; s_ctr++){
		hash = s_begin[s_ctr] + (hash << 6) + (hash << 16) - hash;
	}
	return hash;
}*/


unsigned int hashString_inChain (char *s_begin, int s_len){
	// used "within a chain". That is, used only when different string 
	// is hashed into the same value by hashString
	unsigned int hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
        hash = ((hash << 5) + hash) + s_begin[s_ctr]; /* hash * 33 + c */
    }
	return hash;
	// unsigned int return_hash = hash % ((unsigned int)INT_MAX*2+1); // return int
	// return return_hash;
}


void printstr(char *s, int s_len) {
	for (int i=0; i<s_len; i++)
		fprintf(stderr, "%c", s[i]);
}


// ==== functions for HashTable ====
typedef struct HashTable {
	// for debug
	char str[TOKENS_MAX_N][50];
    // chaining hash table
    // please note that this is a hashTable for ONE mail.
	// unsigned short int range: 0 ~ 65,535
	int chainCapacity[HASH_M];
	int chainElementsN[HASH_M];

	int tokenN;
	// (Jun): store hashes of each token in the original order
	//        to avoid repeated hashing
	int s_hashes[TOKENS_MAX_N]; 
	unsigned int s_hashes_inChain[TOKENS_MAX_N];
	unsigned int *chains[HASH_M];
} HashTable;


void hashTables_init(HashTable *hashTables[], int table_n) {
	// this function initialize a HashTable pointer of n mails.
	for (int mail_ctr=0; mail_ctr<table_n; mail_ctr++){
		hashTables[mail_ctr] = (HashTable *) malloc(sizeof(HashTable));
		hashTables[mail_ctr]->tokenN = 0;
		// (Jun): allocate the chain when needed
		// memset(hashTables[mail_ctr]->chainElementsN, 0, sizeof(unsigned short int)*HASH_M);
		for (int i=0; i<HASH_M; i++) {
			hashTables[mail_ctr]->chains[i] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
			hashTables[mail_ctr]->chainElementsN[i] = 0;
			hashTables[mail_ctr]->chainCapacity[i] = HASH_S;
		}
	}
}


//int max_tokenN = -1;
void hashTable_hashmail(HashTable* hashTable, mail mail) {
	hashTable_hashParagraph(hashTable, mail.subject);
	hashTable_hashParagraph(hashTable, mail.content);
	//max_tokenN = (max_tokenN < hashTable->tokenN)? hashTable->tokenN: max_tokenN;
}


void hashTable_hashParagraph(HashTable *hashTable, char *para) {
	// copied from "void initTokensFromString(TokenList* tokenList, char *T)" in anthony.h
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
	//printstr(s_begin, s_len);
	//fprintf(stderr, "\n"); return;
	unsigned short int s_hash = (unsigned short int)(hashString(s_begin, s_len) % HASH_M);
	unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
	//fprintf(stderr, "hash %hu, hash_inChain %u, N: %hu\n", s_hash, s_hash_inChain, hashTable->chainElementsN[s_hash]);

	// check if the chain allocated
	if (hashTable->chainElementsN[s_hash] == 0){
		// hashTable->chains[s_hash] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
		hashTable->chainCapacity[s_hash] = HASH_S;	
	}

	else{
		// check whether the token repeated
		for (unsigned short i=0; i<hashTable->chainElementsN[s_hash]/2; i++){
			if (hashTable->chains[s_hash][2*i] == s_hash_inChain){
				/*
				fprintf(stderr, "hash %hu, hash_inChain %u, N: %hu\n", s_hash, s_hash_inChain, hashTable->chainElementsN[s_hash]);
				fprintf(stderr, "find \"%s\" repeat with \"", hashTable->str[hashTable->chains[s_hash][2*i+1]]);
				printstr(s_begin, s_len);
				fprintf(stderr, "\"\n");
				*/

				
				for (int j=0; j<s_len; j++) {
					if (s_begin[j] != hashTable->str[hashTable->chains[s_hash][2*i+1]][j]) {
						fprintf(stderr, "UNMATCH!!! same hash\n");
						fprintf(stderr, "first appear: %d\n", hashTable->chains[s_hash][2*i+1]);
						fprintf(stderr, "hash %hu, hash_inChain %u, N: %hu\n", s_hash, s_hash_inChain, hashTable->chainElementsN[s_hash]);
						fprintf(stderr, "find \"%s\" repeat with \"", hashTable->str[hashTable->chains[s_hash][2*i+1]]);
						printstr(hashTable->str[hashTable->chains[s_hash][2*i+1]], s_len);
						printstr(s_begin, s_len);
						fprintf(stderr, "\"\n\n");
						break;
					}
				}

				//fprintf(stderr, " (end)\n");
				return;
			}
		}

		//fprintf(stderr, "not repeat (end)\n");
		// enlarge
		if (hashTable->chainCapacity[s_hash] == hashTable->chainElementsN[s_hash]) {
			unsigned short int newCapacity = (hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO > TOKENS_MAX_N)? 
				TOKENS_MAX_N : hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO;
			hashTable->chains[s_hash] 
				= (unsigned int *) realloc(hashTable->chains[s_hash], sizeof(unsigned int)*newCapacity);
			//fprintf(stderr, "enlarge chain from %d to %d\n", hashTable->chainCapacity[s_hash], newCapacity);
			hashTable->chainCapacity[s_hash] = newCapacity;
		}
		//fprintf(stderr, " (end)\n");
	}

	/*
	if (hashTable->tokenN == 3500){
		printstr(hashTable->str[0], 5); fprintf(stderr, "\n");
		printstr(s_begin, s_len); fprintf(stderr, "\n");
		fprintf(stderr, "hash %hu, hash_inChain %u, N: %hu\n", s_hash, s_hash_inChain, hashTable->chainElementsN[s_hash]);
		printstr(hashTable->str[0], 5); fprintf(stderr, "\n");
		fprintf(stderr, "Hamas in table: hash_inChain: %u\n", hashTable->chains[3510][0]);
	}*/
	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]++] = s_hash_inChain;
	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]++] = (unsigned int) hashTable->tokenN;
	/*
	if (hashTable->tokenN == 3500){
		fprintf(stderr, "fdsa: ");
		printstr(hashTable->str[0], 5); fprintf(stderr, "\n");
		fprintf(stderr, "Hamas in table: hash_inChain: %u\n", hashTable->chains[3510][0]);
	}*/
		

	hashTable->s_hashes[hashTable->tokenN] = s_hash;
	hashTable->s_hashes_inChain[hashTable->tokenN] = s_hash_inChain;
	/*
	if (hashTable->tokenN == 3500){
		fprintf(stderr, "eeeeefdsa: ");
		printstr(hashTable->str[0], 5); fprintf(stderr, "\n");
	}*/
		
	for (int s=0; s<s_len; s++) {
		hashTable->str[hashTable->tokenN][s] = s_begin[s];
	}
	hashTable->str[hashTable->tokenN][s_len] = '\0';
	//fprintf(stderr, "tokenN %d: ", hashTable->tokenN); printstr(hashTable->str[0], 5); fprintf(stderr, "\n");
	/*
	char hamas[6] = "Hamas";
	for (int i=0; i<5; i++){
		if (hamas[i] != s_begin[i]){
			break;
		}
		if (i == 4){
			printstr(hashTable->str[hashTable->chains[s_hash][2*i+1]], s_len);
			exit(-1);
		}
	}*/
		
	//fprintf(stderr, "%s (end)\n", hashTable->str[hashTable->tokenN]);
	
	hashTable->tokenN++;
	
}


bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len) {
    unsigned short int s_hash = hashString(s_begin, s_len) % HASH_M;
    unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
	//fprintf(stderr, "push Token: ");
	//for (int i=0; i<s_len; i++)
	//	fprintf(stderr, "%c", s_begin[i]);
	//fprintf(stderr, "\n");
	//fprintf(stderr, "s_hash: %d, s_hash_inChain: %u\n", s_hash, s_hash_inChain);
    return hashTable_findToken_inputHash(hashTable, s_hash, s_hash_inChain);
}


bool hashTable_findToken_inputHash(HashTable *hashTable, unsigned short int s_hash, unsigned int s_hash_inChain) {
	// fprintf(stderr, "%hu, %u", s_hash, s_hash_inChain);
    if (hashTable->chainElementsN[s_hash] == 0)
        return 0;
    for (unsigned short s_ctr=0; s_ctr < hashTable->chainElementsN[s_hash]; s_ctr++) {
        if (hashTable->chains[s_hash][s_ctr] == s_hash_inChain)
        	return 1;
		s_ctr++;
    }
    return 0;
}
