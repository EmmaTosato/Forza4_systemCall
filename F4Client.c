
/************************************ 
*Matricola: VR446676
*Nome e cognome: Emma Tosato 
*Data di realizzazione: 24/01/2021 
*Titolo esercizio : F4Client.c
*************************************/


#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "informazioni.h"


/* VARIABILI GLOBALI */
struct Shm * punt_Shm;
struct Matrix * punt_Matrix;
int fd_info;
int fd_sem;
int fd_matrice;


/* FUNZIONE PER CONTROL-C */
//Se il Client termina
void segnale_C(int sig){
   printf("\033[1;31m\n\nOPS...\nHai deciso di abbandonare la partita\033[0m\n\n");
   if (getpid() == punt_Shm -> pid1){
	//Se sono il giocatore 1, invio il segnale SIGUSR1 al server
	kill(punt_Shm -> pid_server, SIGUSR1);
	//Dopodichè posso terminare
	exit(1);
   }
   else if (getpid() == punt_Shm -> pid2){
        //Se sono il giocatore 2, invio il segnale SIGUSR2 al server
        kill(punt_Shm -> pid_server, SIGUSR2);
	//Dopodichè posso terminare
	exit(1);
   }
}

void fine_client(int sig){
	printf("\n\n\033[1;36mHai vinto a tavolino, perchè l'altro giocatore ha abbandonato la partita.\033[0m\n\n");
	exit(1);
}


void fine_server(int sig){
	printf("\n\n\033[1;36mIl server ha abbandonato.\nLa partita termina senza vincitori\033[0m\n\n");
	//De alloco ed elimino ipcs
	shmdt(punt_Shm);
        shmctl (fd_info , IPC_RMID, NULL);
        shmdt(punt_Matrix);
        shmctl (fd_matrice , IPC_RMID, NULL );
        semctl(fd_sem, 0, IPC_RMID); 
	exit(1);
}

//Funzione per la stampa della matrice
void stampa(struct Matrix * punt_Matrix, struct Shm * punt_Shm){
	int i, j;
        for (i = 0; i < punt_Shm -> r ; i++){
                for(j = 0; j < punt_Shm -> c ; j++){
                        printf("------");
                }
                printf("-\n");
                for(j = 0; j < punt_Shm -> c ; j++){
                        printf("|  %c  ", punt_Matrix ->matrix[i][j]);
                }
                printf("|\n");
        }
        for(j = 0; j <  punt_Shm -> c ; j++){
                        printf("------");
        }
        printf("-\n");  

}



int main(int argc, char * argv[]){
   /* GUIDA */
   if ( argc < 2 ){
        printf("\033[1;36m\n\nGUIDA\033[0m\nInserisci da linea di comando il tuo nome utente\nOra il programma terminerà e dovrai eseguirlo di nuovo\n\n");
        exit(1);
   }
 

   /* DEFINIZIONI */
   size_t shmsize = sizeof(struct Shm);
   char controllo_computer;
   void segnale_C(int sig);
   void fine_client(int sig);
   void fine_server(int sig);
   void stampa(struct Matrix * punt_Matrix, struct Shm * punt_Shm);

   /* SEGNALI */
   signal(SIGINT,segnale_C);
   signal(SIGXFSZ,fine_client);
   signal(SIGTSTP,fine_server);


   /* MEMORIA CONDIVISA INFORMAZIONI */
   fd_info = shmget(key1, shmsize , 0777);
   if ( fd_info == -1){
      perror("Errore nella creazione della memoria condivisa");
   }

   punt_Shm  = (struct Shm *)shmat(fd_info,NULL,0);

   
   //Contatore per vedere quante volte è stato lanciato il Client
   if ( *(argv + 1) != NULL){
    punt_Shm ->contaC+=1;
   }

   //Memorizzo nome giocatore 
   strcpy(punt_Shm -> nome_utente , argv[1]);

   //Se il giocatore è il computer
   if ( *(argv + 2) != NULL){
	controllo_computer = * argv[2];
	if ( controllo_computer == '*'){
		if (punt_Shm ->contaC == 1){
			punt_Shm -> computer1 ++;
		}
		else{
			punt_Shm -> computer2 ++;
		}
	}
   }
   

   //Prelevo il PID del processo giocatore attuale
   punt_Shm -> pid = getpid();


   /* SEMAFORI  */
   //Dichiarazione  semafori
   fd_sem = semget(key3, 3, 0777);
   if ( fd_sem < 0 ) {
     perror("Errore nella creazione del semaforo");
     exit(1);
   }

   struct sembuf sem;
   sem.sem_flg = 0;


   if ( punt_Shm -> contaC  == 1 ){
     printf("\n\n\033[1;35mSei il primo giocatore!\nIl tuo simbolo è: %c\033[0m\n\n\n\033[1;33mOra attendiamo l'arrivo di un altro\033[0m\n",punt_Shm ->simbolo1);
	//sem.sem_flg = 0;
     	sem.sem_op  = 1;
     	sem.sem_num = 0; 
     	semop(fd_sem, &sem, 1);
     	sem.sem_num = 1;
     	sem.sem_op  = -1;
     	semop(fd_sem, &sem, 1);
   }
   else {
     printf("\n\n\033[1;32mSei il secondo giocatore!\nIl tuo simbolo è: %c\033[0m\n",punt_Shm ->simbolo2);
     	//sem.sem_flg = 0;
     	sem.sem_op  = 1;
     	sem.sem_num = 0;
     	semop(fd_sem, &sem, 1);
     	sem.sem_num =  2;
     	sem.sem_op  = -1;
     	semop(fd_sem, &sem, 1);
   }


   /* MEMORIA CONDIVISA PER LA MATRICE */

   //Definizione variabili
   size_t matrix_size = sizeof(struct Matrix);
   int scelta;

   //Inizializzazione memoria condivisa matrice
   fd_matrice = shmget(key2, matrix_size, 0777);
   if ( fd_matrice == -1) {
      perror("Errore nella creazione della memoria condivisa");
      exit(1);
   }  
   
   punt_Matrix = (struct Matrix *) shmat(fd_matrice,NULL,0);
  

   /* MOSSE GIOCATORI */
   while ( punt_Shm -> end_game != 1 ){

     //Assegno i simboli
     char simbolo;
     if ( getpid() == punt_Shm -> pid1 ){
        simbolo = punt_Shm -> simbolo1;
     }
     else{
        simbolo = punt_Shm -> simbolo2;
     }

     //Stampo matrice
     printf("\n\n\033[1;36m*** TABELLA DI GIOCO PRIMA DELLA MOSSA ***\033[0m\n\n");
     stampa(punt_Matrix, punt_Shm);


     //Inserisci colonna
     //I primi if sono per giocatore-computer
     if ( punt_Shm -> computer1 == 1 && getpid() == punt_Shm -> pid1 ){
	srand(time(0));
	scelta = 1 + rand() % punt_Shm -> c;
	printf("\nIl computer ha scelto la colonna\n");
     }
     else if ( punt_Shm -> computer2 == 1 && getpid() == punt_Shm -> pid2 ){
	srand(time(0));
        scelta = 1 + rand() % punt_Shm -> c;
	printf("\nIl computer ha scelto la colonna\n");
     }
     else{ 
     	do{
        	printf("\nInserisci la colonna (che non deve essere minore di %d): ",  punt_Shm ->c);
        	scanf("%i", &scelta);
     	} while(scelta > punt_Shm ->c || scelta < 0);
     } 

     scelta-=1;    //Le righe dei for partono da 0 (la quarta colonna è la terza)

     //Controllo colonna
     int c_piena = 0;
     int uscita = 1;
     int ultima_r = punt_Shm ->r - 1;
     if ( punt_Matrix-> matrix[0][scelta] == ' ' ) {
       for ( int i = ultima_r ; i >= 0  && uscita == 1; i-- ){
         if (punt_Matrix-> matrix[i][scelta] != punt_Shm ->simbolo1 && punt_Matrix-> matrix[i][scelta] != punt_Shm ->simbolo2){
            punt_Matrix-> matrix[i][scelta] =  simbolo;
            uscita = 0;		//così esco dal for
         }
       }
     }
     else{
       printf ("\n\033[1;31mLa colonna selezionata è piena!\033[0m\n");
     }   

     //Stampo matrice
     printf("\n\n\033[1;36m*** TABELLA DI GIOCO DOPO LA MOSSA ***\033[0m\n");
     stampa(punt_Matrix, punt_Shm);

      /* SEMAFORI */

      //Giocatore 1
      if ( getpid() == punt_Shm -> pid1 ){

	//Sblocco il server
        sem.sem_flg = 0;
        sem.sem_op  = 1;
        sem.sem_num = 0;

	semop(fd_sem, &sem, 1);

        //Blocco il client
        sem.sem_num = 1;
        sem.sem_op  = -1;

        semop(fd_sem, &sem, 1);       
      }

      //Giocatore 2
      else if ( getpid() == punt_Shm -> pid2 ){

	//Sblocco il server 
	sem.sem_flg = 0;
        sem.sem_op  = 1;
        sem.sem_num = 0;
	
        semop(fd_sem, &sem, 1);
 
        //Blocco il client
        sem.sem_num = 2;
        sem.sem_op  = -1;

        semop(fd_sem, &sem, 1);

     }
 }//fine while


  if ( getpid() == punt_Shm -> pid1 ){
	
	/* ANNUNCIO LA VITTORIA AL CLIENT */
        if ( punt_Shm -> winner1 == 1 ){
		printf("\n\033[1;35mHai vinto %s!\033[0m\n\n", punt_Shm -> nome_giocatore1);
	}
	else if (punt_Shm -> winner1 == 0 ){
                printf("\n\033[1;35mHai perso %s...\033[0m\n\n", punt_Shm -> nome_giocatore1);
        }
	else{
		printf("\n\033[1;35mOps %s! La partita è finita in parità.\033[0m\n\n", punt_Shm -> nome_giocatore1);
	}
  }
  else if  ( getpid() == punt_Shm -> pid2 ){
	
	/* ANNUNCIO LA VITTORIA AL CLIENT */
        if ( punt_Shm -> winner2 == 1 ){
                printf("\n\033[1;32mHai vinto %s!\033[0m\n\n", punt_Shm -> nome_giocatore2);
        }
        else if (punt_Shm -> winner2 == 0 ){
                printf("\n\033[1;32mHai perso %s...\033[0m\n\n", punt_Shm -> nome_giocatore2);
        }
	else{
		printf("\n\033[1;32mOps %s! La partita è finita in parità.\033[0m\n\n", punt_Shm -> nome_giocatore2);
	}
   //Gestione parità


  }
  return (0);
}

