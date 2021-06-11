#include "api.h"
#include "hashTable.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// Mark1: Functions declarations

// Main functions
int expressionMatch(char *expr, int *ans_arr, HashTable *hashTables[]);
bool compute(char *expr, int lenExpr, int mailId, HashTable *hashTables[],
    int hashedTokens[], unsigned int hashedInChainTokens[], int exprTokenLens[]);

// Tool functions
int operatorLevel(char);

// Stack
struct Charstack;
typedef struct CharStack CharStack;
void charStack_init(CharStack *);
bool charStack_empty(CharStack *);
void charStack_push(CharStack *, char);
char charStack_pop(CharStack *);
char charStack_top(CharStack *);


// Mark2: Functions definitions
// Stack
typedef struct CharStack {
    // start from 0 so size will be initialize -1
    char arr[3000];
    int size;
} CharStack;


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


int expressionMatch(char *expr, int *ans_arr, HashTable *hashTables[]) {
    int ans_len = 0;
	int lenExpr = strlen(expr);
    int hashedTokens[2048], exprTokenCnt = 0;
    unsigned int hashedInChainTokens[2048];
    int exprTokenLens[2048];
    for (int i = 0; i < lenExpr; i++) {
        if (isDelimiter(expr[i]))
            continue;
        int tokenLen = 0;
        while (i+tokenLen < lenExpr && !isDelimiter(expr[i+tokenLen])) {
            tokenLen++;
        }
        int s_hash = hashString(expr+i, tokenLen) % HASH_M;
        unsigned int s_hash_inChain = hashString_inChain(expr+i, tokenLen);
        hashedTokens[exprTokenCnt] = s_hash;
        hashedInChainTokens[exprTokenCnt] = s_hash_inChain;
        exprTokenLens[exprTokenCnt] = tokenLen;
        exprTokenCnt++;
        i += tokenLen - 1;
    }
    
    for (int i = 0; i < n_mails; i++) {
        if (compute(expr, lenExpr, i, hashTables, hashedTokens, hashedInChainTokens, exprTokenLens)) {
            ans_arr[ans_len++] = i;
        }
    }
    return ans_len;
}


bool compute(char *expr, int lenExpr, int mailId, HashTable *hashTables[],
    int hashedTokens[], unsigned int hashedInChainTokens[], int exprTokenLens[]) {
    // () -> NOT -> AND -> OR
    CharStack boolStack, operStack;
    charStack_init(&boolStack);
    charStack_init(&operStack);
    int exprTokenCnt = 0;
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
            // (Jun): I changed findToken() into using hashTable
            // char charPushed = findToken(token, lenToken, mailId)?1:0;
            char charPushed = hashTable_findToken_inputHash(hashTables[mailId]
                , hashedTokens[exprTokenCnt], hashedInChainTokens[exprTokenCnt])?1:0;
            i += exprTokenLens[exprTokenCnt] - 1;
            exprTokenCnt++;
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
    bool returnAns = charStack_top(&boolStack) == 1;
    return returnAns;
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
