#ifndef _UTILS_C_
#define _UTILS_C_

#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<time.h>
#include<errno.h>
#include<pthread.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<sys/wait.h>
#include<arpa/inet.h>
#include<inttypes.h>
#include<signal.h>
#include"estab.h"

/*struct utili*/
/*server*/
typedef struct {
	int id;
	int socket_fd;
	struct sockaddr_un address;
}server_t;

/*costanti utili*/
#define TRUE			1
#define FALSE			0
#define STR_MAX			256
#define UNIX_PATH_MAX	108
#define MSG_BUF_MAX		sizeof(uint64_t)
#define SKT_PREFIX		"./OOB-server-"
#define ONE_MILLION		1000000
#define SERVER_PATH		"./server"
#define Q_INIT			20
#define SECRET_MAX		3000

/*funzioni utili*/
/*
 Genera numeri casuali a 64bit.
*/
uint64_t random64() {
    union{
		uint32_t lv[2];
		uint64_t llv;
	}u;
    u.lv[0]=random();
    u.lv[1]=random();
    return u.llv;
}

/*
 Funzioni conversione network/host byte order 64 bit.
*/
uint64_t htonll(uint64_t v) {
    union{
		uint32_t lv[2];
		uint64_t llv;
	}u;
    u.lv[0]=htonl(v >> 32);
    u.lv[1]=htonl(v & 0xFFFFFFFF);
    return u.llv;
}

uint64_t ntohll(uint64_t v) {
    union{
		uint32_t lv[2];
		uint64_t llv;
	}u;
    u.llv=v;
    return ((uint64_t)ntohl(u.lv[0]) << 32) | (uint64_t)ntohl(u.lv[1]);
}

/*
 Calcola il nome del server a partire dal suo id.
*/
char* get_server_name(char* str,int id){
	char tmp[UNIX_PATH_MAX];
	
	strcpy(str,SKT_PREFIX);
	sprintf(tmp,"%d",id);/*per convertire da int a stringa*/
	return strcat(str,tmp);
}

/*
 Inizializza struttura server_t.
*/
void init_server_t(server_t* server,int id){
	char tmp[UNIX_PATH_MAX];
	
	server->id=id;
	server->address.sun_family=AF_UNIX;
	strcpy(server->address.sun_path,get_server_name(tmp,id));
}

/*
 Funzioni di conversione timespec/millis
*/
long int ts_to_msec(struct timespec ts){
	return ts.tv_sec*1000+ts.tv_nsec/ONE_MILLION;
}

struct timespec* msec_to_ts(long int msec){
	struct timespec* ts=malloc(sizeof(struct timespec));
	
	ts->tv_sec=msec/1000;
	msec-=ts->tv_sec*1000;
	ts->tv_nsec=msec*ONE_MILLION;
	return ts;
}
/*
 Abilita/disabilita i segnali.
*/
int sig_block_all(sigset_t* set){
	
	if(-1==pthread_sigmask(SIG_SETMASK,set,NULL))
		return FALSE;
	return TRUE;
}
int sig_unblock_all(sigset_t* set){
	if(-1==sigemptyset(set))
		return FALSE;
	if(-1==pthread_sigmask(SIG_SETMASK,set,NULL))
		return FALSE;
	return TRUE;
}

/*
 Interrompe l'esecuzione del programma.
 Stampa un messaggio di errore in base a errno, prima di uscire.
*/
void fatal(const char* errmsg,int errcode){
	int tmp=errcode;
	puts("fatal error");	
	perror(errmsg);	
	exit(tmp);
}
#endif /*_UTILS_C_*/
