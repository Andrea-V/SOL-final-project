#include<stdio.h>
#include<pthread.h>
#include<arpa/inet.h>
#include<inttypes.h>
#include<malloc.h>

#include"estab.h"

/*operazionio private (non thread-safe)*/
void init_entry(esentry_t* entry,uint64_t,long int);
void add_entry(estab_t*,uint64_t,long int);

/*implementazione*/
void init_tab(estab_t* t){
	pthread_mutex_init(&t->mutex,NULL);
	t->entries=NULL;
	t->size=0;
}

void init_entry(esentry_t* entry,uint64_t id,long int secret){
	entry->id=id;
	entry->secret=secret;
	entry->count=1;
}

void add_entry(estab_t* tab,uint64_t id,long int secret){
	tab->size++;
	tab->entries=realloc(tab->entries,tab->size*sizeof(esentry_t*));
	tab->entries[tab->size-1]=malloc(sizeof(esentry_t));
	init_entry(tab->entries[tab->size-1],id,secret);
}

void destroy_tab(estab_t* t){
	int i;	
	
	for(i=0;i<t->size;i++)		
		free(t->entries[i]);
	free(t->entries);
	free(t);
}

void add_new_est(estab_t* t,secret_t* est){
	int i;

	pthread_mutex_lock(&t->mutex);

	for(i=0;i<t->size;i++)
		if(t->entries[i]->id==est->id){
			t->entries[i]->count++;

			/*cambio la stima sse e' minore (=> piu' precisa) di quella che ho gia'*/
			if(t->entries[i]->secret>est->secret)
				t->entries[i]->secret=est->secret;
			pthread_mutex_unlock(&t->mutex);
			return;
		}
	add_entry(t,est->id,est->secret);
	pthread_mutex_unlock(&t->mutex);
}

void print_tab(estab_t* t,FILE* fd){
	int i;
	pthread_mutex_lock(&t->mutex);
	
	fprintf(fd,"\n");
	for(i=0;i<t->size;i++)
		fprintf(fd,"SUPERVISOR ESTIMATE %d FOR %"PRIX64" BASED ON %d\n",t->entries[i]->secret,t->entries[i]->id,t->entries[i]->count);

	pthread_mutex_unlock(&t->mutex);
}

int get_est_of(estab_t* t,uint64_t id){
	int i,res=-1;
	pthread_mutex_lock(&t->mutex);
	
	for(i=0;i<t->size;i++)
		if(t->entries[i]->id==id)
			res=t->entries[i]->secret;

	pthread_mutex_unlock(&t->mutex);
	return res;
}
