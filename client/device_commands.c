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

int executeDeviceCommand(char* buffer) {

    char* command = NULL;
    int srv_port = 4242;
    char* username = NULL;
    char* password = NULL;
    char* file_name = NULL;
    
    // prendo il comando inserito dall'utente
    command = strtok(buffer, " ");

    // a seconda del comando inserito prendo i parametri 
    // e chiamo la funzione corrispondente
    if(!strncmp(command, "in", 2)) {
        srv_port = atoi(strtok(NULL, " "));
        username = strtok(NULL, " ");
        password = strtok(NULL, " ");
        in(srv_port, username, password);
    } else if(!strncmp(command, "signup", 6)) {
        username = strtok(NULL, " ");
        password = strtok(NULL, " ");
        signup(username, password);
    } else if(!strncmp(command, "hanging", 7)) {
        hanging();
    } else if(!strncmp(command, "show", 4)) {
        username = strtok(NULL, " ");
        show(username);
    } else if(!strncmp(command, "chat", 4)) {
        username = strtok(NULL, " ");
        chat(username);
    } else if(!strncmp(command, "share", 5)) {
        file_name = strtok(NULL, " ");
        share(file_name);
    } else if(!strncmp(command, "out", 3)) {
        out();
    } else { // in caso di comando non valido restituisco -1
        return -1;
    }

    return 0;
}