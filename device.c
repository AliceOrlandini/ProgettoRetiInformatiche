#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#include "./client/include/device_consts.h"
#include "./client/include/device_commands.h"
#include "./network/include/network.h"

struct User user;

/* 
    Gestione dei descrittori pronti 
    tramite l'io multiplexing 
*/
void ioMultiplexing(int* sd, char* commands_buffer) {
    
    // pid_t pid;
    int ret;
    int len;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;

    char message_buffer[BUFFER_SIZE];

    memset(&message_buffer, '\0', BUFFER_SIZE);

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(*sd, &master);
    fdmax = *sd;

    for(;;) {
        read_fds = master;
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        if(ret < 0) { perror("Error0 select"); }
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == STANDARD_INPUT){
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)commands_buffer, BUFFER_SIZE);
                    len = strlen(commands_buffer);
                    commands_buffer[len-1] = '\0'; // per togliere il \n
                    
                    // eseguo l'azione prevista dal comando
                    ret = executeDeviceCommand((char*)commands_buffer, &user, sd, NULL);
                    if(ret == -2) { printf("Comando non valido, i comandi accettati sono hanging, show, chat, share e out\n"); }
                    
                    // se il comando era out allora tolgo il socket dal set dei monitorati
                    if(ret == 0 && !strncmp(commands_buffer, "out", 3)) {
                        FD_CLR(*sd, &master);
                        
                        // pulisco il buffer dei comandi
                        memset(commands_buffer, '\0', BUFFER_SIZE);
                        
                        // faccio terminare l'io multiplexing e di conseguenza il client
                        return;
                    }
                    // pulisco il buffer dei comandi
                    memset(commands_buffer, '\0', BUFFER_SIZE);
                } 
            }
        }
    }
}

int main(int argc, char *argv[]) {

    in_port_t device_port;
    int sd;
    struct sockaddr_in server_addr;
    int ret;
    char commands_buffer[BUFFER_SIZE];

    // se l'utente non ha specificato la porta termino 
    if(argv[1] == NULL) {
        printf("Error: porta device non specificata\n");
        return 0;
    } else {
        device_port = atoi(argv[1]);
    }

    // imposto i valori dell'utente
    user.user_state = DISCONNECT;
    user.my_port = device_port;

    // stampo i comandi disponibili
    printCommands(user);

    // pulisco il buffer dei comandi
    memset(&commands_buffer, '\0', BUFFER_SIZE);

    // finchè l'utente non si connette non faccio partire l'iomultiplexing 
    // altrimenti ad fdmax verrebbe assegnato un valore non significativo
    while(user.user_state == DISCONNECT) {
        // prelievo il comando dallo standard input e lo salvo nel buffer
        read(STANDARD_INPUT, (void*)&commands_buffer, BUFFER_SIZE);
        
        // eseguo l'azione prevista dal comando
        ret = executeDeviceCommand((char*)&commands_buffer, &user, &sd, &server_addr);
        if(ret == -1) { printf("Comando non valido, i comandi accettati sono in e signup\n"); }
        
        // pulisco il buffer dei comandi
        memset(&commands_buffer, '\0', BUFFER_SIZE);
    }

    // stampo il menu dei comandi disponibili
    printCommands(user);

    ioMultiplexing(&sd, (char*)&commands_buffer);
    
    return 0;
}