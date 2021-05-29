#include "api.h"
#include "hashTable.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define CHAR_LEN_FOR_HIT 10
#define TOKEN_MAX_LEN 1000

#define min(x, y) x>y?y:x


// Main functions
int expressionMatch(char *expr, int *ans_arr, HashTable *hashTables[]);
bool compute(char *expr, int lenExpr, int mailId, HashTable *hashTables[]);

// Tool functions
int operatorLevel(char);
void myStrcpy(char *dest, char *source, int len);

// Stack
typedef struct CharStack {
    // start from 0 so size will be initialize -1
    char *arr;
    int size, capa;
} CharStack;
void charStack_init(CharStack *);
bool charStack_empty(CharStack *);
void charStack_push(CharStack *, char);
char charStack_pop(CharStack *);
char charStack_top(CharStack *);
void charStack_print(CharStack *);



int expressionMatch(char *expr, int *ans_arr, HashTable *hashTables[]) {
    int ans_len = 0;
	int lenExpr = strlen(expr);
    for (int i = 0; i < n_mails; i++) {
        if (compute(expr, lenExpr, i, hashTables)) {
            ans_arr[ans_len++] = i;
        }
    }
    return ans_len;
}


bool compute(char *expr, int lenExpr, int mailId, HashTable *hashTables[]) {
    // () -> NOT -> AND -> OR
    CharStack boolStack, operStack;
    charStack_init(&boolStack);
    charStack_init(&operStack);
    for (int i = 0; i < lenExpr; i++) {
        // printf("\n");
        // charStack_print(&boolStack);
        // charStack_print(&operStack);
        if (expr[i] == '!') {
            // printf("found operation %c\n", expr[i]);
            charStack_push(&operStack, '!');
        }
        else if (expr[i] == '(') {
            // printf("found operation %c\n", expr[i]);
            charStack_push(&operStack, '(');
        }
        else if (expr[i] == ')') {
            // printf("found operation %c at %d\n", expr[i], i);
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
            char token[TOKEN_MAX_LEN];
            int lenToken = 0;
            while (i < lenExpr && !isDelimiter(expr[i])) {
                token[lenToken++] = expr[i];
                i++;
            }
            i--;
            token[lenToken] = '\0';
            
            // (Jun): I changed findToken() into using hashTable
            // char charPushed = findToken(token, lenToken, mailId)?1:0;
            char charPushed = hashTable_findToken_inputString(hashTables[mailId], token, lenToken);

            // printf("found string %s => %d\n", token, charPushed==1);
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
        // printf("\n");
        // charStack_print(&boolStack);
        // charStack_print(&operStack);
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
    // printf("\n");
    // charStack_print(&boolStack);
    // charStack_print(&operStack);
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


void myStrcpy(char *dest, char *source, int len) {
    // printf("copying len=%d\n", len);
    for (int i = 0; i < len; i++) {
        dest[i] = source[i];
    }
    dest[len] = '\0';
}


// Stack
void charStack_init(CharStack *cs) {
    cs->size = -1;
    cs->capa = 1;
    cs->arr = malloc(sizeof(char)*cs->capa);
}


bool charStack_empty(CharStack *cs) {
    return cs->size <= -1;
}


void charStack_push(CharStack *cs, char charPushed) {
    if (cs->size+1 >= cs->capa) {
        cs->capa *= 2;
        cs->arr = realloc(cs->arr, sizeof(char)*cs->capa);
    }
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


void charStack_print(CharStack *cs) {
    for (int i = 0; i <= cs->size; i++) {
        if (cs->arr[i] == 0) {
            printf("0 ");
        }

        else if (cs->arr[i] == 1) {
            printf("1 ");
        }
        
        else {
            printf("%c ", cs->arr[i]);
        }
    }
    printf(" size = %d", cs->size);
    printf("\n");
}
// Stack end




/*
// ======= dynamic array =======
// ======= function declearations =======

typedef struct TokenList
{
    int *list;  // start from 0
    char *charList;  // use for spurious hit
    int size, capa;
}TokenList;

void initTokensFromMails();
void token_init(TokenList *);
void token_push(TokenList *tokenList, char *T, int len);
void initTokensFromString(TokenList *, char *);
bool findToken(char *P, int lenP, int mailId);

TokenList mailTokenLists[MAX_ANSWER_LEN];
*/


/*
// ======= dynamic array =======
// ======= function definitions =======

void initTokensFromMails() {
    for (int i = 0; i < n_mails; i++) {
        token_init(&mailTokenLists[i]);
        initTokensFromString(&mailTokenLists[i], mails[i].subject);
        initTokensFromString(&mailTokenLists[i], mails[i].content);
    }
}


void token_init(TokenList *tokenList) {
    tokenList->capa = 1;
    tokenList->size = -1;
    tokenList->list = malloc(tokenList->capa * sizeof(int));
    tokenList->charList = malloc(CHAR_LEN_FOR_HIT * tokenList->capa * sizeof(char));
}


void token_push(TokenList *tokenList, char *T, int len) {
    // printf("pushing ");
    // for (int i = 0; i < len; i++)
    //     printf("%c", T[i]);
    // printf("\n");
    if (tokenList->size+1 >= tokenList->capa) {
        tokenList->capa *= 2;
        tokenList->list = realloc(tokenList->list, sizeof(int) * tokenList->capa);
        tokenList->charList = realloc(tokenList->charList, CHAR_LEN_FOR_HIT * tokenList->capa * sizeof(char));
    }
    tokenList->list[++tokenList->size] = hashString(T, len);
    myStrcpy(tokenList->charList + tokenList->size * CHAR_LEN_FOR_HIT, T, min(CHAR_LEN_FOR_HIT-1, len));
}


void initTokensFromString(TokenList* tokenList, char *T) {
    for (int s = 0; T[s] != '\0'; s++) {
        if ((s == 0 && isDelimiter(T[s])) || (s != 0 && !(isDelimiter(T[s-1]) && !isDelimiter(T[s])))) {
            continue;
        }
        int sLen = 0;
        while (!isDelimiter(T[s+sLen])) {
            // printf("%c", T[s+sLen]);
            sLen++;
        }
        // printf("\n");
        token_push(tokenList, T+s, sLen);
        s += sLen;
    }
}


bool findToken(char *P, int lenP, int mailId) {
    bool found = false;
    int p = hashString(P, lenP);
    // printf("searching for token %s=%d\n", P, p);
    int *list = mailTokenLists[mailId].list;
    for (int i = 0; i <= mailTokenLists[mailId].size; i++) {
        if (list[i] == p) {
            // printf("We got a hit!\n");
            char *charList = mailTokenLists[mailId].charList + i * CHAR_LEN_FOR_HIT;
            for (int i = 0; i < CHAR_LEN_FOR_HIT; i++) {
                // printf("comparing %c to %c", P[i], charList[i]);
                if (charList[i] == '\0') {
                    // printf("Found!!\n");
                    found = true;
                    break;
                }
                if (toNumber(P[i]) != toNumber(P[i])) break;
            }
        }
    }
    return found;
}

// Dynamic array end

*/