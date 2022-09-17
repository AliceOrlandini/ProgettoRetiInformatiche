#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../include/server_commands.h"
#include "./../include/server_consts.h"

/* stampa i comandi che il server ha a disposizione */
void printCommands() {
    printf("comandi disponibili:\n1) help --> mostra i dettagli dei comandi\n2) list --> mostra un elenco degli utenti connessi\n3) esc  --> chiude il server\n");
    return;
}

/* stampa le info dei comandi disponibili */
void help() {
    printf("\nEcco le info dei comandi:\n-list: mostra l'elenco degli utenti connessi nel formato \"username*timestamp*porta\".\n-esc: termina il server.\n\n");
    return;
}

/* */
void list() {
    
}

/* */
void esc() {
    
}

/* verifica che il comando sia valido ed esecuzione della funzione corrispondente */
void executeServerCommand(char* buffer) {

    char server_command[SERVER_COMMAND_SIZE];
    sscanf(buffer, "%s", server_command);

    // Controllo che il comando sia valido 
    if(strcmp("help", server_command) && strcmp("list", server_command) && strcmp("esc", server_command)) {
        printf("\nComando non valido, ecco i ");
        // mostro di nuovo i comandi disponibili
        printCommands();
        return;
    }

    // Eseguo la funzione corrispondente al comando
    if(!strcmp("help", server_command)) {
        help();
        printCommands();
    }
    else if(!strcmp("list", server_command)) {
        list();
        printCommands();
    }
    else
        esc();
    return;
}