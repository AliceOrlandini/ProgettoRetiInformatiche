#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "./../include/device_requests.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"

/*
    Permette ad un utente di effettuare il login.
*/
void in(int* sd, char* dev_username, char* dev_password) {
    
    char fileLine[64];
    char* username;
    char* password;
    char* port;
    int username_len;
    int password_len;
    int ret;
    int position = 0;
    int lastLen; 
    FILE *fp;
    time_t t;
    char timestamp_login[TIMESTAMP_SIZE]; 

    memset(&fileLine, '\0', sizeof(fileLine));
    
    fp = fopen("./server/files/db_users.txt", "r+"); // apro il file sia in lettura che in scrittura
    if(fp == NULL) { printf("Error0 in\n"); return; }

    // verifico che il device sia effettivamente registrato
    while (fgets(fileLine, sizeof(fileLine), fp) != NULL) {

        lastLen = strlen(fileLine);
        position += lastLen;
    
        username = strtok(fileLine, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");

        username_len = (strlen(username) > strlen(dev_username))? strlen(username):strlen(dev_username);
        password_len = (strlen(password) > strlen(dev_password))? strlen(password):strlen(dev_password);
        
        // controllo se ho trovato l'username e la password è corretta
        if(!strncmp(dev_username, username, username_len) && !strncmp(dev_password, password, password_len)) {
        
            // calcolo il timestamp del login
            t = time(NULL);
            strftime(timestamp_login, sizeof(timestamp_login), "%H:%M:%S", localtime(&t));

            // registro che l'utente si è connesso aggiornando 
            // il timestamp_login e il timestamp_logout 
            position -= lastLen;
            fseek(fp, position, SEEK_SET);
            fprintf(fp, "%s %s %s %s NULL", username, password, port, timestamp_login);

            fclose(fp);

            // comunico all'utente che il login è stato effettuato
            ret = send_TCP(sd, "ok");
            if(ret < 0) { printf("Error2 in\n"); return; }

            printf("Un nuovo utente ha effettuato il login!\n");
            return;
        }
    }

    fclose(fp);

    // comunico all'utente che l'username o password non sono validi
    ret = send_TCP(sd, "no");
    if(ret < 0) { printf("Error1 in\n"); } 

    return;
}

/*
    Permette a un utente di registrarsi un username e una password.
*/
void signup(int* sd, char* dev_username, char* dev_password, char* dev_port) {
    
    int ret; 
    time_t t;
    char timestamp_login[TIMESTAMP_SIZE];
    FILE *fp;

    // apro il file db_users in append
    fp = fopen("./server/files/db_users.txt", "a");
    if(fp == NULL) { printf("Error0 signup\n"); return; }

    // calcolo il timestamp del login
    t = time(NULL);
    strftime(timestamp_login, sizeof(timestamp_login), "%H:%M:%S", localtime(&t));

    // inserisco il nuovo record che sarà: 
    // username password port timestamp_login timestamp_logout
    ret = fprintf(fp, "%s %s %s %s NULL\n", dev_username, dev_password, dev_port, timestamp_login);
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

/*

*/
void out(char* username) {

}