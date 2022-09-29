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

/* 
    Gestione dei descrittori pronti 
    tramite l'io multiplexing.
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
                        struct pendingMessage* pending_message_list = NULL;

                        while(1) {

                            // inizializzo il buffer 
                            memset(&buffer, '\0', BUFFER_SIZE);
                            
                            // ricevo la richiesta dal client
                            ret = receive_TCP(&i, buffer);
                            if(ret < 0) { out(username); delPMList(&pending_message_list); break; }

                            // se ricevo -2 significa che il client si è disconnesso
                            if(ret == -2) { out(username); delPMList(&pending_message_list); break; }

                            // se ricevo -1 dalla receive significa che
                            // la comunicazione ha avuto qualche problema
                            else if(ret == -1) { continue; }

                            // servo la richiesta del client
                            ret = serveDeviceRequest(&i, buffer, &username, &pending_message_list);
                            if(ret < 0) { printf("Richiesta non valida\n"); continue; }
                            else if(ret == 1) { 
                                username[strlen(username)] = '\0'; 
                                
                                // creo la lista dei messaggi pendenti
                                createPMList(&pending_message_list, username);

                                // invio le notifiche dei messaggi letti mentre era offline
                                sendNotifications(&i, username);

                            } else if(ret == 2) { // in questo caso salvo i messaggi che arriano dal client
                                while(1) {
                                    
                                    // inizializzo il buffer 
                                    memset(&buffer, '\0', BUFFER_SIZE);

                                    // ricevo il messaggio dal client
                                    ret = receive_TCP(&i, buffer);
                                    if(ret < 0) { out(username); delPMList(&pending_message_list); return; }
                                    
                                    // controllo se l'utente vuole terminare la chat
                                    if(!strncmp(buffer, "\\q", 2)) { 
                                        printf("Chat terminata con successo!\n"); break;
                                    } else { 
                                        // altrimenti salvo il messaggio
                                        saveMessage(buffer);
                                        printf("Messaggio salvato con successo!\n");  
                                    }
                                }
                            }
                        }
                        
                        // chiudo il socket di comunicazione
                        close(new_sd);
                        // lo tolgo dal set di monitorati
                        FD_CLR(new_sd, &master);
                        // libero la memoria allocata per l'username
                        free(username);
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
    ret = init_listener(&listener, &server_addr, srv_port);
    if(ret < 0) { exit(0); }

    // mostro i comandi disponibili 
    printCommands();

    // faccio partire l'io multiplexing
    ioMultiplexing(listener);
    
    return 0;
}