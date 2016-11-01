#include"utils.c"
#include"queue.c"

int	args_ok(int,char**,int*,int*);
int	create_server_socket(int,server_t*);
void* worker(void*);
int estimate(queue_t*);
int set_signal_handler(sigset_t*,struct sigaction*);
void sigquit_handler(int);

server_t myself;

/*
	argomenti: id e fd della pipe con supervisor
*/
int main(int argc,char**argv){
	int id,c_skt,to_super;	
	int* args;
	pthread_t tid;
	sigset_t sigset;
	struct sigaction sact;	
	
	/*init*/
	if(!set_signal_handler(&sigset,&sact))
		fatal("server signal handler",errno);

	if(!args_ok(argc,argv,&id,&to_super))
		fatal("server args",errno);
	
	init_server_t(&myself,id);

	if(!create_server_socket(id,&myself))
		fatal("server socket",errno);

	printf("SERVER %d ACTIVE\n",myself.id);	
	
	/*main loop (dyspatcher)*/
	while(1){
		if(-1==(c_skt=accept(myself.socket_fd,NULL,0)))
			switch(errno){
				case EINTR:/*la unlink dell'handler di SIGQUIT interrompe la accept, non devo stampare errore*/
					return 0;
				default:
					fatal("server accept",errno);
			}			
		
		
		printf("SERVER %d CONNECT FROM CLIENT\n",myself.id);

		/*creo thread worker*/
		args=malloc(2*sizeof(int));
		args[0]=c_skt;	/*per evitare race condition su arg*/
		args[1]=to_super;
		if(pthread_create(&tid,NULL,worker,(void*)args))
			fatal("server thread create",errno);
	}
	return 0;
}

/*
 Thread per trattare una connessione.
*/
void* worker(void*args){
	int cskt	=((int*)args)[0];
	int to_super=((int*)args)[1];	
	int res,est;
	long int arr;
	uint64_t cid;
	queue_t* timeq=malloc(sizeof(queue_t));
	struct timespec ts;
	secret_t est_msg;
	
	initq(timeq,Q_INIT);
	free((int*)args);
	
	/*leggo finche' ho da leggere*/
	while((res=read(cskt,&cid,MSG_BUF_MAX))){		
		if(res!=MSG_BUF_MAX)
			fatal("server read",errno);
		
		if(-1==clock_gettime(CLOCK_REALTIME,&ts))
				fatal("server get time",errno);

		cid=ntohll(cid);
		arr=ts_to_msec(ts);
		timeq=enqueue(timeq,arr);/*salvo il tempo di arrivo*/
		
		printf("SERVER %d INCOMING FROM %"PRIX64" @ %ld\n",myself.id,cid,arr);
	}

	/*calcolo e invio i risultati al supervisor, solo se ho una stima decente*/
	if(-1!=(est=estimate(timeq))){
		printf("SERVER %d CLOSING %"PRIX64" ESTIMATE %d\n",myself.id,cid,est);
		
		est_msg.id=cid;
		est_msg.secret=est;

		if(-1==write(to_super,&est_msg,sizeof(secret_t)))
			fatal("server write",errno);
	}
	/*passo e chiudo*/
	destroyq(timeq);
	free(timeq);
	return NULL;
}

/*
 Gestore di SIGQUIT.
*/
void sigquit_handler(int n){
	if(-1==unlink(myself.address.sun_path))
		fatal("unlink socket",errno);
}

/*
 Imposta la politica di gestione dei segnali.
 Ignoro tutto tranne SIGQUIT.
*/
int set_signal_handler(sigset_t* set,struct sigaction* sa){	
	if(-1==sigfillset(set))
		return FALSE;

	if(-1==sigdelset(set,SIGQUIT))
		return FALSE;

	if(-1==pthread_sigmask(SIG_SETMASK,set,NULL))
		return FALSE;

	memset(sa,0,sizeof(struct sigaction));
	sa->sa_handler=sigquit_handler;
	if(-1==sigaction(SIGQUIT,sa,NULL))
		return FALSE;

	return TRUE;
}

/*
 Stima il secret del client:
	- calcolo i tempi di interarrivo.
	- prendo quello minimo
 (se ho ricevuto 2 segnali consecutivi dallo stesso client, ho la stima esatta)
*/
int estimate(queue_t* times){
	long int prec,tmp;
	int est=SECRET_MAX;
	
	if(times->size==0)/*non conosco neanche il client che si e' connesso, non stimo nulla*/
		return -1;
	if(times->size==1)/*stima inattendibile, ma almeno ho un valore e qualunque altra stima*/
		return est;   /*migliore coprira' questa (poco male)*/
		
	times=dequeue(times,&tmp);
	prec=tmp;

	while(times->size>0){
		times=dequeue(times,&tmp);
		if(est>tmp-prec)
			est=tmp-prec;
		prec=tmp;
	}
	return est;
}

/*
  Crea il socket del server.
*/
int create_server_socket(int id,server_t* server){

	if(-1==(server->socket_fd=socket(AF_UNIX,SOCK_STREAM,0)))
			return FALSE;
	if(-1==bind(server->socket_fd,(struct sockaddr*)&server->address,sizeof(server->address)))
			return FALSE;
	if(-1==listen(server->socket_fd,SOMAXCONN))
			return FALSE;
	
	return TRUE;/*e' stata dura, ma ce l'abbiamo fatta!*/
}

/*
  Controlla che gli argomenti siano corretti.
*/
int args_ok(int argc,char**argv,int*id,int*pipe){
	if(argc!=3){
		errno=EINVAL;
		return FALSE;	
	}
	*id  =atoi(argv[1]);
	*pipe=atoi(argv[2]);
	return TRUE;
}
