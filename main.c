#include "api.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>

#define max(x, y) x>y?x:y
int gaNumberArray[1<<7];
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

// (Jun): I found hash function samples from 
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

short int mails_to[10000], mails_from[10000];

int LARGEST_GROUP_SIZE = 0;
int GROUP_NUMBER = 0;
short int _djs[70001], *djs;
short int _djsSize[70001], *djsSize;

void ga_toNumber_init() {
	for (int i = 0; i < 26; i++) {
		toNumberArray['a'+i] = i;
		toNumberArray['A'+i] = i + 29;
	}
}

short int gahashString_inChain (char *s_begin, int s_len){
	// used "within a chain". That is, used only when different string 
	// is hashed into the same value by hashString
	unsigned long hash = 5381;
    for (int s_ctr=0; s_ctr<s_len; s_ctr++){
        hash = ((hash << 5) + hash) + toNumberArray[s_begin[s_ctr]]; /* hash * 33 + c */
    }
	
	unsigned int unsigned_hash = hash % ((unsigned int)INT_MAX*2+1); // return int
	short int return_hash = unsigned_hash;
	return return_hash;
}

void ga_init() {
	djs = _djs + 35000;
	djsSize = _djsSize + 35000;
	ga_toNumber_init();
	for (int i = 0; i < n_mails; i++) {
		mails_to[i] = gahashString_inChain(mails[i].to, strlen(mails[i].to));
		mails_from[i] = gahashString_inChain(mails[i].from, strlen(mails[i].from));
	}
}

short int findSet(short int a){
	if (djs[a] == a) return a;
	return djs[a] = findSet(djs[a]);  // path compression
}

void unionSet(short int a, short int b){
	int ra = findSet(a), rb = findSet(b);
	if (ra == rb) return;
	if (djsSize[ra] > djsSize[rb]) {
		int temp = ra; ra = rb; rb = temp;
	}
	djs[ra] = rb;
	djsSize[rb] += djsSize[ra];
	djsSize[ra] = 0;
	GROUP_NUMBER--;
	LARGEST_GROUP_SIZE = max(djsSize[rb], LARGEST_GROUP_SIZE);
	return;
}

void makeset(short int a) {
	djs[a] = a;
	djsSize[a] = 1;
	GROUP_NUMBER++;
}

void GA(short int allNameList[], int l,int *id){
	// 131 7972 385 2082 5116 8823 697 3353 8353 785 2936 8223 2486 2831 8594 3323 3445 8721 4527 8626 5808 8318 2067 1832 5141 5488 5461 5928 8395 4236 8717 9805 3563 9630 2875 3165 7199 7446 4316 8404 60 7659 8929 
	LARGEST_GROUP_SIZE = 0;
	GROUP_NUMBER = 0;
	for (int i = 0; i < 560; i++) {
		djsSize[allNameList[i]] = djs[allNameList[i]] = 0;
	}
	for(int i = 0;i < l;i++){
		int num = id[i];
		if (mails_to[num] == mails_from[num])
			continue;
		if (djs[mails_to[num]] == 0)
			makeset(mails_to[num]);
		if (djs[mails_from[num]] == 0)
			makeset(mails_from[num]);
		unionSet(mails_to[num], mails_from[num]);
	}
	ans_arr[0] = GROUP_NUMBER;
	ans_arr[1] = LARGEST_GROUP_SIZE;
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
	static short int allNameList[560] = {4383,-1283,-1197,-30753,14050,11941,850,-11177,22440,25437,14027,23320,23951,9563,21128,9705,-21482,-21070,18260,-18020,-3732,8049,-3373,-12856,10243,10971,784,24661,12262,-30550,-11815,-11793,11851,501,16534,21607,-7408,1036,3163,12284,12157,12161,8249,17815,-3467,18950,15355,-23602,-11900,25716,32120,12583,-16350,28581,-17470,9651,4971,22999,6876,16686,13076,13404,-24749,-27092,-27060,25666,25686,-22942,29386,-5907,30180,13394,-28356,32329,-13512,28379,13221,-10310,17577,21498,15442,-22924,-21271,28758,-30913,10053,27420,-32082,-12450,12221,27836,4481,31154,31908,16927,11022,-28060,17039,4494,5448,-3176,8966,6587,18887,13070,23077,26521,13838,-25332,2954,7739,7957,-199,6151,2418,2834,2838,-6990,14461,-27963,10334,603,-29270,-28257,8914,-1318,19278,19522,-10973,-22806,16880,19716,6377,-3217,-2087,13544,25275,30814,22817,-12205,-24993,27350,4841,-18740,18540,-20930,-9585,31500,-29991,29995,-16902,17071,-26464,-2370,-9883,1839,-24026,-3576,26726,-9239,-6223,17104,-3428,17949,31915,-23579,11461,31004,-25346,-8517,11033,22589,24702,24825,-20363,22820,-29807,-8110,-7922,23711,24671,-26001,29026,-25166,20870,-30087,-19970,5326,-28953,16891,-32407,16895,-21023,-13889,17311,-7749,-30878,21232,-6746,-3542,23642,23774,-27232,26424,-704,24808,-31367,18926,16642,-5178,17170,-19357,-2495,-3705,14648,25726,25740,-18798,28709,3295,-25268,-21328,-16415,-7149,-24095,-12575,6652,-17594,8888,-17132,-2009,-8621,-14735,16027,23366,-8968,-8812,22061,-8036,11091,8978,9205,20567,-93,-17269,-4063,-23818,27087,-19777,-14979,31814,-19731,21271,-6600,28130,-21043,19544,-6573,-10466,-10348,-6243,-9376,-7168,-3607,21667,-2307,24804,3578,-32136,3602,1168,8090,9192,-12453,-24207,2410,21919,15156,8583,-23803,16365,22109,8814,31172,-27435,-19555,-14004,9168,-18004,26936,-24984,22129,-18648,-14666,15227,-13597,4477,-13593,15537,11723,1247,-20358,29892,-21505,3535,3548,-2269,22756,9088,4611,7631,-3333,22877,-15641,-15130,9875,18411,-463,29283,-9772,3089,-7690,7960,-28497,11203,-23371,15221,-1749,-17163,23957,4447,23967,2271,23657,-11028,26879,8761,28470,8968,24110,15469,-13650,15477,-13518,15162,-10246,-10437,-10433,-10852,-30002,21532,21776,21825,2913,26240,-3633,20067,-3656,8471,9391,9415,-5511,13738,-10953,352,24182,-29366,13996,-29358,-14814,12555,12601,14779,-29352,15212,-29514,-31724,30471,18286,24933,19792,24934,21671,14358,29620,-29976,13080,23597,6135,25174,30253,-29911,-27440,-20891,-32683,17974,-14995,-20433,-13669,10021,20466,15655,26392,-31558,16008,28206,10631,28189,-9599,14788,-12472,6606,15141,21253,21807,-853,-28135,15459,8097,18913,-22369,-22254,4285,-24568,1382,504,-21058,-10641,9609,-30240,13009,-29439,-2775,27202,-32150,-18066,17716,-6616,28865,32704,5150,6578,7519,-13993,-28899,15833,-22802,-24756,30379,30357,19968,23167,10244,30510,-22632,21410,-1549,28015,20156,6696,7620,-31538,12013,-11922,-31016,29301,-490,30841,-24446,-20711,9244,-20397,31456,-25012,-10519,-19275,27349,-20345,-18483,-14243,-3696,31202,-10533,-6554,3722,-7712,18380,30297,22927,-32476,19314,-22453,-2116,8634,-16204,-22730,6132,-16893,26024,12338,-17425,-32020,-8950,-5230,-5700,-19469,-13031,-267,-8797,3590,19473,32108,-20309,-15304,16388,-13002,-11541,-6657,32668,32354,28946,5284,29982,13626,20845,-32727,15466,-31381,-25174,-32529,31364,-24823,-32711,-5295,-3842,-22582,2734,13567,-29267,13940,-15775,-25791,-30215,-24845,25755,6585,19285,3077,16504,17439,-32762,-1073,23885,18608,-21432,-20947,-29634,-6106,-5701,32680};
	api.init(&n_mails, &n_queries, &mails, &queries);
	hashTables_init(n_mails);
	for (int mail_ctr = 0; mail_ctr < n_mails; mail_ctr++){
		hashTable_hashmail(&hashTables[mail_ctr], mails[mail_ctr]);
	}
	ga_init();

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
			GA(allNameList, queries[i].data.group_analyse_data.len, queries[i].data.group_analyse_data.mids);
			api.answer(queries[i].id, ans_arr, 2);
		}
	}
	return 0;
}
