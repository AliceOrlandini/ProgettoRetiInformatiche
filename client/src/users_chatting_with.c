#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "./../../network/include/network.h"
#include "./../include/users_chatting_with.h"

/*
    Permette di aggiungere un utente alla lista degli 
    utenti online. L'inserimento avviene in testa.
*/
void addElemToChattingWithList(struct usersChattingWith** users_chatting_with, char* username, in_port_t port, int p2p_sd) {
    
    struct usersChattingWith* new_user;
    int len;

    // creo il nuovo utente
    new_user = malloc(sizeof(struct usersChattingWith));

    // inizializzo i dati del nuovo utente
    len = strlen(username);
    new_user->dst_username = malloc(len + 1);
    strncpy(new_user->dst_username, username, len);
    new_user->dst_username[len] = '\0';

    new_user->port = port;
    new_user->p2p_sd = p2p_sd;

    // inserisco in testa
    new_user->next = *users_chatting_with;
    *users_chatting_with = new_user;

    return;
}

/*
    Elimina tutta la lista degli utenti con cui si sta chattando.
*/
void delChattingWithList(struct usersChattingWith** users_chatting_with) {
    
    struct usersChattingWith *del_user;
    while(*users_chatting_with != NULL) {
        del_user = (*users_chatting_with)->next;
        // if((*users_chatting_with)->dst_username != NULL)
        // libero la memoria allocata per l'utente
        free((*users_chatting_with)->dst_username);
        free(*users_chatting_with);
        *users_chatting_with = del_user;
    }
    printf("Lista degli utenti con cui si sta chattando eliminata con successo!\n");
}

/*
    Funzione per stampare il contenuto della lista degli 
    utenti online. Viene usata solo a scopo di debug.
*/
void printChattingWithList(struct usersChattingWith** users_chatting_with) {
    
    struct usersChattingWith* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    elem = *users_chatting_with;
    while(elem != NULL) {
        printf("\nusername: %s\n", elem->dst_username);
        printf("port: %d\n", elem->port);
        elem = elem->next;
    }
    printf("Fine lista.\n");
    return;
}

/*
    Permette di eliminare un utente con p2p_sd specificato
    come parametro dalla lista degli utenti con cui si sta chattando.
*/
void delUserFromChattingWithList(struct usersChattingWith** users_chatting_with, int p2p_sd) {
    
    struct usersChattingWith* elem = NULL;
    // se la lista è vuota non faccio nulla
    if(*users_chatting_with == NULL) {
        return;
    }

    // controllo se ho trovato il p2p_sd dell'utente
    if((*users_chatting_with)->p2p_sd == p2p_sd) {
        elem = *users_chatting_with;
        *users_chatting_with = (*users_chatting_with)->next;
        // libero la memoria allocata per l'indirizzo
        // if(&elem->addr != NULL)
            // free(&elem->addr);
        // libero la memoria allocata per l'utente
        free(elem->dst_username);
        free(elem);

        // chiamo la funzione in modo ricorsivo
        delUserFromChattingWithList(users_chatting_with, p2p_sd);
    } else {
        delUserFromChattingWithList(&(*users_chatting_with)->next, p2p_sd);
    }
    return;
}

/*
    Permette di inviare il messaggio specificato come 
    parametro a tutti gli utenti presenti in lista. 
*/
int sendMessageToAll(struct usersChattingWith** users_chatting_with, char* message) {
    
    int ret;
    struct usersChattingWith* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return -1;
    }

    // per ogni elemento della lista invio il messaggio
    elem = *users_chatting_with;
    while(elem != NULL) {
        ret = send_TCP(&elem->p2p_sd, message);
        if(ret < 0) { return -1; }
        elem = elem->next;
    }

    return 0;
}

/*
    Permette di creare una connessione con un utente e di
    aggiungerlo alla lista degli utenti con cui si sta chattando.
    L'inserimento in lista avviene in testa.
*/
void addNewConnToChattingWithList(struct usersChattingWith** users_chatting_with, char* username, in_port_t port, int* sd) {
    
    struct usersChattingWith* new_user;
    int len;
    int ret;

    // creo il nuovo utente
    new_user = malloc(sizeof(struct usersChattingWith));

    // inizializzo i dati del nuovo utente
    len = strlen(username);
    new_user->dst_username = malloc(len + 1);
    strncpy(new_user->dst_username, username, len);
    new_user->dst_username[len] = '\0';

    new_user->port = port;

    // mi connetto all'utente
    ret = connect_to(&new_user->p2p_sd, &new_user->addr, port);
    *sd = new_user->p2p_sd;

    // aggiungo l'utente in testa
    new_user->next = *users_chatting_with;
    *users_chatting_with = new_user;

    return;
}

/*
    Permette di disconnettersi da tutti gli utenti e 
    presenti nella lista degli utenti online.
*/
void delAllConnFromChattingWithList(struct usersChattingWith** users_chatting_with, fd_set* master) {

    struct usersChattingWith* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    // scorro la lista e mi disconnetto da ogni utente
    elem = *users_chatting_with;
    while(elem != NULL) {
        disconnect_to(&elem->p2p_sd);
        FD_CLR(elem->p2p_sd, master);
        elem = elem->next;
    }
    return;
}

/*
    Permette di disconnettersi dall'utente con 
    p2p_sd specificato come parametro.
*/
void delConnFromChattingWithList(struct usersChattingWith** users_chatting_with, int* sd, fd_set* master) {

    struct usersChattingWith* elem = NULL;
    
    // se la lista è vuota non faccio nulla
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    // scorro la lista e quando trovo l'utente mi disconnetto
    elem = *users_chatting_with;
    while(elem != NULL) {
        if(elem->p2p_sd == *sd) {
            disconnect_to(&elem->p2p_sd);
            FD_CLR(elem->p2p_sd, master);
            return;
        }
        elem = elem->next;
    }
    return;
}