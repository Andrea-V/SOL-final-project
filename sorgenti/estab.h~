#ifndef _ESTAB_H_
#define _ESTAB_H_

/*
 Tabella delle stime, dinamica.
*/

/*entry tabella delle stime*/
typedef struct{
	uint64_t id;
	int secret;
	int count;
}esentry_t;

/*tabella delle stime*/
typedef struct{
	pthread_mutex_t mutex;
	esentry_t** entries;
	int size;
}estab_t;

/*secret*/
typedef struct{
	uint64_t id;
	int secret;
}secret_t;

/*operazioni*/
void init_tab(estab_t*);
void add_new_est(estab_t*,secret_t*);
void destroy_tab(estab_t*);
int  get_est_of(estab_t*,uint64_t);
void print_tab(estab_t*,FILE*);

#endif /*_ESTAB_H_*/
