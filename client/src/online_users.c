#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "./../include/online_users.h"

/**
 * Inserisce un nuovo utente alla lista degli utenti
 * online. L'inserimento in lista avviene in testa.
 * 
 * @param online_user_list puntatore alla lista degli utenti online.
 * @param username puntatore al buffer contenente l'username dell'utente.
 * @param port puntatore al buffer contenente la porta dell'utente.
 */
void addElemToOnlineUserList(struct onlineUser** online_user_list, char* username, char* port) {
    
    struct onlineUser* new_user;
    int len;
    
    // creo il nuovo utente
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

    // inserisco il nuovo utente in testa
    new_user->next = *online_user_list;
    *online_user_list = new_user;

    return;
}

/**
 * Elimina tutta la lista degli utenti online.
 * 
 * @param online_user_list puntatore alla lista degli utenti online.
 */
void delOnlineUserList(struct onlineUser** online_user_list) {
    
    struct onlineUser *del_user;
    while(*online_user_list != NULL) {
        del_user = (*online_user_list)->next;

        // libero la memoria allocata per i dati dell'utente
        free((*online_user_list)->username);
        free((*online_user_list)->port);
        free(*online_user_list);

        // elimino l'utente
        *online_user_list = del_user;
    }
    printf("Lista degli utenti online con successo!\n");
}

/**
 * Permette di stampare il contenuto della lista degli 
 * utenti online. Utilizzata solo in fase di debug.
 * 
 * @param online_user_list puntatore alla lista degli utenti online.
 */
void printOnlineUserList(struct onlineUser** online_user_list) {
    
    struct onlineUser* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*online_user_list == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    elem = *online_user_list;
    while(elem != NULL) {
        printf("\nusername: %s\n", elem->username);
        printf("port: %s\n", elem->port);
        elem = elem->next;
    }
    printf("Fine lista.\n");
    return;
}

/**
 * Permette di eliminare dalla lista degli utenti online 
 * l'utente il cui username è specificato come parametro.
 * 
 * @param online_user_list puntatore alla lista degli utenti online.
 * @param username puntatore al buffer contenente l'username dell'utente.
 */
void delUserFromOnlineUserList(struct onlineUser** online_user_list, char* username) {
    
    int len;
    struct onlineUser* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*online_user_list == NULL) {
        return;
    }

    // controllo se ho trovato l'username
    len = (strlen((*online_user_list)->username) > strlen(username))? strlen((*online_user_list)->username):strlen(username);
    if(!strncmp((*online_user_list)->username, username, len)) {
        elem = *online_user_list;
        *online_user_list = (*online_user_list)->next;
        
        // elimino la memoria allocata per i dati dell'utente
        free(elem->username);
        free(elem->port);
        free(elem);

        // chiamo la funzione in modo ricorsivo
        delUserFromOnlineUserList(online_user_list, username);
    } else {
        delUserFromOnlineUserList(&(*online_user_list)->next, username);
    }
}

/**
 * Scorre la lista degli utenti online e restituisce
 * la porta dell'utente specificato come parametro.
 * 
 * @param online_user_list puntatore alla lista degli utenti online.
 * @param username puntatore al buffer contenente l'username dell'utente.
 * @return la porta dell'utente.
 */
char* getPortFromOnlineUserList(struct onlineUser** online_user_list, char* username) {

    int len;
    struct onlineUser* elem = NULL;

    // se la lista è vuota restituisco NULL
    if(*online_user_list == NULL) {
        return NULL;
    }

    elem = *online_user_list;
    while(elem != NULL) {

        // se ho trovato l'username restituisco la porta
        len = (strlen(username) > strlen(elem->username))? strlen(username):strlen(elem->username);
        if(!strncmp(username, elem->username, len)) {
            return elem->port;
        }
        elem = elem->next;
    }

    // se non ho trovato l'utente restituisco NULL
    return NULL;
}