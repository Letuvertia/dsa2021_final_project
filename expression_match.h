#include "api.h"
#include "hashTable.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

// #define TOKEN_MAX_LEN 1000 // (Jun): this is the max length of ONE token?
                           // (Anthony): yes. I just guess that it'll less than 1000

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
            int tokenLen = 0;
            while (i+tokenLen < lenExpr && !isDelimiter(expr[i+tokenLen])) {
                tokenLen++;
            }
            
            // (Jun): I changed findToken() into using hashTable
            // char charPushed = findToken(token, lenToken, mailId)?1:0;
            char charPushed = hashTable_findToken_inputString(hashTables[mailId], expr+i, tokenLen)?1:0;
            i += tokenLen - 1;
            // 1736
            // Henry
            // Mumbai Champs was one of the nine teams played in the defunct Indian Cricket League (ICL). The team is based in Mumbai, India and its captain is former New Zealand batsman Nathan Astle.[1][2] The squad announced for the inaugural tournament comprises four international cricketers. Nathan Astle, Johan Van der Wath, Tino Best, Michael Kasprowicz are amongst the high-profile names playing for the Mumbai Champs.[3] Coach - Sandeep Patil   I read the paragraph on http://wikipedia.org 
            // Mumbai Champs
            // Victoria

            // !((proboard)&((c19)&(plo))|(aira)|(!((tolbooth)|(459e)|(!tino)&(!mdrx)|(522052n)))&(!20022007))

            // if (mailId == 1736)
            //     printf("found string %s(hashed:%d) => %d\n", token, hashString(token, lenToken), charPushed?1:0);

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
    free(boolStack.arr);
    free(operStack.arr);
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
