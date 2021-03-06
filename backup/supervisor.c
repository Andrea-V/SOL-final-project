#include"utils.c"

int args_ok(int,char**,int*);
void* listener(void*);
int set_signal_mask(sigset_t*);
void wait_double_sigint(sigset_t*,int);

/*risorse in sola lettura*/
int** pipes;

/*tabella delle stime (thread-safe)*/
estab_t* tab;

/*
 Argomenti: k server da lanciare.
*/
int main(int argc,char**argv){
	int k,i,pid;
	int* arg;
	char arg1[STR_MAX],arg2[STR_MAX];
	pthread_t tid;
	sigset_t sigset;

	/*init*/
	if(!set_signal_mask(&sigset))/*i thread ereditano la signal mask del loro creatore*/
		fatal("supervisor signal handler",errno);

	tab=malloc(sizeof(estab_t));
	init_tab(tab);	

	if(!args_ok(argc,argv,&k))
		fatal("supervisor args",errno);
		
	pipes=malloc((k+1)*sizeof(int*));/*la posizione 0 e' sprecata (poco male)*/
	for(i=1;i<k+1;i++)
		pipes[i]=malloc(3*sizeof(int));/*la terza posizione conterra' il pid del figlio*/

	printf("SUPERVISOR STARTING %d\n",k);

	/*creo le pipes e forko i server*/
	for(i=1;i<k+1;i++){

		if(-1==pipe(pipes[i]))
			fatal("supervisor pipe",errno);		

		switch((pid=fork())){
			case  0:/*figlio*/
				close(pipes[i][0]);
				sprintf(arg1,"%d",i);
				sprintf(arg2,"%d",pipes[i][1]);
				execl(SERVER_PATH,"server",arg1,arg2,NULL);
				fatal("supervisor exec",errno);
			case -1:/*errore*/
				fatal("supervisor fork",errno);
			default:/*padre*/
				pipes[i][2]=pid;
				close(pipes[i][1]);
				arg=malloc(sizeof(int));
				*arg=i;	/*per evitare race condition su arg*/
				if(pthread_create(&tid,NULL,listener,(void*)arg))
					fatal("supervisor thread create",errno);
		}
	}
	
	/*attendo segnale di chiusura*/
	wait_double_sigint(&sigset,1);
	printf("\nSUPERVISOR EXITING\n");	
	
	/*dico ai servers di chiudersi*/
	for(i=1;i<k+1;i++)
		if(-1==kill(pipes[i][2],SIGQUIT))
			kill(pipes[i][2],SIGKILL);

	/*passo e chiudo*/
	for(i=1;i<k+1;i++)
		free(pipes[i]);
	free(pipes);
	destroy_tab(tab);
	return 0;
}

/*
	Thread, ascolta le stime del server assegnato e aggiorna la tab delle stime.
*/
void* listener(void*args){
	int sid=*((int*)args);
	secret_t est;	
	int res,ss=sizeof(secret_t);

	free((int*)args);

	/*leggo e salvo le stime*/
	while((res=read(pipes[sid][0],&est,ss))){
		if(res!=ss)
			fatal("supervisor read",errno);

		add_new_est(tab,&est);
		printf("SUPERVISOR ESTIMATE %d FOR %"PRIX64" FROM %d\n",est.secret,est.id,sid);
	}
	return NULL;
}
/*
	Aspetta il doppio SIGINT per la chiusura
*/
void wait_double_sigint(sigset_t* sigset,int t){
	int exit=FALSE;
	int alert=FALSE;
	int sig;

	while(!exit){
		if((errno=sigwait(sigset,&sig)))
			fatal("supervisor sigwait",errno);

		if(sig==SIGINT&&!alert){/*primo SIGINT: mi metto in allerta*/			
			print_tab(tab,stderr);			
			alarm(t);
			alert=TRUE;
		}
		else if(sig==SIGINT&&alert)/*secondo SIGINT! esco*/
			exit =TRUE;	
		else if(sig==SIGALRM)/*era un singolo SIGINT, mi rimetto tranquillo*/
			alert=FALSE;
	}
}


/*
 Imposta la politica di gestione dei segnali.
 Ignoro tutti, gestiro' poi SIGINT e SIGALRM con sigwait()
 direttamente nel main, senza handler.
*/
int set_signal_mask(sigset_t* set){
	if(-1==sigfillset(set))
		return FALSE;
	if(-1==pthread_sigmask(SIG_SETMASK,set,NULL))
		return FALSE;
	return TRUE;
}

/*
  Legge e controlla gli argomenti.
*/
int args_ok(int argc,char** argv,int* k){
	if(argc!=2){
		errno=EINVAL;
		return FALSE;	
	}
	*k=atoi(argv[1]);
	return TRUE;
}
