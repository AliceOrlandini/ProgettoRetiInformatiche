#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h> 

#include "./../include/network.h"
#include "./../include/device_consts.h"

/*
    Funzione per la connessione al server
*/
int connect_to_server(int* sd, struct sockaddr_in* server_addr, in_port_t srv_port) {
    
    int ret; 
    int addrlen = sizeof(struct sockaddr_in);
    
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0 connect_to_server"); return -1; }
    
    memset(server_addr, 0, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(srv_port);
    inet_pton(AF_INET, LOCALHOST, &server_addr->sin_addr); 

    ret = connect(*sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
    if(ret < 0) { perror("Error1 connect_to_server"); return -2; }
    printf("Stabilita la connessione!\n");
    return 0;
}

/*
    Funzione per l'invio di messaggi TCP
*/
int send_TCP(int* sd, char* message) {

    int len; 
    int ret;
    uint16_t lmsg;
    
    len = strlen(message); 
    lmsg = htons(len);
    
    // invio la dimensione dei dati che invierò
    ret = send(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0) { perror("Error0 send_TCP len"); return -1; }
    
    // invio i dati
    ret = send(*sd, (void*)message, len, 0);
    if(ret < 0) { perror("Error1 send_TCP data"); return -2; }

    return 0;
}

/*
    Funzione per la ricezione di messaggi TCP
*/
void receive_TCP(int* sd, char* message) {

}

int disconnect_to_server(int* sd) {
    
    int ret; 

    // chiudo il socket
    ret = close(*sd);
    if(ret < 0) { perror("Error0 disconnect_to_server"); return -1; } 

    return 0;
}