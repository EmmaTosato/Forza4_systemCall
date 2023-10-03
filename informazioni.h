
/************************************ 
*Matricola: VR446676
*Nome e cognome: Emma Tosato 
*Data di realizzazione: 24/01/2021 
*Titolo esercizio : informazioni.h
*************************************/


#define N 30
#define num 15
#define key1 116864587 
#define key2 33641803
#define key3 50419019


/* SHM INFORMAZIONI */
struct Shm {
    //Contatore
    int contaC;
    //Righe e colonne
    int r ;
    int c ;
    //Simboli
    char simbolo1 ;
    char simbolo2 ;
    //Chiavi
    //long key_info;
    //long key_m;
    //Nome utente
    char nome_utente[N];
    char nome_giocatore1[N];
    char nome_giocatore2[N];
    //Pid processi client
    int pid;
    int pid1;
    int pid2;
    int pid_server;
    //Variabile per il controllo della fine partita
    int end_game;
    //Variabili vincitori
    int winner1;
    int winner2;
    //Variabile per il controllo del giocatore-computer
    int computer1;
    int computer2;
};

/* SHM MATRICE */
struct Matrix {
     char matrix[num][num];
};

