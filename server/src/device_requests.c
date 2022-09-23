#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "./../include/device_requests.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"

/*
    Permette ad un utente di effettuare il login.
*/
void in(int* sd, char* dev_username, char* dev_password) {
    
    char file_line[64];
    char* username;
    char* password;
    char* port;
    int username_len;
    int password_len;
    int ret;
    int position = 0;
    int last_len; 
    FILE *fp;
    time_t t;
    char timestamp_login[TIMESTAMP_SIZE]; 

    memset(&file_line, '\0', sizeof(file_line));
    
    fp = fopen("./server/files/db_users.txt", "r+"); // apro il file sia in lettura che in scrittura
    if(fp == NULL) { printf("Error0 in\n"); return; }

    // verifico che il device sia effettivamente registrato
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {

        last_len = strlen(file_line);
        position += last_len;
    
        username = strtok(file_line, " ");
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
            position -= last_len;
            fseek(fp, position, SEEK_SET);
            fprintf(fp, "%s %s %s %s NULL    ", username, password, port, timestamp_login);

            fclose(fp);

            // comunico all'utente che il login è stato effettuato
            ret = send_TCP(sd, "ok");
            if(ret < 0) { printf("Error2 in\n"); return; }

            printf("%s ha effettuato il login!\n", dev_username);
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
    ret = fprintf(fp, "%s %s %s %s NULL    \n", dev_username, dev_password, dev_port, timestamp_login);
    if(ret < 0) { printf("Error1 signup\n"); return; }

    // comunico al client che la registrazione è avvenuta con successo
    ret = send_TCP(sd, "ok");
    if(ret < 0) { printf("Error2 signup\n"); return; }

    // chiudo il file
    fclose(fp);

    printf("%s si è registrato!\n", dev_username);
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
    Permette ad un utente di iniziare una chat con dst_username.
    Se questo non dovesse essere online i messaggi verranno bufferizzati.
*/
void chat(int* sd, char* dst_username) {
    
    int ret;
    FILE* fp;
    char file_line[64];
    int username_len;
    char* username;
    char* password;
    char* port;
    char* timestamp_login;
    char* timestamp_logout;
    bool status = false; // true: online; false: offline;

    // controllo se il destinatario è online o meno
    fp = fopen("./server/files/db_users.txt", "r"); 
    if(fp == NULL) { printf("Error0 chat\n"); return; }

    // verifico che il device sia effettivamente registrato
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        
        // ricavo i dati del destinatario
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");
        timestamp_logout = strtok(NULL, " ");

        username_len = (strlen(username) > strlen(dst_username))? strlen(username):strlen(dst_username);
        
        // controllo se il timestamp del logout è NULL
        if(!strncmp(timestamp_logout, "NULL", 4) && !strncmp(dst_username, username, username_len)) {
            status = true;
            break;
        }
    }

    fclose(fp);

    if(status) {
        printf("ONLINE\n");
        // nel caso in cui il destinatario sia online 
        // restituisco al client la porta del destinatario 
        ret = send_TCP(sd, port);
    } else {
        printf("OFFLINE\n");
        // nel caso in cui il destinatario sia offline 
        // restituisco al client l'avviso che il messaggio
        // è stato bufferizzato
        ret = send_TCP(sd, "offline");
    }
}

/*
    Permette di registrare l'ultimo accesso dell'utente.
*/
void out(char* dev_username) {

    char file_line[64];
    char* username;
    char* password;
    char* port;
    char* timestamp_login;
    int username_len;
    int last_len;
    int position = 0;
    time_t t;
    char timestamp_logout[TIMESTAMP_SIZE];
    FILE *fp;

    // apro il file db_users in append
    fp = fopen("./server/files/db_users.txt", "r+"); // apro il file sia in lettura che in scrittura
    if(fp == NULL) { printf("Error0 signup\n"); return; }

    // scorro tutto il file finchè non trovo l'username
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {

        last_len = strlen(file_line);
        position += last_len;

        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");

        username_len = (strlen(username) > strlen(dev_username))? strlen(username):strlen(dev_username);

        // controllo se ho trovato l'username
        if(!strncmp(dev_username, username, username_len)) {

            // calcolo il timestamp del logout
            t = time(NULL);
            strftime(timestamp_logout, sizeof(timestamp_logout), "%H:%M:%S", localtime(&t));

            // registro che l'utente si è connesso aggiornando il timestamp_logout
            position -= last_len;
            fseek(fp, position, SEEK_SET);

            fprintf(fp, "%s %s %s %s %s", username, password, port, timestamp_login, timestamp_logout);

            fclose(fp);

            printf("%s ha effettuato il logout!\n", dev_username);
            return;
        }
    }

    return;
}

/*
    Gestione della richiesta del device, a seconda
    del comando ricevuto si invoca la funzione corrispondente.
*/
int serveDeviceRequest(int* sd, char* request, char** username) {

    char* command = NULL;
    char* dev_username;
    char* dev_password;
    char* dev_port;
    int len;

    printf("Richiesta ricevuta da un client %s\n", request);
    
    // prendo il comando inserito 
    command = strtok(request, " ");

    if(!strncmp(command, "in", 2)) {
        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");

        // salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);

        in(sd, dev_username, dev_password);
        
        return 1;
    } else if(!strncmp(command, "signup", 6)) {

        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");
        dev_port = strtok(NULL, " ");

        // salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);
        
        signup(sd, dev_username, dev_password, dev_port);

        return 1;
    } else if(!strncmp(command, "hanging", 7)) {
        hanging();
    } else if(!strncmp(command, "show", 4)) {
        show();
    } else if(!strncmp(command, "chat", 4)) {
        dev_username = strtok(NULL, " ");
        
        chat(sd, dev_username);
    } else if(!strncmp(command, "share", 5)) {
        share();
    } else { return -1; }
    return 0;
}