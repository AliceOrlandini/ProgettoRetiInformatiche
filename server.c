#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include "./server/include/server_consts.h"
#include "./server/include/server_commands.h"
#include "./server/include/device_requests.h"
#include "./network/include/network.h"

/*#include <sys/select.h>
#include <time.h>
#include <stdbool.h>*/

/*
    Gestione della richiesta del device, a seconda
    del comando ricevuto si invoca la funzione corrispondente
*/
int serveDeviceRequest(int* sd, char* request, char** username) {

    char* command = NULL;
    char* dev_username;
    char* dev_password;
    char* dev_port;
    int len;
    
    // prendo il comando inserito 
    command = strtok(request, " ");

    if(!strncmp(command, "in", 2)) {
        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");

        in(sd, dev_username, dev_password);

        // mi salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);
        username[len] = '\0';
        *username = dev_username; 
        
        return 1;
    } else if(!strncmp(command, "signup", 6)) {

        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");
        dev_port = strtok(NULL, " ");
        
        signup(sd, dev_username, dev_password, dev_port);

        // mi salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);
        username[len] = '\0';
        *username = dev_username; 

        return 1;
    } else if(!strncmp(command, "hanging", 7)) {
        hanging();
    } else if(!strncmp(command, "show", 4)) {
        show();
    } else if(!strncmp(command, "chat", 4)) {
        chat();
    } else if(!strncmp(command, "share", 5)) {
        share();
    } else { return -1; }
    return 0;
}

/*struct onlineUser {
    char* username;
    int sd; // socket di comunicazione associato a questo user
    struct onlineUser* next_user; // per creare la lista degli user online
};
void addUserToList(struct onlineUser** online_users_list, struct onlineUser** new_user, int new_sd, char* username) {
    
    // creo il nuovo utente
    int len = strlen(username);
    // struct onlineUser* new_user;
    *new_user = malloc(sizeof(struct onlineUser));

    // inizializzo i dati del nuovo utente
    (*new_user)->username = malloc(len + 1);
    strncpy((*new_user)->username, username, len);
    (*new_user)->username[len] = '\0';
    (*new_user)->sd = new_sd;

    // aggiungo il nuovo utente in testa alla lista
    (*new_user)->next_user = *online_users_list;
    *online_users_list = *new_user;

    return;
}

void delUserFromList(struct onlineUser** online_users_list) {

    // free(new_user->username);
    // free(new_user);
}

void delList(struct onlineUser** online_users_list) {
    
    struct onlineUser *del_user;
    while(*online_users_list != NULL) {
        del_user = (*online_users_list)->next_user;
        free((*online_users_list)->username);
        free(*online_users_list);
        *online_users_list = del_user;
    }
}

void printList(struct onlineUser** online_users_list) {
    
    if(online_users_list == NULL) {
        return;
    }

    struct onlineUser* user = *online_users_list;
    while(user != NULL) {
        printf("USER: %s\n", user->username);
        user = user->next_user;
    }
    printf("FINE\n");
    return;
}*/

/* 
    Gestione dei descrittori pronti 
    tramite l'io multiplexing 
*/
void ioMultiplexing(int listener) {
    
    int new_sd;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(struct sockaddr_in);
    
    pid_t pid;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;
    
    int ret;
    char buffer[BUFFER_SIZE];

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(listener, &master);
    fdmax = listener;

    for(;;) {
        read_fds = master;
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == listener) {
                    // accetto la nuova richiesta di connessione
                    new_sd = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                    if(new_sd < 0) { perror("Error0 accept"); }
                    else {
                        printf("Stabilita una connessione!\n");
                        FD_SET(new_sd, &master);
                        if(new_sd > fdmax) { fdmax = new_sd; } 
                    }
                } else if(i == STANDARD_INPUT) {
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)&buffer, SERVER_COMMAND_SIZE);
                    
                    // eseguo l'azione prevista dal comando
                    executeServerCommand((char*)&buffer, &listener);
                } else { 

                    pid = fork();
                    if(pid < 0) { perror("Error1 fork"); }
                    else if(pid == 0) { // sono nel processo figlio
                        close(listener);

                        char* username = NULL;

                        while(1) {
                            // inizializzo il buffer per ricevere la lunghezza
                            memset(&buffer, '\0', sizeof(buffer));

                            ret = receive_TCP(&i, buffer);

                            // se ricevo -2 dalla receive significa
                            // che il client si è disconnesso
                            if(ret == -2) { out(username); break; }

                            // se ricevo -1 dalla receive significa che
                            // la comunicazione ha avuto qualche problema
                            else if(ret == -1) { continue; }

                            printf("Richiesta ricevuta da un client %s\n", buffer);

                            // a seconda del tipo di richiesta eseguo la funzione corrispondente
                            ret = serveDeviceRequest(&i, buffer, &username);
                            if(ret < 0) { printf("Richiesta non valida\n"); }
                        }
                        
                        printf("Il client si è disconnesso\n"); 
                            
                        // chiudo il socket di comunicazione
                        close(new_sd);
                        // lo tolgo dal set di monitorati
                        FD_CLR(new_sd, &master);
                        // termino il processo figlio
                        exit(0);
                    } else { // sono nel processo padre
                        // chiudo il socket di comunicazione
                        close(i);
                        // tolgo il descrittore del socket di comunicazione dal set dei monitorati
                        FD_CLR(i, &master);
                    }
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {

    int listener;
    in_port_t srv_port;
    struct sockaddr_in server_addr;
    int ret;

    srv_port = (argv[1] == NULL)? SERVER_PORT:atoi(argv[1]);

    printf("***** SERVER STARTED *****\n");
    
    // inizializzo il server
    ret = init_server(&listener, &server_addr, srv_port);
    if(ret < 0) { exit(0); }

    // mostro i comandi disponibili 
    printCommands();

    // faccio partire l'io multiplexing
    ioMultiplexing(listener);
    
    return  0;
}