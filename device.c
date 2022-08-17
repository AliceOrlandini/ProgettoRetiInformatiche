#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include "./client/device_consts.h"
#include "./client/device_commands.h"

void ioMultiplexing(int sd, struct sockaddr_in* server_addr) {
    
    // int addrlen = sizeof(struct sockaddr_in);
    // pid_t pid;
    int ret;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;
    // int len;
    char message_buffer[BUFFER_SIZE];
    char commands_buffer[BUFFER_SIZE];

    memset(&message_buffer, '\0', BUFFER_SIZE);
    memset(&commands_buffer, '\0', BUFFER_SIZE);

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(sd, &master);
    fdmax = sd;

    for(;;) {
        read_fds = master;
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        if(ret < 0) { /*errore*/ }
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                /*if(i == sd) {
                     ret = connect(sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
                    if(ret < 0) { perror("Error1"); exit(0); }
                    printf("Stabilita la connessione!\n");  

                } else*/ if(i == STANDARD_INPUT){
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)&commands_buffer, DEVICE_COMMAND_SIZE);
                    
                    // eseguo l'azione prevista dal comando
                    ret = executeDeviceCommand((char*)&commands_buffer);
                    if(ret < 0) { printf("Comando non valido\n"); }
                    // pulisco il buffer dei comandi
                    memset(&commands_buffer, '\0', BUFFER_SIZE);
                } /*else {
                    pid = fork();
                    if(pid < 0) { }
                    if(pid == 0) { // sono nel processo figlio
                        close(sd);
                        
                        // Invio un messaggio di prova
                        len = 1;
                        buffer[0] = '1';
                        ret = send(sd, (void*)buffer, len, 0);
                        if(ret < 0) { perror("Error2"); break; }
                        printf("Richiesta inviata al server\n");
                        
                        // close(new_sd);
                        exit(0);
                    } else { // sono nel processo padre
                        // chiudo il socket connesso
                        close(i);
                        // tolgo il descrittore del socket connesso dal set dei monitorati
                        FD_CLR(i, &master);
                    }  
                } */
            }
        }
    }
}
int main(int argc, char *argv[]) {

    // in_port_t device_port = atoi(argv[1]);
    int sd;
    struct sockaddr_in server_addr;
    int addrlen = sizeof(struct sockaddr_in);
    int ret;

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) { perror("Error0"); exit(0); }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    ret = connect(sd, (struct sockaddr*)&server_addr, (socklen_t)addrlen);
    if(ret < 0) { perror("Error1"); exit(0); }
    printf("Stabilita la connessione!\n"); 

    ioMultiplexing(sd, &server_addr);
    
    return  0;
}