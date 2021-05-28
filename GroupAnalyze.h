#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "api.h"
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
set *findset(char *a);
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
void makeset(set *d,char *a){
	d->h = d;
	d->n = a;
	d->s = 1;
	addGroupNumber();
	return;
}
void uni(set *a,set *b){
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
		set *a = findset(m[num].to),*b = findset(m[num].from);
		if(a->h == NULL || b->h == NULL){
			if (a->h == NULL)
				makeset(a,m[num].to);
			if (b->h == NULL)
				makeset(b,m[num].from);
			uni(a,b);
		}else if(a != b)
			uni(a,b);
	}
	ans[0] = getGroupNumber();
	ans[1] = getLargestSize();
	return ans;
}
int char_value(char a){
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
		value += char_value(a[i])*i;
		i++;
	}
	value *= i;
	value += 7 * char_value(a[0]);
	if(value > 8000)
		return 4800;
	if(value > 5000)
		return 4801;
	return value - 24;
}

set *findset(char *a){		
	int ind = hash_GroupAnalyze(a);
	set *m = &DJ[ind];
	if (m->h == NULL)
		return m;
	while(m->h != NULL && strcmp(m->n,a))		//防止collide
		m += 1;
	
	if (m->h == NULL || m->h == m)
		return m;

	while(m->h != m)			//向上找set 要用path compression
		m = m->h;
	
	return m;
}