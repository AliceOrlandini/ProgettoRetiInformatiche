#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../include/server_commands.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"

/* 
    Stampa i comandi che il server ha a disposizione 
*/
void printCommands() {
    printf("\nI comandi disponibili sono:\n1) help --> mostra i dettagli dei comandi.\n2) list --> mostra un elenco degli utenti connessi.\n3) esc  --> chiude il server.\n");
    return;
}

/* 
    Stampa le info dei comandi disponibili 
*/
void help() {
    printf("\nEcco le info dei comandi:\n-list: mostra l'elenco degli utenti connessi nel formato \"username*timestamp*porta\".\n-esc: termina il server.\n\n");
    return;
}

/* */
void list() {
    
}

/* 
    Chiude il server
*/
void esc(int* sd) {
    
    int ret; 
    
    ret = disconnect_to(sd);
    if(ret == 0) { printf("Server disconnesso con successo!\n"); }
}

/* 
    Verifica che il comando sia valido ed esecuzione della funzione corrispondente 
*/
void executeServerCommand(char* buffer, int* sd) {

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
        esc(sd);
        exit(0);
    }
        
    return;
}