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

/*
    funzione per la connessione al server
*/
void connect_to_server(int* sd, struct sockaddr_in* server_addr) {
    
    int ret; 
    int addrlen = sizeof(struct sockaddr_in);
    
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0 connect_to_server"); exit(0); }
    
    memset(server_addr, 0, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, LOCALHOST, &server_addr->sin_addr); 

    ret = connect(*sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
    if(ret < 0) { perror("Error1 connect_to_server"); exit(0); }
    printf("Stabilita la connessione!\n");
}

/*
    funzione per l'invio di messaggi TCP al server
*/
void send_TCP(int* sd, char* messaggio) {

    int len; 
    int ret;
    uint16_t lmsg;
    
    len = strlen(messaggio);
    lmsg = htons(len);
    // invio la dimensione dei dati che invierò
    ret = send(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0) { perror("Error0 send_TCP len"); }
    // invio i dati
    ret = send(*sd, (void*)messaggio, len, 0);
    if(ret < 0) { perror("Error1 send_TCP data"); }
    printf("Richiesta inviata al server!\n");
}

/*
    funzione per la ricezione di messaggi TCP da parte del server
*/
void receive_TCP(int* sd, char* messaggio) {

}