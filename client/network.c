#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h> 

#include "network.h"
#include "device_consts.h"

void server_connect(int* sd, struct sockaddr_in* server_addr) {
    int ret; 
    int addrlen = sizeof(struct sockaddr_in);
    
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0"); exit(0); }
    
    memset(server_addr, 0, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &server_addr->sin_addr); 

    ret = connect(*sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
    if(ret < 0) { perror("Error1"); exit(0); }
    printf("Stabilita la connessione!\n");
}