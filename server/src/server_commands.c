#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../include/server_commands.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"

void addElemToOnlineUserList(struct onlineUser** online_user_list, char* username, char* port) {
    
    // aggiungo il nuovo utente in coda alla lista
    struct onlineUser* new_user;
    struct onlineUser* q;
    struct onlineUser* p;
    int len;

    for(q = *online_user_list; q != NULL; q = q->next) {
        p = q;
    }
    // inizializzo il nuovo utente
    new_user = malloc(sizeof(struct onlineUser));

    // inizializzo i dati del nuovo utente
    len = strlen(username);
    new_user->username = malloc(len + 1);
    strncpy(new_user->username, username, len);
    new_user->username[len] = '\0';

    len = strlen(port);
    new_user->port = malloc(len + 1);
    strncpy(new_user->port, port, len);
    new_user->port[len] = '\0';

    new_user->next = NULL;
    if(q == *online_user_list)
        *online_user_list = new_user;
    else 
        p->next = new_user;

    return;
}

void delOnlineUserList(struct onlineUser** online_user_list) {
    
    struct onlineUser *del_user;
    while(*online_user_list != NULL) {
        del_user = (*online_user_list)->next;
        free((*online_user_list)->username);
        free((*online_user_list)->port);
        free(*online_user_list);
        *online_user_list = del_user;
    }
    printf("Lista dei messaggi pendenti eliminata con successo!\n");
}

void printOnlineUserList(struct onlineUser** online_user_list) {
    
    printf("PUNTATORE: %p\n", *online_user_list);
    if(*online_user_list == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    struct onlineUser* elem = *online_user_list;
    while(elem != NULL) {
        printf("\nUSERNAME: %s\n", elem->username);
        printf("PORT: %s\n", elem->port);

        /*printf("\nUSERNAME2: %s\n", (*online_user_list)->username);
        printf("PORT2: %s\n", (*online_user_list)->port);*/
        elem = elem->next;
    }
    printf("FINE\n");
    return;
}

void delUserFromOnlineUserList(struct onlineUser** online_user_list, char* username) {
    
    int len;

    if(*online_user_list == NULL) {
        return;
    }

    len = (strlen((*online_user_list)->username) > strlen(username))? strlen((*online_user_list)->username):strlen(username);
    if(!strncmp((*online_user_list)->username, username, len)) {
        struct onlineUser* elem = *online_user_list;
        *online_user_list = (*online_user_list)->next;
        
        // elimino la memoria allocata per il messaggio
        free(elem->username);
        free(elem->port);

        delUserFromOnlineUserList(online_user_list, username);
    } else {
        delUserFromOnlineUserList(&(*online_user_list)->next, username);
    }
}

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