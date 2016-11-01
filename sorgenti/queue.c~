#ifndef _QUEUE_C_
#define _QUEUE_C_

#include<stdio.h>
#include<malloc.h>

/*
 Coda FIFO dinamica.
*/
#define TYPE	long int
#define GROWTH	2
/*#define DEBUG*/

typedef struct {
	long int*queue;
	int head,tail;
	int size,maxsize,minsize;
}queue_t;

/*operazioni*/
void debug_printq(queue_t*);	 /*O(maxsize) nota: usarla solo se DEBUG e' definito*/
void printq(queue_t*); 			 /*O(size)*/
void initq(queue_t*,int);		 /*O(1)*/
void destroyq(queue_t*);		 /*O(1)*/
queue_t* expand(queue_t*); 	 	 /*O(size)*/
queue_t* reduce(queue_t*);   	 /*O(size)*/
queue_t* enqueue(queue_t*,TYPE); /*O(1) ammortizzato*/
queue_t* dequeue(queue_t*,TYPE*);/*O(1) ammortizzato*/

/*implementazione*/
void debug_printq(queue_t*q){
	int i;
	printf("[");	
	for(i=0;i<q->maxsize;i++)
		printf("%ld,",q->queue[i]);
	printf("\b]\n");
}

void printq(queue_t*q){
	int i,j;
	printf("[");	
	for(j=0,i=q->head;j<q->size;i=(i+1)%q->maxsize,j++)
		printf("%ld ",q->queue[i]);
	printf("]\n");
}

void initq(queue_t *q,int n){
#ifdef DEBUG	
	int i; /**/
#endif
	q->maxsize=n;
	q->minsize=n;
	q->size=0;
	q->head=0;
	q->tail=0;
	q->queue=malloc(n*sizeof(TYPE));
#ifdef DEBUG
	for(i=0;i<n;i++)/**/
		q->queue[i]=0;
#endif
}
void destroyq(queue_t *q){
	free(q->queue);
}

queue_t* expand(queue_t* src){
	queue_t* dst;
	TYPE tmp;	

	dst=malloc(sizeof(queue_t));
	initq(dst,src->size*GROWTH);
	dst->minsize=src->minsize; /*il size minimo non cambia*/	
	
	while(src->size>0){
		src=dequeue(src,&tmp);
		dst=enqueue(dst, tmp);
	}
	destroyq(src);
	return dst;
}

queue_t* reduce(queue_t* src){
	queue_t* dst;
	TYPE tmp;

	dst=malloc(sizeof(queue_t));
	initq(dst,src->size/GROWTH);
	dst->minsize=src->minsize; /*il size minimo non cambia*/
	
	while(src->size>0){
		src=dequeue(src,&tmp);
		dst=enqueue(dst, tmp);
	}
	destroyq(src);
	return dst;
}

queue_t* enqueue(queue_t* q,TYPE el){
	if(q->size==q->maxsize)
		q=expand(q);
	
	q->queue[q->tail]=el;
	q->tail=(q->tail+1)%q->maxsize;
	q->size++;
	return q;
}

queue_t* dequeue(queue_t* q,TYPE *el){
	if(!q->size)
		return q;
	
	*el=q->queue[q->head];
#ifdef DEBUG
	q->queue[q->head]=0;/**/
#endif	
	q->head=(q->head+1)%q->maxsize;
	q->size--;

	if((q->size<(q->maxsize/GROWTH)) && q->size>q->minsize)
		q=reduce(q);
	return q;
}

#endif /*_QUEUE_C_*/
