#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

// debug config
#define DEBUG 0

#define HASH_M 1000 // unsigned long (4 bytes) * HASH_M= memory used for one mail
#define HASH_S 1
#define TOKENS_MAX_N 3500 // mid 9771 (start from 0) has the most tokens of 3416.
#define S_ENLARGE_RATIO 2

// #define Q_RabinKarp 1 << 25
// #define D_RabinKarp 64  // aka << 6

int toNumberArray[1<<7];
int ans_arr[10010];
int n_mails, n_queries;
mail *mails;
query *queries;


// debug config
#define DEBUG 1

// ======= tools =======

void toNumber_init() {
	for (int i = 0; i < 10; i++) {
		toNumberArray['0'+i] = i;
	}
	for (int i = 0; i < 26; i++) {
		toNumberArray['a'+i] = toNumberArray['A'+i] = 10+i;
	}
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
		ans = ((ans << 6) + toNumberArray[s[i]]) & ((1 << 26) - 1);
    }
    return ans;
}

// (Jun): I found some hash function samples from 
// 		  http://www.cse.yorku.ca/~oz/hash.html
unsigned int hashString_inChain (char *s_begin, int s_len){
	// used "within a chain". That is, used only when different string 
	// is hashed into the same value by hashString
	unsigned long hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
        hash = ((hash << 5) + hash) + toNumberArray[s_begin[s_ctr]]; /* hash * 33 + c */
    }
	
	unsigned int return_hash = hash % ((unsigned int)INT_MAX*2+1); // return int
	return return_hash;
}


// ======= hash table functions =======

typedef struct HashTable {
    // chaining hash table
    // please note that this is a hashTable for ONE mail.
	unsigned int *chains[HASH_M];
	unsigned char chainCapacity[HASH_M]; // using char just for saving memory
	unsigned char chainElementsN[HASH_M];

	int tokenN;
	// (Jun): store hashes of each token in the original order
	//        to avoid repeated hashing
	int s_hashes[TOKENS_MAX_N]; 
	unsigned int s_hashes_inChain[TOKENS_MAX_N];
} HashTable;

HashTable hashTables[10010];


void hashTables_init(int n_mails);
void hashTable_hashParagraph(HashTable *hashTable, char *para);
void hashTable_pushToken(HashTable *hashTable, char *s_begin, int s_len);
bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len);
bool hashTable_findToken_inputHash(HashTable *hashTable, int s_hash, unsigned int s_hash_inChain);


void hashTables_init(int n_mails) {
	// this function initialize a HashTable pointer of n mails.
	for (int mail_ctr=0; mail_ctr<n_mails; mail_ctr++){
		memset(hashTables[mail_ctr].chainElementsN, 0, sizeof(unsigned char)*HASH_M);
		hashTables[mail_ctr].tokenN = 0;
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
	// check whether the token repeated
	int s_hash = hashString(s_begin, s_len) % HASH_M;
	unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);

	if (hashTable->chainElementsN[s_hash] == 0) {
		hashTable->chains[s_hash] = (unsigned int *) malloc(sizeof(unsigned int)*HASH_S);
		hashTable->chainCapacity[s_hash] = HASH_S;
		hashTable->chainElementsN[s_hash] = 0;
	}
	else {
		for (int i=0; i<hashTable->chainElementsN[s_hash]; i++)
			if (hashTable->chains[s_hash][i] == s_hash_inChain){
				//fprintf(stderr, "repeated\n");
				return;
			}
	}

	// enlarge
	if (hashTable->chainCapacity[s_hash] == hashTable->chainElementsN[s_hash]) {
		unsigned char newCapacity = (hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO > (CHAR_MAX*2+1))? 
			(CHAR_MAX*2+1) : hashTable->chainElementsN[s_hash] * S_ENLARGE_RATIO;
		hashTable->chains[s_hash] 
			= (unsigned int *) realloc(hashTable->chains[s_hash], sizeof(unsigned int)*newCapacity);
		hashTable->chainCapacity[s_hash] = newCapacity;
	}

	hashTable->chains[s_hash][hashTable->chainElementsN[s_hash]] = s_hash_inChain;
	hashTable->chainElementsN[s_hash]++;
	//fprintf(stderr, "s_hash: %d, s_hash_inChain: %u\n\n", s_hash, s_hash_inChain);

	hashTable->s_hashes[hashTable->tokenN] = s_hash;
	hashTable->s_hashes_inChain[hashTable->tokenN] = s_hash_inChain;
	hashTable->tokenN++;
}


bool hashTable_findToken_inputString(HashTable *hashTable, char *s_begin, int s_len) {
    int s_hash = hashString(s_begin, s_len) % HASH_M;
    unsigned int s_hash_inChain = hashString_inChain(s_begin, s_len);
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


// Main functions
int expressionMatch(char *expr);
bool compute(char *expr, int lenExpr, int mailId);

// Tool functions
int operatorLevel(char);

// Stack
typedef struct CharStack {
    // start from 0 so size will be initialize -1
    char arr[2048];
    int size;
} CharStack;
void charStack_init(CharStack *);
bool charStack_empty(CharStack *);
void charStack_push(CharStack *, char);
char charStack_pop(CharStack *);
char charStack_top(CharStack *);
void charStack_print(CharStack *);



int expressionMatch(char *expr) {
    int ans_len = 0;
	int lenExpr = strlen(expr);
    for (int i = 0; i < n_mails; i++) {
        if (compute(expr, lenExpr, i)) {
            ans_arr[ans_len++] = i;
        }
    }
    return ans_len;
}

bool compute(char *expr, int lenExpr, int mailId) {
    // () -> NOT -> AND -> OR
    CharStack boolStack, operStack;
    charStack_init(&boolStack);
    charStack_init(&operStack);
    for (int i = 0; i < lenExpr; i++) {
        if (expr[i] == '!') {
            charStack_push(&operStack, '!');
        }
        else if (expr[i] == '(') {
            charStack_push(&operStack, '(');
        }
        else if (expr[i] == ')') {
            while (charStack_top(&operStack) != '(') {
                char topOperator = charStack_pop(&operStack);
                if (topOperator == '|') {
                    char bool1 = charStack_pop(&boolStack);
                    char bool2 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, bool1 | bool2);
                }
                else if (topOperator == '&') {
                    char bool1 = charStack_pop(&boolStack);
                    char bool2 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, bool1 & bool2);
                }
                else if (topOperator == '!') {
                    char bool1 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, !bool1);
                }
            }
            charStack_pop(&operStack);  // Pop the '(' 
        }
        else if (expr[i] == '|') {
            // printf("found operation %c\n", expr[i]);
            charStack_push(&operStack, '|');
        }
        else if (expr[i] == '&') {
            // printf("found operation %c\n", expr[i]);
            charStack_push(&operStack, '&');
        }
        else {  // reach a token
            int tokenLen = 0;
            while (i+tokenLen < lenExpr && !isDelimiter(expr[i+tokenLen])) {
                tokenLen++;
            }
            char charPushed = hashTable_findToken_inputString(&hashTables[mailId], expr+i, tokenLen)?1:0;
            i += tokenLen - 1;
            charStack_push(&boolStack, charPushed);
        }
        while (operStack.size > 0) {  // size larger than 1
            char topOperator = charStack_pop(&operStack);
            char preOperator = charStack_pop(&operStack);
            int preLevel = operatorLevel(preOperator);
            int topLevel = operatorLevel(topOperator);
            if (preLevel >= topLevel && preOperator != '(') {
                if (preOperator == '|') {
                    char bool1 = charStack_pop(&boolStack);
                    char bool2 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, bool1 | bool2);
                }
                else if (preOperator == '&') {
                    char bool1 = charStack_pop(&boolStack);
                    char bool2 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, bool1 & bool2);
                }
                else if (preOperator == '!') {
                    char bool1 = charStack_pop(&boolStack);
                    charStack_push(&boolStack, !bool1);
                }
                charStack_push(&operStack, topOperator);
            } else {
                charStack_push(&operStack, preOperator);
                charStack_push(&operStack, topOperator);
                break;
            }
        }
    }
    while (!charStack_empty(&operStack)) {
        char topOperator = charStack_pop(&operStack);
        if (topOperator == '|') {
            char bool1 = charStack_pop(&boolStack);
            char bool2 = charStack_pop(&boolStack);
            charStack_push(&boolStack, bool1 | bool2);
        }
        else if (topOperator == '&') {
            char bool1 = charStack_pop(&boolStack);
            char bool2 = charStack_pop(&boolStack);
            charStack_push(&boolStack, bool1 & bool2);
        }
        else if (topOperator == '!') {
            char bool1 = charStack_pop(&boolStack);
            charStack_push(&boolStack, !bool1);
        }
    }
    return charStack_top(&boolStack) == 1;
}

int operatorLevel(char someOperator) {
    if (someOperator == '|') {
        return 0;
    }
    if (someOperator == '&') {
        return 1;
    }
    if (someOperator == '!') {
        return 2;
    }
    if (someOperator == '(') {
        return 3;
    }
    return -1;
}

// Stack
void charStack_init(CharStack *cs) {
    cs->size = -1;
}

bool charStack_empty(CharStack *cs) {
    return cs->size <= -1;
}

void charStack_push(CharStack *cs, char charPushed) {
    cs->size++;
    cs->arr[cs->size] = charPushed;
}

char charStack_pop(CharStack *cs) {
    if (!charStack_empty(cs)) {
        cs->size--;
        return cs->arr[cs->size+1];
    }
    else {
        return -1;
    }
}

char charStack_top(CharStack *cs) {
    if (!charStack_empty(cs)) {
        return cs->arr[cs->size];
    }
    else {
        return -1;
    }
}
// Stack end

int LARGEST_GROUP_SIZE = 0;
int GROUP_NUMBER = 0;
/*
Hash方式 
回傳值範圍是 0 ~ 5000
*/
typedef struct DJset{
	int s;			//size
	char *n;		//原本的名字 防止撞
	struct DJset *h;//屬於哪個集合
}set;
set DJ[5000];
set *findSet(char *a);
int hash_GroupAnalyze(char *a);
void init_GroupAnalyze(){
	LARGEST_GROUP_SIZE = 0;
	GROUP_NUMBER = 0;
	for(int i = 0;i<5000;i++){
		DJ[i].h = NULL;
		DJ[i].s = 0;
	}
	return;
}
int getLargestSize(){
	return LARGEST_GROUP_SIZE;
}
int getGroupNumber(){
	return GROUP_NUMBER;
}
void addGroupNumber(){
	GROUP_NUMBER++;
}
void minusGroupNumber(){
	GROUP_NUMBER--;
}
int max(int a,int b){
	if (a>b)
		return a;
	else
		return b;
}
void updateLargestSize(int n){
	LARGEST_GROUP_SIZE = max(n,LARGEST_GROUP_SIZE);
}
void makeSet(set *d,char *a){
	d->h = d;
	d->n = a;
	d->s = 1;
	addGroupNumber();
	return;
}
void unionSet(set *a,set *b){
	if(a->s > b->s){
		b->h = a;
		a->s += b->s;
		updateLargestSize(a->s);
	}
	else{
		a->h = b;
		b->s += a->s;
		updateLargestSize(b->s);
	}
	minusGroupNumber();
	return;
}
int *GA(int l,int *id,mail *m){
	init_GroupAnalyze();

	for(int i = 0;i < l;i++){
		int num = id[i];
		if(!strcmp(m[num].to,m[num].from))
			continue;
		set *a = findSet(m[num].to);
		if (a->h == NULL){
				makeSet(a,m[num].to);
		}
		set *b = findSet(m[num].from);
		if (b->h == NULL){
				makeSet(b,m[num].from);
		}
		if(a != b)
			unionSet(a,b);

	}
	ans_arr[0] = getGroupNumber();
	ans_arr[1] = getLargestSize();
}
int charValue(char a){
	if (a > 96){
		return a - 96;
	}else
		return a - 64;
}
int hash_GroupAnalyze(char *a){		//記得傳來得要'\0'結尾
	if(a[0] == 'l')
		return 0;					//只有一個lindsay
	int value = 0,i = 1;
	while(a[i] != '\0'){
		value += charValue(a[i])*i;
		i++;
	}
	value *= i;
	value += 7 * charValue(a[0]);
	if(value > 8000)
		return 4800;
	if(value > 5000)
		return 4801;
	return value - 24;
}
set *pathCompression(set *m){
	if(m != m->h){
		m->h = pathCompression(m->h);
		return m->h;
	}else
		return m;
}
set *findSet(char *a){		
	int ind = hash_GroupAnalyze(a);

	set *m = &DJ[ind];
	if (m->h == NULL)
		return m;
	while(m->h != NULL && strcmp(m->n,a) != 0 ){		//防止collide
		m += 1;
	}
	if (m->h == NULL || m->h == m)
		return m;

	
	return pathCompression(m);
}

int findSimilar_solve (int mid, double thres){
	int ans_len = 0;
	for (int mail_ctr=0; mail_ctr < n_mails; mail_ctr++){
		if (mail_ctr == mid)
			continue;
		

		// compare the less token mail with the more token one
		// 「我要叫他啟發式比較。」
		//			 - 2021, 半夜在寫扣的人
		HashTable *hashTable_lessToken;
		HashTable *hashTable_moreToken;
		if (hashTables[mid].tokenN > hashTables[mail_ctr].tokenN){
			hashTable_lessToken = &hashTables[mail_ctr];
			hashTable_moreToken = &hashTables[mid];
		}
		else{
			hashTable_lessToken = &hashTables[mid];
			hashTable_moreToken = &hashTables[mail_ctr];
		}
	
		double union_count = hashTable_moreToken->tokenN;
		double intersec_count = 0;
		for (int token_ctr=0; token_ctr < hashTable_lessToken->tokenN; token_ctr++){
			int find = hashTable_findToken_inputHash(hashTable_moreToken, hashTable_lessToken->s_hashes[token_ctr], hashTable_lessToken->s_hashes_inChain[token_ctr]);
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


int main (void) {
	// Initialization
	toNumber_init();
	api.init(&n_mails, &n_queries, &mails, &queries);
	hashTables_init(n_mails);
	for (int mail_ctr = 0; mail_ctr < n_mails; mail_ctr++){
		hashTable_hashmail(&hashTables[mail_ctr], mails[mail_ctr]);
	}

	for(int i = 0; i < n_queries; i++){
		if (queries[i].type == expression_match){
			continue;
			int ans_len = expressionMatch(queries[i].data.expression_match_data.expression);
			api.answer(queries[i].id, ans_arr, ans_len);
		}

		if (queries[i].type == find_similar){
			int ans_len = findSimilar_solve(queries[i].data.find_similar_data.mid, queries[i].data.find_similar_data.threshold);
			api.answer(queries[i].id, ans_arr, ans_len);
		}
			
		if (queries[i].type == group_analyse){
			continue;
			GA(queries[i].data.group_analyse_data.len,queries[i].data.group_analyse_data.mids,mails);
			api.answer(queries[i].id, ans_arr, 2);
		}
	}
	return 0;
}
