#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../include/device_commands.h"
#include "./../include/device_consts.h"
#include "./../include/network.h"

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
int signup(char* command, char* username, char* password, int* sd, struct sockaddr_in* server_addr) {

    int len;
    int ret;
    char* message; 

    // stabilisco la connessione con il server
    ret = connect_to_server(sd, server_addr, SERVER_PORT);
    if(ret < 0) { return -1; }
    
    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(username) + strlen(password) + 2; // il +2 serve per gli spazi
    message = malloc(len);
    snprintf(message, len, "%s %s %s", command, username, password);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire la registrazione\n"); free(message); return -1; }
    
    // libero la memoria utilizzata per il messaggio
    free(message);
    
    printf("Registrazione avvenuta con successo!\n");
    return 0;
}

/*
    Permette al device di richiedere al 
    server la connessione al servizio.
*/
int in(char* command, int srv_port, char* username, char* password, int* sd, struct sockaddr_in* server_addr) {
    
    int len;
    int ret;
    char* message; 
    
    // stabilisco la connessione con il server
    ret = connect_to_server(sd, server_addr, srv_port);
    if(ret < 0) { return -1; }

    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(username) + strlen(password) + 2; // il +2 serve per gli spazi
    message = malloc(len);
    snprintf(message, len, "%s %s %s", command, username, password);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire il login\n"); free(message); return -2; }
    
    // libero la memoria utilizzata per il messaggio
    free(message);

    printf("Login avvenuto con successo!\n");
    return 0;
}

/*
    Permette all'utente di ricevere la lista degli utenti 
    chi gli hanno inviato messaggi mentre era offline.
*/
void hanging(char* command, int* sd) {
    
    int ret;

    // invio al server il messaggio
    ret = send_TCP(sd, command);
    if(ret < 0) { printf("Impossibile eseguire hanging\n"); return; }

    printf("Hanging avvenuta con successo!\n");
    return;
}

/*
    Consente all'utente di ricevere i messaggi 
    pendenti dall'utente username.
*/
void show(char* username) {
    
}

/*
    Avvia una chat con l'utente username.
*/
void chat(char* username) {
    
}

/*
    Invia il file file_name al device su cui è connesso
    l'utente o gli utenti con cui si sta chattando. 
*/
void share(char* file_name) {
    
}

/*
    Permette al device di disconnettersi dal server.
*/
void out(int* sd) {
    
    int ret;
    // invio la richiesta di disconnessione
    ret = disconnect_to_server(sd);
    if(ret == -1) { printf("Impossibile disconnettersi\n"); }
    printf("Disconnessione avvenuta con successo!\n");
}

/*
    A seconda del comando digitato dall'utente
    si esegue la funzione corrispondente.
*/
int executeDeviceCommand(char* buffer, struct User* user, int* sd, struct sockaddr_in* server_addr) {

    int ret;
    char* command = NULL;
    char* file_name = NULL;
    
    // prendo il comando inserito 
    command = strtok(buffer, " ");

    // controllo che per i comandi in e signin l'utente sia disconnesso
    // per gli altri comandi l'utente deve essere connesso.
    // Poi a seconda del comando inserito prendo i parametri e chiamo la funzione
    if(user->user_state == DISCONNECT) {
        if(!strncmp(command, "in", 2)) {
            user->srv_port = atoi(strtok(NULL, " "));
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");
             
            ret = in(command, user->srv_port, user->my_username, user->my_password, sd, server_addr);
            if(ret == 0) { user->user_state = LOGGED; }

        } else if(!strncmp(command, "signup", 6)) { 
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");

            ret = signup(command, user->my_username, user->my_password, sd, server_addr);
            if(ret == 0) { user->user_state = LOGGED; }
        
        } else { // in caso di comando non valido restituisco -1
            return -1;
        }
    } else if(user->user_state == LOGGED) {
        if(!strncmp(command, "hanging", 7)) {
            hanging(command, sd);
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
            out(sd);
            
            user->user_state = DISCONNECT;
        } else { // in caso di comando non valido restituisco -2
            return -2;
        }
    }

    return 0;
}