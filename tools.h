#include <stdio.h>
#include <string.h>

#define Q_RabinKarp 59650000
#define D_RabinKarp 36


char toNumber(char c) {
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('A' <= c && c <= 'Z') {
        return c - 'A' + 10;
    }
    return c - 'a' + 10;
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
        hash = ((hash << 5) + hash) + s_begin[s_ctr]; /* hash * 33 + c */
    }
	
	unsigned int return_hash = hash % (INT_MAX*2+1); // return int
	return return_hash;
}

