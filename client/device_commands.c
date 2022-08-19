#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device_commands.h"
#include "device_consts.h"
#include "network.h"

/*
    Stampa a video i comandi disponibili a 
    seconda dello stato dell'utente.
*/
void printCommands(struct User user) {
    printf("I comandi disponibili sono:\n");
    if(user.user_state == DISCONNECT) {
        printf("1) in --> per accedere al servizio \n2) signup --> per creare un account\n");
    } else if(user.user_state == LOGGED) {
        printf("1) hanging --> per ricevere i messaggi mentre si era offline\n2) show --> per ricevere i messaggi pendenti dall'utente specificato\n3) chat --> per chattare con un altro utente\n4) share --> per condividere un file\n5) out --> per disconnettersi\n");
    }
}

/* 
    Permette a un utente di creare un account sul server, 
    caratterizzato da username e password.
*/
void signup(char* command, char* username, char* password, int* sd, struct sockaddr_in* server_addr) {

    int len;
    int ret;
    char* message; 

    // stabilisco la connessione con il server
    connect_to_server(sd, server_addr);
    
    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(username) + strlen(password) + 2; // il +2 serve per gli spazi
    message = malloc(len);
    snprintf(message, len, "%s %s %s", command, username, password);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire la registrazione\n"); }
    
    // libero la memoria utilizzata per il messaggio
    free(message);
    
    printf("Registrazione avvenuta con successo!\n");
}

void in(char* command, int srv_port, char* username, char* password, int* sd, struct sockaddr_in* server_addr) {
    
    // stabilisco la connessione con il server
    connect_to_server(sd, server_addr);

    printf("Login avvenuto con successo!\n");
}

void hanging() {
    
}

void show(char* username) {
    
}

void chat(char* username) {
    
}

void share(char* file_name) {
    
}

void out() {
    
}

/*
    A seconda del comando digitato dall'utente
    si esegue la funzione corrispondente.
*/
int executeDeviceCommand(char* buffer, struct User* user, int* sd, struct sockaddr_in* server_addr) {

    char* command = NULL;
    char* file_name = NULL;
    
    // prendo il comando inserito 
    command = strtok(buffer, " ");

    // controllo che per i comandi in e signin l'utente sia disconnesso
    // per gli altri comandi l'utente deve essere connesso.
    // Poi a seconda del comando inserito prendo i parametri 
    // e chiamo la funzione 
    if(user->user_state == DISCONNECT) {
        if(!strncmp(command, "in", 2)) {
            user->srv_port = atoi(strtok(NULL, " "));
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");
             
            in(command, user->srv_port, user->my_username, user->my_password, sd, server_addr);
            user->user_state = LOGGED;
        } else if(!strncmp(command, "signup", 6)) { 
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");

            signup(command, user->my_username, user->my_password, sd, server_addr);
            user->user_state = LOGGED;
        } else { // in caso di comando non valido restituisco -1
            return -1;
        }
    } else if(user->user_state == LOGGED) {
        if(!strncmp(command, "hanging", 7)) {
            hanging();
        } else if(!strncmp(command, "show", 4)) {
            user->dst_username = strtok(NULL, " ");
            show(user->dst_username);
        } else if(!strncmp(command, "chat", 4)) {
            user->dst_username = strtok(NULL, " ");
            chat(user->dst_username);
        } else if(!strncmp(command, "share", 5)) {
            file_name = strtok(NULL, " ");
            share(file_name);
        } else if(!strncmp(command, "out", 3)) {
            out();
            user->user_state = DISCONNECT;
        } else { // in caso di comando non valido restituisco -2
            return -2;
        }
    }

    return 0;
}