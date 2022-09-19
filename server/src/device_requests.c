#include <stdlib.h>
#include <stdio.h>

#include "./../include/device_requests.h"
#include "./../../network/include/network.h"

/*
    Permette 
*/
void in() {

}

/*
    Permette ad un utente di registrarsi un una mail e una password
*/
void signup(int* sd, char* username, char* password) {
    
    int ret; 
    FILE *fp;

    // apro il file db_users in append
    fp = fopen("./server/files/db_users.txt", "a");
    if(fp == NULL) { printf("Error0 signup\n"); return; }

    // inserisco il nuovo record
    ret = fprintf(fp, "%s %s\n", username, password);
    if(ret < 0) { printf("Error1 signup\n"); return; }

    // comunico al client che la registrazione è avvenuta con successo
    ret = send_TCP(sd, "ok");
    if(ret < 0) { printf("Error2 signup\n"); return; }

    // chiudo il file
    fclose(fp);
}

/*
    Permette 
*/
void hanging() {

}

/*
    Permette 
*/
void show() {

}

/*
    Permette 
*/
void share() {

}

/*
    Permette 
*/
void chat() {

}