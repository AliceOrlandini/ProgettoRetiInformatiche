#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

#define DEVICE_COMMAND_SIZE 20
#define STANDARD_INPUT 0
#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 4242
#define BUFFER_SIZE 1024

void ioMultiplexing(int sd, char* buffer, struct sockaddr_in* server_addr) {
    
    int addrlen = sizeof(struct sockaddr_in);
    int ret;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;
    int len;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(sd, &master);
    fdmax = sd;

    for(;;) {
        read_fds = master;
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == sd) {
                    ret = connect(sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
                    if(ret < 0) {
                        perror("Error1");
                        exit(0);
                    }
                    printf("Stabilita una connessione\n");

                    len = 1;
                    buffer[0] = '1';
                    ret = send(sd, (void*)buffer, len, 0);
                    if(ret == -1) {
                        perror("Error2");
                        break;
                    }
                    printf("Richiesta inviata al server\n");
                    


                } else if(i == STANDARD_INPUT){
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)&buffer, DEVICE_COMMAND_SIZE);
                    // eseguo l'azione prevista dal comando
                    // executeServerCommand((char*)&buffer);
                }
            }
        }
    }
}
int main(int argc, char *argv[]) {

    // in_port_t device_port = atoi(argv[1]);
    char buffer[BUFFER_SIZE];
    int sd;
    struct sockaddr_in server_addr;

    sd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    ioMultiplexing(sd, (char*)&buffer, &server_addr);
    
    return  0;
}