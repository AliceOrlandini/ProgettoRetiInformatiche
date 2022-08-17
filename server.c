#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include "./server/server_consts.h"
#include "./server/server_commands.h"

/*#include <sys/select.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>*/

/* gestione dei descrittori pronti tramite l'io multiplexing */
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
                    new_sd = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                    printf("Stabilita una connessione\n");
                    FD_SET(new_sd, &master);
                    if(new_sd > fdmax) { fdmax = new_sd; }
                } else if(i == STANDARD_INPUT){
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)&buffer, SERVER_COMMAND_SIZE);
                    // eseguo l'azione prevista dal comando
                    executeServerCommand((char*)&buffer);
                } else { // Il socket connesso è pronto
                    pid = fork();
                    if(pid < 0) { /* errore */ }
                    if(pid == 0) { // sono nel processo figlio
                        close(listener);
                        
                        /* memset(&buffer, '\0', sizeof(buffer));
                        len = 1;
                        ret = recv(i, (void*)&buffer, len, 0);
                        if(ret < 0) { }
                        printf("Richiesta ricevuta da un client %s\n", buffer); */
                        
                        // close(new_sd);
                        exit(0);
                    } else { // sono nel processo padre
                        // chiudo il socket connesso
                        close(i);
                        // tolgo il descrittore del socket connesso dal set dei monitorati
                        FD_CLR(i, &master);
                    }  
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {

    // variabili di utilita'
    int listener;
    struct sockaddr_in server_addr;
    int ret;

    printf("***** SERVER STARTED *****\n");
    printCommands();
    
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("Error1");
        exit(0);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    ret = bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        perror("Error2");
        exit(0);
    }
    
    ret = listen(listener, BACKLOG);
    if(ret < 0) {
        perror("Error3");
        exit(0);
    }

    ioMultiplexing(listener);
    
    return  0;
}