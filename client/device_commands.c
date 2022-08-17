#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device_commands.h"
#include "device_consts.h"

void printCommands() {
    
}

void signup(char* username, char* password) {
    
}

void in(int srv_name, char* username, char* password) {
    
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

int executeDeviceCommand(char* buffer, struct User* user) {

    char* command = NULL;
    char* file_name = NULL;
    
    // prendo il comando inserito 
    command = strtok(buffer, " ");

    // controllo anche che per i comandi in e signin l'utente sia disconnesso
    // per gli altri comandi l'utente deve essere connesso.
    // Poi a seconda del comando inserito prendo i parametri 
    // e chiamo la funzione corrispondente
    if(user->user_state == DISCONNECT) {
        if(!strncmp(command, "in", 2)) {
            user->srv_port = atoi(strtok(NULL, " "));
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");
            in(user->srv_port, user->my_username, user->my_password);
            user->user_state = LOGGED;
        } else if(!strncmp(command, "signup", 6)) {
            user->my_username = strtok(NULL, " ");
            user->my_password = strtok(NULL, " ");
            signup(user->my_username, user->my_password);
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