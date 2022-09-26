#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "./../include/pending_messages.h"

void addElemToPMList(struct pendingMessage** pending_message_list, char* username, char* timestamp, char* message) {
    
    int len;
    
    // creo il nuovo messaggio
    struct pendingMessage* new_message;
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

    // aggiungo il nuovo messaggio in testa alla lista
    new_message->next = *pending_message_list;
    *pending_message_list = new_message;

    return;
}

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
    printf("Lista dei messaggi pendenti eliminata con successo!\n");
}

void printPMList(struct pendingMessage** pending_message_list) {
    
    if(pending_message_list == NULL) {
        return;
    }

    struct pendingMessage* elem = *pending_message_list;
    while(elem != NULL) {
        printf("USERNAME: %s\n", elem->username_src);
        printf("TIMESTAMP: %s\n", elem->timestamp);
        printf("MESSAGE: %s\n", elem->message);
        elem = elem->next;
    }
    printf("FINE\n");
    return;
}

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

    fclose(fp);

    // stampa di controllo
    // printPMList(pending_message_list);
}