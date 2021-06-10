/*
#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


// Mark1: Functions declarations
struct DJset;
typedef struct DJset set;
int charValue(char a);
int hash_GroupAnalyze(char *a);
set *pathCompression(set *m);
set *findSet(char *a, set *);
void makeSet(set *d, char *a, int *GROUP_NUMBER);
void unionSet(set *a,set *b, int *LARGEST_GROUP_SIZE, int *GROUP_NUMBER);
int *GA(int *ans_arr, int l,int *id, mail *m);


// Mark2: Functions definitions
typedef struct DJset{
	int s;			 //size
	char *n;		 
	struct DJset *h; 
}set;


int charValue(char a){
	if (a > 96){
		return a - 96;
	}else
		return a - 64;
}


int hash_GroupAnalyze(char *a){		
	if(a[0] == 'l')
		return 0;
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


set *findSet(char *a, set *DJ){		
	int ind = hash_GroupAnalyze(a);

	set *m = DJ+ind;
	if (m->h == NULL)
		return m;
	while(m->h != NULL && strcmp(m->n,a) != 0 ){
		m += 1;
	}
	if (m->h == NULL || m->h == m)
		return m;

	while(m->h != m)
		m = m->h;
	return m;
}


void makeSet(set *d, char *a, int *GROUP_NUMBER){
	d->h = d;
	d->n = a;
	d->s = 1;
	*GROUP_NUMBER++;
	return;
}


void unionSet(set *a,set *b, int *LARGEST_GROUP_SIZE, int *GROUP_NUMBER){
	if(a->s > b->s){
		b->h = a;
		a->s += b->s;
		*LARGEST_GROUP_SIZE = (a->s > *LARGEST_GROUP_SIZE)? a->s: *LARGEST_GROUP_SIZE;
	}
	else{
		a->h = b;
		b->s += a->s;
		*LARGEST_GROUP_SIZE = (b->s > *LARGEST_GROUP_SIZE)? b->s: *LARGEST_GROUP_SIZE;
	}
	*GROUP_NUMBER--;
	return;
}


int *GA(int *ans_arr, int l,int *id, mail *m){
	set DJ[5000];

	// init
	int *LARGEST_GROUP_SIZE = 0;
	int *GROUP_NUMBER = 0;
	for(int i = 0; i<5000; i++){
		DJ[i].h = NULL;
		DJ[i].s = 0;
	}

	for(int i = 0; i<l; i++){
		int num = id[i];
		if(!strcmp(m[num].to,m[num].from))
			continue;
		set *a = findSet(m[num].to, DJ);
		if (a->h == NULL){
				makeSet(a,m[num].to, GROUP_NUMBER);
		}
		set *b = findSet(m[num].from, DJ);
		if (b->h == NULL){
				makeSet(b,m[num].from, GROUP_NUMBER);
		}
		if(a != b)
			unionSet(a, b, LARGEST_GROUP_SIZE, GROUP_NUMBER);
	}
	ans_arr[0] = GROUP_NUMBER;
	ans_arr[1] = LARGEST_GROUP_SIZE;

	return ans_arr;
}

抱歉了俊偉 但我的扣比較讚:)
*/


// (Jun to Kain): Belows are your original codes.
// Sorry for reorgranizing your code.
// For the convience of merging header files, I make your
// global variables into local ones, and delete some short functions.
// Please let me know if you have any questions about the current version.

#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


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
bool IsSet[5000] = {};
int ans[2];
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
		set *a = findSet(m[num].to),*b = findSet(m[num].from);
		if(a->h == NULL || b->h == NULL){
			if (a->h == NULL)
				makeSet(a,m[num].to);
			if (b->h == NULL)
				makeSet(b,m[num].from);
			unionSet(a,b);
		}else if(a != b)
			unionSet(a,b);
	}
	ans[0] = getGroupNumber();
	ans[1] = getLargestSize();
	return ans;
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
	while(m->h != NULL && strcmp(m->n,a))		//防止collide
		m += 1;
	
	if (m->h == NULL || m->h == m)
		return m;
	pathCompression(m);
	
	return m;
}
