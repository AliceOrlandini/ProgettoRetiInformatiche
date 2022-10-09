#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include "./../include/server_commands.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"


/* 
    Stampa i comandi che il server ha a disposizione
    con una breve descrizione del funzionamento.
*/
void printCommands() {
    printf("\nI comandi disponibili sono:\n1) help --> mostra i dettagli dei comandi.\n2) list --> mostra un elenco degli utenti connessi.\n3) esc  --> chiude il server.\n");
    return;
}

/* 
    Stampa più info dei comandi che 
    il server ha a disposizione.
*/
void help() {
    printf("\nEcco le info dei comandi:\n-list: mostra l'elenco degli utenti connessi nel formato \"username * timestamp * porta\".\n-esc: termina il server.\n\n");
    return;
}

/* 
    Mostra l'elenco degli utenti connessi. Per fare ciò, scorre
    il file db_users.txt e controlla se il campo timestamp_logout
    è impostato a NULL. In caso affermativo, stampa a schermo
    l'username, il timestamp_login e la porta. 
*/
void list() {
    
    int ret;
    FILE* fp;
    char file_line[64];
    char* username;
    char* password;
    char* port;
    char* timestamp_login;
    char* timestamp_logout;

    // apro il file db_users.txt in lettura
    fp = fopen("./server/files/db_users.txt", "r"); 
    if(fp == NULL) { printf("Error0 chat\n"); return; }

    // se l'utente è online allora avrà campo NULL nel file
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        
        // ricavo i dati dell'utente
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");
        timestamp_logout = strtok(NULL, " ");

        // controllo se il timestamp_logout è NULL
        if(!strncmp(timestamp_logout, "NULL", 4)) {
            printf("\n%s * %s * %s", username, timestamp_login, port);
        }
    }
    printf("\n");

    // chiudo il file
    fclose(fp);
    return;
}

/* 
    Esegue la disconnessione del socket dedicato
    ad ascoltare le richieste provenienti dai device.
    Inoltre, se ci sono processi figli attivi li elimina.
*/
void esc(int* server_sd, struct child** child_list) {
    
    int ret; 

    // scorro la lista dei processi figli e 
    // se quel processo ancora esiste lo termino
    ret = killChildren(child_list);
    if(ret == -1) { return; }
    
    // se la kill è avvenuta elimino la lista dei processi figli
    delChildList(child_list); 
    
    // chiamo la funzione per disconnettere il socket
    ret = disconnect_to(server_sd);
    if(ret == 0) { printf("Server disconnesso con successo!\n"); }
    
    // termino anche il processo padre
    exit(0);
}

/* 
    Verifica che il comando sia valido ed 
    esegue della funzione corrispondente.
*/
void executeServerCommand(char* buffer, int* sd, struct child** child_list) {

    char server_command[SERVER_COMMAND_SIZE];
    sscanf(buffer, "%s", server_command);

    // controllo che il comando sia valido 
    if(strcmp("help", server_command) && strcmp("list", server_command) && strcmp("esc", server_command)) {
        
        printf("\nComando non valido.");
        
        // mostro di nuovo i comandi disponibili
        printCommands();
        return;
    }

    // eseguo la funzione corrispondente al comando
    if(!strcmp("help", server_command)) {
        help();
        printCommands();
    }
    else if(!strcmp("list", server_command)) {
        list();
        printCommands();
    }
    else {
        esc(sd, child_list);
    }
    return;
}