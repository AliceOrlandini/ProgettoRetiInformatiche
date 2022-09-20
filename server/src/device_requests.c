#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "./../include/device_requests.h"
#include "./../../network/include/network.h"

/*
    Permette ad un utente di richiedere il servizio
*/
void in(int* sd, char* dev_username, char* dev_password) {
    
    // verifico che il device sia effettivamente registrato
    char fileLine[64];
    char* username;
    char* password;
    int found = 0;
    int ret;
    FILE *fp;
    
    fp = fopen("./server/files/db_users.txt", "r");
    if(fp == NULL) { printf("Error0 in\n"); return; }
    while (fgets(fileLine, sizeof(fileLine), fp) != NULL) {
        
        username = strtok(fileLine, " ");
        password = strtok(NULL, " ");

        // controllo se ho trovato l'username e la password è corretta
        if(!strncmp(dev_username, username, strlen(dev_username)) && !strncmp(dev_password, password, strlen(dev_username))) {
            found = 1;
            break;
        }
    }

    fclose(fp);

    if(found == 0) { 
        // comunico all'utente che il suo username o password non sono validi
        ret = send_TCP(sd, "no");
        if(ret < 0) { printf("Error1 in\n"); return; }
    } else {
        // registro che l'utente si è connesso aggiornando il timestamp_login
        // fp = fopen("./server/files/db_users.txt", "w");



        // fclose(fp);

        // comunico all'utente che il login è stato effettuato
        ret = send_TCP(sd, "ok");
        if(ret < 0) { printf("Error2 in\n"); return; }
    }

}

/*
    Permette a un utente di registrarsi un una mail e una password
*/
void signup(int* sd, char* dev_username, char* dev_password) {
    
    int ret; 
    time_t t;
    char timestamp_login[20];
    FILE *fp;

    // apro il file db_users in append
    fp = fopen("./server/files/db_users.txt", "a");
    if(fp == NULL) { printf("Error0 signup\n"); return; }

    // calcolo il timestamp del login
    t = time(NULL);
    strftime(timestamp_login, sizeof(timestamp_login), "%H:%M:%S", localtime(&t));

    // inserisco il nuovo record che sarà: 
    // username password port timestamp_login timestamp_logout
    ret = fprintf(fp, "%s %s PORT %s NULL\n", dev_username, dev_password, timestamp_login);
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