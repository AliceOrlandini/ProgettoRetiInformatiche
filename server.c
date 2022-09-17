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

/*#include <sys/select.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>*/

/*
    Gestione della richiesta del device, a seconda
    del comando ricevuto si invoca la funzione corrispondente
*/
int serveDeviceRequest(char* request) {

    char* command = NULL;
    
    // prendo il comando inserito 
    command = strtok(request, " ");

    if(!strncmp(command, "in", 2)) {
        printf("IN\n");
    } else if(!strncmp(command, "signup", 6)) {
        printf("SIGNUP\n");
    } else if(!strncmp(command, "hanging", 7)) {
        printf("HANGING\n");
    } else if(!strncmp(command, "show", 4)) {
        printf("SHOW\n");
    } else if(!strncmp(command, "chat", 4)) {
        printf("CHAT\n");
    } else if(!strncmp(command, "share", 5)) {
        printf("SHARE\n");
    } else if(!strncmp(command, "out", 3)) {
        printf("OUT\n");
    } else {
        return -1;
    }
    return 0;
}

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
    int len;
    uint16_t lmsg;
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
                    executeServerCommand((char*)&buffer);
                } else { 
                    pid = fork();
                    if(pid < 0) { perror("Error1 fork"); }
                    else if(pid == 0) { // sono nel processo figlio
                        close(listener);

                        while(1) {
                            // inizializzo il buffer per ricevere la lunghezza
                            memset(&buffer, '\0', sizeof(buffer));
                            
                            // ricevo la quantità di dati
                            ret = recv(i, (void*)&lmsg, sizeof(uint16_t), 0);
                            if(ret < 0) { perror("Error2 receive_TCP len"); exit(0); }

                            // controllo se ho ricevuto 0 byte perchè  
                            // nel caso il client si è disconnesso
                            if(ret == 0) { break; }
                            // riconverto la dimensione in formato host
                            len = ntohs(lmsg);
                            
                            // inizializzo il buffer per ricevere il dato
                            memset(&buffer, '\0', len);

                            // ricevo i dati
                            ret = recv(i, (void*)&buffer, len, 0);
                            if(ret < 0) { perror("Error3 receive_TCP data"); exit(0); }
                            if(ret == 0) { break; }
                            printf("Richiesta ricevuta da un client %s\n", buffer);

                            // a seconda del tipo di richiesta eseguo la funzione corrispondente
                            ret = serveDeviceRequest(buffer);
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
    printCommands();
    
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) { perror("Error0 socket"); exit(0); }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(srv_port);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    ret = bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0) { perror("Error1 bind"); exit(0); }
    
    ret = listen(listener, BACKLOG);
    if(ret < 0) { perror("Error2 listen"); exit(0); }

    ioMultiplexing(listener);
    
    return  0;
}