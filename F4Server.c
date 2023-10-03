/************************************ 
*Matricola: VR446676
*Nome e cognome: Emma Tosato 
*Data di realizzazione: 24/01/2021 
*Titolo esercizio : F4Server.c
*************************************/


#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include "informazioni.h"


/* VARIABILI GLOBALI */
  struct Shm * punt_Shm;
  struct Matrix * punt_Matrix;
  //Variabili per il control-c 
  int conta_ctrlC; 
  int status_ctrlC;
  //Variabili per ipcs
  int fd_sem;
  int fd_info;
  int fd_matrice;


/* FUNZIONE ARBITRAGGIO */
int arbitraggio(struct Matrix * punt_Matrix, struct Shm * punt_Shm){
    int piena = 0;
    int conta_O1 = 0;
    int conta_O2 = 0;
    int esci = 0;
    
    
    /* Controllo se la matrice è piena */
    int i=0;
    for(int j = 0; j < punt_Shm -> c && esci == 0; j++){
                if (punt_Matrix -> matrix[i][j] != ' ' )
                        piena++;
    }

    if ( piena == punt_Shm -> c){
        printf("\n\n\033[1;36mLa partita è finita in parità perchè la tabella di gioco è piena\033[0m\n\n");
        punt_Shm -> winner1 = 2;
        punt_Shm -> winner2 = 2;
        return 1;
    }


    /* CONTROLLO IN ORIZZONTALE */	    
    for(int i = 0; i < punt_Shm -> r; i++){
        for(int j = 0; j < (punt_Shm -> c - 1) && esci == 0; j++){
                if (punt_Matrix -> matrix[i][j] != ' ' &&  punt_Matrix -> matrix[i][j] == punt_Shm -> simbolo1 && punt_Matrix -> matrix[i][j] == punt_Matrix -> matrix[i][j+1]){
                        conta_O1 += 1;
                }
                else if (punt_Matrix -> matrix[i][j] != ' ' &&  punt_Matrix -> matrix[i][j] == punt_Shm -> simbolo2 && punt_Matrix -> matrix[i][j] == punt_Matrix -> matrix[i][j+1]){
                        conta_O2 += 1;
                }   
		else{
			conta_O1 = 0;
			conta_O2 = 0;
		}
		if (conta_O1 == 3 || conta_O2 == 3){
			esci = 1;

		}
        }
	if (esci == 1)
		break;
	conta_O1 = 0;
        conta_O2 = 0;
    }

    if (conta_O1 == 3){
        printf("\n\n\033[1;35mABBIAMO UN VINCITORE!\nIl giocatore %s ha vinto!\033[0m\n", punt_Shm -> nome_giocatore1);
        punt_Shm -> winner1 ++;   
        return 1;
    }
    else if (conta_O2 == 3){
        printf("\n\n\033[1;32mABBIAMO UN VINCITORE!\nIl giocatore %s ha vinto!\033[0m\n", punt_Shm -> nome_giocatore2);
        punt_Shm -> winner2 ++;   
        return 1;
    }



    esci = 0;
    int conta_V1 = 0;
    int conta_V2 = 0;

    /* CONTROLLO IN VERTICALE */          
    for(int j = 0; j < punt_Shm -> c ; j++){
        for(int i = 0; i < (punt_Shm -> r - 1) && esci == 0; i++){
                if (punt_Matrix -> matrix[i][j] != ' ' &&  punt_Matrix -> matrix[i][j] == punt_Shm -> simbolo1 && punt_Matrix -> matrix[i][j] == punt_Matrix -> matrix[i+1][j]){
                        conta_V1 += 1;
                }
                else if (punt_Matrix -> matrix[i][j] != ' ' &&  punt_Matrix -> matrix[i][j] == punt_Shm -> simbolo2 && punt_Matrix -> matrix[i][j] == punt_Matrix -> matrix[i+1][j]){

                        conta_V2 += 1;
                }   
                else{
                        conta_V1 = 0;
                        conta_V2 = 0;
                }
                if (conta_V1 == 3 || conta_V2 == 3){
                        esci = 1;
                }
        }
	if (esci == 1)
                break;
	conta_V1 = 0;
	conta_V2 = 0;
    }


    if (conta_V1 == 3){
        printf("\n\n\033[1;35mABBIAMO UN VINCITORE!\nIl giocatore %s ha vinto!\033[0m\n", punt_Shm -> nome_giocatore1);
        punt_Shm -> winner1 ++;   
        return 1;
    }
    else if (conta_V2 == 3){
        printf("\n\n\033[1;32mABBIAMO UN VINCITORE!\nIl giocatore %s ha vinto!\033[0m\n", punt_Shm -> nome_giocatore2);
        punt_Shm -> winner2 ++;   
        return 1;
    }

    return 0;
}


/* FUNZIONE PER CONTROL-C */
/* SE TERMINA IL SERVER */
void segnale_C(int sig){
   conta_ctrlC++;
   if (conta_ctrlC == 1) {
    	printf("\033[1;31m\n\nATTENZIONE\033[0m\033[0;31m\nSe la tua prossima mossa sarà ancora la pressione di ^C, il programma terminerà.\nHai 7 secondi per decidere, se non fai niente ritorni da dove hai interrotto.\n\033[0m");
        sleep(7);
   }
   //Rimozione memorie condivise 
   else if (conta_ctrlC == 2){
        printf("\n\n\033[1;36mIl server ha terminato.\033[0m\n\n\n");
	if (punt_Shm -> nome_giocatore1[0] != '\0' && punt_Shm -> nome_giocatore2[0] != '\0'){
		//Invio segnali ai client
        	kill(punt_Shm -> pid1,SIGTSTP);
        	kill(punt_Shm -> pid2,SIGTSTP);
		signal(SIGTSTP, SIG_DFL);
	}
	else if (punt_Shm -> nome_giocatore1[0] != '\0'){
                //Invio segnali ai client
                kill(punt_Shm -> pid1,SIGTSTP);
                signal(SIGTSTP, SIG_DFL);
        }
	//Termino
	signal(SIGINT,SIG_DFL);
	exit(1);
  } 
}

/* SE TERMINANO I CLIENT */
//Se termina il giocatore 1
void signal_user1(int sig){
	if (punt_Shm -> nome_giocatore2[0] != '\0' ){
		printf("\n\033[1;35mHa abbandonato il giocatore %s, la vittoria andrà a %s.\033[0m\n\n", punt_Shm -> nome_giocatore1, punt_Shm -> nome_giocatore2);
		//Mando segnale al client 2
		kill(punt_Shm -> pid2, SIGXFSZ);
        	signal(SIGXFSZ, SIG_DFL);
	}
	else 
		printf("\n\033[1;35mIl giocatore %s ha abbandonato, la partita non è mai iniziata.\033[0m\n\n",punt_Shm -> nome_giocatore1);
	//Dealloco ed elimino ipcs
        shmdt(punt_Shm);
        shmctl (fd_info , IPC_RMID, NULL );
        shmdt(punt_Matrix);
        shmctl (fd_matrice , IPC_RMID, NULL );
        semctl(fd_sem, 0, IPC_RMID); 
        //Termino il server
	exit(1);
}

//Se termina il giocatore 2
void signal_user2(int sig){
	printf("\n\n\033[1;32mHa abbandonato il giocatore %s, la vittoria andrà a %s.\033[0m\n\n", punt_Shm -> nome_giocatore2, punt_Shm -> nome_giocatore1);
	//Mando segnale al client 1
	kill(punt_Shm -> pid1, SIGXFSZ);
        signal(SIGXFSZ, SIG_DFL);
        //Dealloco ed elimino ipcs
        shmdt(punt_Shm);
        shmctl (fd_info , IPC_RMID, NULL );
        shmdt(punt_Matrix);
        shmctl (fd_matrice , IPC_RMID, NULL );
        semctl(fd_sem, 0, IPC_RMID); 
        //Termino i processi 
	exit(1);
}



/* MAIN */
int main(int argc, char * argv[]){

   /* DEFINIZIONI VARIABILI E FUNZIONI */
   size_t shmsize = sizeof(struct Shm);
   size_t matrix_size = sizeof(struct Matrix);
   int arbitraggio(struct Matrix * punt_Matrix , struct Shm * punt_Shm);
   void segnale_C(int sig);
   void signal_user1(int sig);
   void signal_user2(int sig);


   /* INIZIALIZZAZIONI */
   conta_ctrlC = 0; 
   status_ctrlC = 0;

   /* SEGNALI */
   signal(SIGINT,segnale_C);
   signal(SIGUSR1,signal_user1);
   signal(SIGUSR2,signal_user2);

   /* LEGGO I PARAMETRI DA TASTIERA */
   //Controllo i parametri da tastiera
   if ( argc != 5 ) {
	printf("\033[1;36m\n\nGUIDA\nDevi inserire da linea di comando:\033[0m\n1) Il numero di righe della matrice\n2) Il numero di colonne della matrice\n3) I simboli con cui giocare\n4) Righe e colonne non possono essere minori di 5 e maggiori di 15\nOra terminerà il programma e dovrai eseguirlo ancora.\n\n\n");
        exit(1);
   }
 

   /* CREO LA MEMORIA CONDIVISA PER LE INFORMAZIONI */
   fd_info = shmget(key1, shmsize , 0777 | IPC_CREAT);
   if ( fd_info == -1) {
      perror("Errore nella creazione della memoria condivisa");
      exit(1);
   }
   
   punt_Shm  = (struct Shm *)shmat(fd_info,NULL,0);


   /* INIZIALIZZAZIONI INFORMAZIONI */
   punt_Shm -> pid_server = getpid();
   punt_Shm -> r = atoi(argv[1]);;
   punt_Shm -> c = atoi(argv[2]);;
   punt_Shm -> simbolo1 = * argv[3];
   punt_Shm -> simbolo2 = * argv[4];
   punt_Shm -> contaC = 0;
   punt_Shm -> winner1 = 0;
   punt_Shm -> winner2 = 0;
   punt_Shm -> computer1 = 0;
   punt_Shm -> computer2 = 0;


   /* CONTROLLO NUMERO RIGHE E COLONNE  */
   if (punt_Shm -> r < 5 ){
	do{
		int riga;
		printf("\033[1;31m\nHai inserito un numero di righe e/o colonne minore di 5.\nRiprova\033[0m");
                printf("\n\nInserisci riga: ");
                scanf("%i", &riga);
                punt_Shm -> r = riga;
   	}while (punt_Shm -> r < 5);
   }
   if ( punt_Shm -> c < 5 ){
	do{
                int colonna;
		printf("\033[1;31m\nHai inserito un numero di righe e/o colonne minore di 5.\nRiprova\033[0m");
                printf("\n\nInserisci colonna: ");
                scanf("%i", &colonna);
                punt_Shm -> c = colonna;
        }while (punt_Shm -> c < 5);
   }

   

   /* STAMPO INFORMAZIONI */
   printf("\033[1;36m\nBevenuto nel gioco\n\033[0m");
   printf("\033[0;36mNumero righe della matrice:\033[0m %d \n",punt_Shm ->r);
   printf("\033[0;36mNumero colonne della matrice:\033[0m %d \n",punt_Shm ->c);
   printf("\033[0;36mSimboli :\033[0m  %c e %c \n",punt_Shm ->simbolo1, punt_Shm ->simbolo2);



   /* CREO LA MEMORIA CONDIVISA PER LA MATRICE */

   fd_matrice = shmget(key2, matrix_size, 0777 | IPC_CREAT);
   if ( fd_matrice == -1) {
      perror("Errore nella creazione della memoria condivisa");
      exit(1);
   }

   punt_Matrix = (struct Matrix *) shmat(fd_matrice,NULL,0);
   
   //Inizializzo celle matrice a spazio
   for (int i = 0; i < punt_Shm ->r; i++){
                for(int j = 0; j < punt_Shm ->c; j++){
                        punt_Matrix->matrix[i][j] = ' ';
                }
    }

  
   /* SEMAFORI */
   //Creazione semafori
   fd_sem = semget(key3, 3, IPC_CREAT | 0777);
   if ( fd_sem < 0 ){
     	perror("Errore nella creazione del semaforo");
     	exit(1);
   }
 
   struct sembuf sem;
   sem.sem_op  = -1;
   sem.sem_num = 0;
   sem.sem_flg = 0;

 
   do{
	status_ctrlC = conta_ctrlC;
   	//Attesa giocatori
   	printf("\nAttendiamo giocatore 1...\n");
   	semop(fd_sem, &sem, 1);

   }while (status_ctrlC !=  conta_ctrlC && punt_Shm -> nome_giocatore1[0] == '\0');
   conta_ctrlC = 0;

   //Memorizzazione pid e nome utente in memoria condivisa
   strcpy(punt_Shm -> nome_giocatore1, punt_Shm -> nome_utente);
   punt_Shm -> pid1 = punt_Shm -> pid;
   printf("\n\n\033[1;35mHo trovato il giocatore 1!\033[0m\n\033[0;35mNome giocatore 1: \033[0m%s\n\033[0;35mPid: \033[0m%d\n\033[0;35mSimbolo: \033[0m%c\n" , punt_Shm -> nome_giocatore1, punt_Shm -> pid1, punt_Shm -> simbolo1);

   //Se il giocatore è il computer
   if (punt_Shm -> computer1 == 1 ){
	printf("\n\n\033[1;35mHo trovato il giocatore 1!\nE' il computer\n\033[0m\n\033[0;35mNome giocatore 1: \033[0m%s\n\033[0;35mPid: \033[0m%d\n\033[0;35mSimbolo: \033[0m%c\n" , punt_Shm -> nome_giocatore1, punt_Shm -> pid1, punt_Shm -> simbolo1);
   }

   do{
	status_ctrlC = conta_ctrlC;
   	printf("\nAttendiamo giocatore 2...\n");
   	semop(fd_sem, &sem, 1);

   }while (status_ctrlC != conta_ctrlC && punt_Shm -> nome_giocatore2[0] == '\0');
   conta_ctrlC = 0;
   
   //Memorizzazione pid e nome utente in memoria condivisa
   strcpy(punt_Shm -> nome_giocatore2, punt_Shm -> nome_utente);
   punt_Shm -> pid2 = punt_Shm -> pid;
   printf("\n\n\033[1;32mHo trovato il giocatore 2!\033[0m\n\033[0;32mNome giocatore 2: \033[0m%s\n\033[0;32mPid: \033[0m %d\n\033[0;32mSimbolo:\033[0m %c\n" , punt_Shm -> nome_giocatore2, punt_Shm -> pid2, punt_Shm -> simbolo2);

   //Se il giocatore è il computer
   if ( punt_Shm -> computer2 == 1 ){
	printf("\n\n\033[1;35mHo trovato il giocatore 2!\nE' il computer\n\033[0m\n\033[0;35mNome giocatore 2: \033[0m%s\n\033[0;35mPid: \033[0m%d\n\033[0;35mSimbolo: \033[0m%c\n" , punt_Shm -> nome_giocatore2, punt_Shm -> pid2, punt_Shm -> simbolo2);
   }

   printf("\n\n\033[1;36mIL GIOCO E' INIZIATO!\033[0m\n\n\033[1;35mAttendo la mossa del giocatore 1...\033[0m\n");

   /* ARBITRAGGIO PARTITA E TURNI PER LE MOSSE */
   punt_Shm -> end_game = 0; 
   while (punt_Shm -> end_game != 1){

     	//Inizializzo i semafori
     	sem.sem_flg = 0;

     	//Tocca al giocatore 1, prima lo sblocco...
     	sem.sem_op  = 1;
     	sem.sem_num = 1;
	do{
		status_ctrlC = conta_ctrlC;
     		semop(fd_sem, &sem, 1);
	}while(status_ctrlC != conta_ctrlC);
	conta_ctrlC = 0;

     	//Blocco Server
     	sem.sem_op  = -1;
     	sem.sem_num = 0;
	do{
                status_ctrlC = conta_ctrlC;
                semop(fd_sem, &sem, 1);
        }while(status_ctrlC != conta_ctrlC);
	conta_ctrlC = 0;


     	//Giocatore 1 ha inserito, arbitraggio
     	punt_Shm -> end_game = arbitraggio(punt_Matrix, punt_Shm);
	//Se qualcuno ha vinto
     	if ( punt_Shm -> end_game == 1 ){
		printf("\n\033[1;36mIl gioco si è concluso. Ciao !\033[0m\n\n");  
		//Attivo i client per la comunicazione della vittoria ed esco
		sem.sem_op  = 1;
        	sem.sem_num = 1;
		do{
	                status_ctrlC = conta_ctrlC;
        	        semop(fd_sem, &sem, 1);
        	}while(status_ctrlC != conta_ctrlC);
        	conta_ctrlC = 0;
		sem.sem_num = 2;
                do{
                        status_ctrlC = conta_ctrlC;
                        semop(fd_sem, &sem, 1);
                }while(status_ctrlC != conta_ctrlC);
	 	break;
     	}
	//Se nessuno ha ancora vinto
     	else{
		printf("\nNon ci sono ancora vincitori\n");
		printf("\n\033[1;32mAttendo la mossa del giocatore 2...\033[0m\n");
     	}

     	//Tocca al giocatore 2, prima lo sblocco...
     	sem.sem_op  = 1;
     	sem.sem_num = 2;
	do{
                status_ctrlC = conta_ctrlC;
                semop(fd_sem, &sem, 1);
        }while(status_ctrlC != conta_ctrlC);
	conta_ctrlC = 0; 

     	//Blocco Server
     	sem.sem_op  = -1;
     	sem.sem_num = 0;
	do{
                status_ctrlC = conta_ctrlC;
                semop(fd_sem, &sem, 1);
        }while(status_ctrlC != conta_ctrlC);
 	conta_ctrlC = 0;

     	//Giocatore 2 ha inserito, arbitratrggio
     	punt_Shm -> end_game = arbitraggio(punt_Matrix, punt_Shm);
	//Se qualcuno ha vinto
     	if ( punt_Shm -> end_game == 1 ){
		printf("\n\033[1;36mIl gioco si è concluso. Ciao !\033[0m\n\n");		
		//Attivo i client per la comunicazione della vittoria ed esco
                sem.sem_op  = 1;
                sem.sem_num = 1;
		do{
                        status_ctrlC = conta_ctrlC;
                        semop(fd_sem, &sem, 1);
                }while(status_ctrlC != conta_ctrlC);
                conta_ctrlC = 0;
                sem.sem_num = 2;
                do{
                        status_ctrlC = conta_ctrlC;
                        semop(fd_sem, &sem, 1);
                }while(status_ctrlC != conta_ctrlC);
         	//break;
     	}
	//Se nessuno ha ancora vinto
     	else{
		printf("\nNon ci sono ancora vincitori\n");
		printf("\n\033[1;35mAttendo la mossa del giocatore 2...\033[0m\n");
        }
   }


   /* ELIMINO AREE DI MEMORIA E SEMAFORI */
   //LIBERO MEMORIA
   shmdt(punt_Shm);
   shmdt(punt_Matrix);

   //CANCELLA AREA DI MEMORIA
   shmctl (fd_info , IPC_RMID, NULL );
   shmctl (fd_matrice , IPC_RMID, NULL );
   semctl(fd_sem, 0, IPC_RMID); 

   return 0;
}
 
