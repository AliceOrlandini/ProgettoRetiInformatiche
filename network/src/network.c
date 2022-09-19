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

/*
    Funzione per inizializzare il server
*/
int init_server(int* sd, struct sockaddr_in* server_addr, in_port_t srv_port) {
    
    int ret; 

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0 init_server"); return -1; }

    memset(server_addr, 0, sizeof(server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(srv_port);
    inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr); 

    ret = bind(*sd, (struct sockaddr*)server_addr, sizeof(*server_addr));
    if(ret < 0) { perror("Error1 bind"); return -1; }
    
    ret = listen(*sd, 10);
    if(ret < 0) { perror("Error2 listen"); return -1; }

    printf("Server inizializzato con successo!\n");
    return 0;
}

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
    inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr); 

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
int receive_TCP(int* sd, char* message) {
    
    int ret; 
    int len;
    uint16_t lmsg;

    // ricevo la quantità di dati
    ret = recv(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0) { perror("Error2 receive_TCP len"); return -1; }

    // controllo se ho ricevuto 0 byte perchè  
    // nel caso il socket si è disconnesso
    if(ret == 0) { return -2; }
    
    // riconverto la dimensione in formato host
    len = ntohs(lmsg);

    // inizializzo il buffer per ricevere il dato
    memset(message, '\0', len);

    // ricevo i dati
    ret = recv(*sd, (void*)message, len, 0);
    if(ret < 0) { perror("Error3 receive_TCP data"); return -1; }
    if(ret == 0) { return -2; }
    
    return 0;
}

/*
    Funzione per chiudere il socket
*/
int disconnect_to_server(int* sd) {
    
    int ret; 

    // chiudo il socket
    ret = close(*sd);
    if(ret < 0) { perror("Error0 disconnect_to_server"); return -1; } 

    return 0;
}