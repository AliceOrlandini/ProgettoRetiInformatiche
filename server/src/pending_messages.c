#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./../include/pending_messages.h"

/**
 * Permette di aggiungere un nuovo messaggio alla lista
 * dei messaggi pendenti specificata come parametro della
 * funzione. L'inserimento avviene in coda per ordinare
 * i messaggi dal più vecchio al più nuovo.
 * 
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 * @param username puntatore al buffer contenente l'username da inserire in lista.
 * @param timestamp puntatore al buffer contenente il timestamp da inserire in lista.
 * @param message puntatore al buffer contenente il messaggio da inserire in lista.
 */
void addElemToPMList(struct pendingMessage** pending_message_list, char* username, char* timestamp, char* message) {
    
    struct pendingMessage* new_message;
    struct pendingMessage* q;
    struct pendingMessage* p;
    int len;

    // scorro la lista fino a raggiungere il fondo
    for(q = *pending_message_list; q != NULL; q = q->next) {
        p = q;
    }
    
    // creo il nuovo messaggio
    new_message = malloc(sizeof(struct pendingMessage));

    // inizializzo i dati del nuovo messaggio
    len = strlen(username);
    new_message->username_src = malloc(len + 1);
    strncpy(new_message->username_src, username, len);
    new_message->username_src[len] = '\0';

    len = strlen(timestamp);
    new_message->timestamp = malloc(len + 1);
    strncpy(new_message->timestamp, timestamp, len);
    new_message->timestamp[len] = '\0';

    len = strlen(message);
    new_message->message = malloc(len + 1);
    strncpy(new_message->message, message, len);
    new_message->message[len] = '\0';

    new_message->next = NULL;

    // inserisco il nuovo messaggio in coda
    if(q == *pending_message_list)
        *pending_message_list = new_message;
    else 
        p->next = new_message;

    return;
}

/**
 * Permette di eliminare tutta la lista dei messaggi pendenti.
 * 
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 */
void delPMList(struct pendingMessage** pending_message_list) {
    
    struct pendingMessage *del_user;
    while(*pending_message_list != NULL) {
        del_user = (*pending_message_list)->next;
        free((*pending_message_list)->username_src);
        free((*pending_message_list)->timestamp);
        free((*pending_message_list)->message);
        free(*pending_message_list);
        *pending_message_list = del_user;
    }
    *pending_message_list = NULL;
    return; 
}

/**
 * Funzione che stampa username, timestamp e messaggio
 * di ogni elemento presente nella lista dei messaggi 
 * pendenti. Questa funzione viene utilizzata solo in 
 * fase di debugging del codice. 
 * 
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 */
void printPMList(struct pendingMessage** pending_message_list) {
    
    struct pendingMessage* elem;

    // se la lista è vuota non faccio nulla
    if(*pending_message_list == NULL) {
        return;
    }

    elem = *pending_message_list;
    while(elem != NULL) {
        printf("username: %s\n", elem->username_src);
        printf("timestamp: %s\n", elem->timestamp);
        printf("messagge: %s\n", elem->message);
        elem = elem->next;
    }
    printf("end print\n");
    return;
}

/**
 * Funzione per creare la lista dei messaggi pendenti
 * relativa ad un utente specificato per parametro.
 * I messaggi vengono prelevati dal file db_messages.txt.
 * 
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 * @param dev_username puntatore al buffer che contiene l'username dell'utente.
 */
void createPMList(struct pendingMessage** pending_message_list, char* dev_username) {
    
    FILE* fp;
    char file_line[1024];
    char* timestamp;
    char* username_src;
    char* username_dst;
    char* message;
    int username_dst_len;
    
    fp = fopen("./server/files/db_messages.txt", "r");
    if(fp == NULL) { printf("Error0 hanging\n"); return; }

    // prelevo una riga del file
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        timestamp = strtok(file_line, " ");
        username_src = strtok(NULL, " ");
        username_dst = strtok(NULL, " ");
        message = strtok(NULL, "\n");

        username_dst_len = (strlen(username_dst) > strlen(dev_username))? strlen(username_dst):strlen(dev_username);

        // controllo se ho trovato l'username dell'utente
        if(!strncmp(dev_username, username_dst, username_dst_len)) {
            
            // aggiungo il messaggio alla lista dei messaggi pendenti
            addElemToPMList(pending_message_list, username_src, timestamp, message);
        }
    } 

    // chiudo il file
    fclose(fp);

    // stampa di debugging
    // printPMList(pending_message_list);
}

/**
 * Permette di eliminare tutti i messaggi che hanno come 
 * mittente l'utente specificato come parametro. 
 * 
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 * @param username puntatore al buffer che contiene l'username dell'utente.
 */
void delMessagesFromPMList(struct pendingMessage** pending_message_list, char* username) {
    
    int len;

    // se la lista è vuota non faccio nulla
    if(*pending_message_list == NULL) {
        return;
    }

    // se ho trovato l'username del mittente allora elimino quel messaggio
    len = (strlen((*pending_message_list)->username_src) > strlen(username))? strlen((*pending_message_list)->username_src):strlen(username);
    if(!strncmp((*pending_message_list)->username_src, username, len)) {
        struct pendingMessage* elem = *pending_message_list;
        *pending_message_list = (*pending_message_list)->next;
        
        // libero la memoria allocata per il messaggio
        free(elem->username_src);
        free(elem->timestamp);
        free(elem->message);

        // chiamo la funzione in modo ricorsivo
        delMessagesFromPMList(pending_message_list, username);
    } else {
        delMessagesFromPMList(&(*pending_message_list)->next, username);
    }
    return; 
}