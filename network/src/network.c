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
    Funzione per inizializzare il listener.
*/
int init_listener(int* sd, struct sockaddr_in* addr, in_port_t port) {
    
    int ret; 

    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0 socket"); return -1; }

    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr->sin_addr); 

    ret = bind(*sd, (struct sockaddr*)addr, sizeof(*addr));
    if(ret < 0) { perror("Error1 bind"); return -1; }
    
    ret = listen(*sd, BACKLOG);
    if(ret < 0) { perror("Error2 listen"); return -1; }

    printf("Listener inizializzato con successo!\n");
    return 0;
}

/*
    Funzione per la connessione al server.
*/
int connect_to(int* sd, struct sockaddr_in* server_addr, in_port_t srv_port) {
    
    int ret; 
    int addrlen = sizeof(struct sockaddr_in);
    
    *sd = socket(AF_INET, SOCK_STREAM, 0);
    if(*sd < 0) { perror("Error0 connect_to"); return -1; }

    memset(server_addr, 0, sizeof(*server_addr));
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(srv_port);
    inet_pton(AF_INET, "127.0.0.1", &server_addr->sin_addr); 

    ret = connect(*sd, (struct sockaddr*)server_addr, (socklen_t)addrlen);
    if(ret < 0) { perror("Error1 connect_to"); return -2; }
    printf("Stabilita la connessione!\n");
    return 0;
}

/*
    Funzione per l'invio di messaggi TCP.
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
    Funzione per la ricezione di messaggi TCP.
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
    Funzione per l'invio di messaggi UDP.
*/
int send_UDP(int* sd, in_port_t port, char* message) {
/*  
    int ret;
    int len;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);

    // creo il socket
    *sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*sd < 0) { perror("Error0 send_UDP"); return -1; }

    // creo l'indirizzo
    memset(&addr, 0, addrlen);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    // invio i dati
    len = sendto(*sd, message, strlen(message), 0, (struct sockaddr*)&addr, addrlen);
    if(len < 0) { perror("Error1 send_UDP"); return -1; }
    
    // chiudo il socket
    close(*sd);
    return 0;
*/
return 0;
}

int receive_UDP(int* sd, in_port_t port, struct sockaddr_in* my_addr, char* message) {
/*
    int ret;
    int len;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);

    // creo il socket
    *sd = socket(AF_INET, SOCK_DGRAM, 0);
    if(*sd < 0) { perror("Error0 send_UDP"); return -1; }

    // creo l'indirizzo
    memset(my_addr, 0, addrlen);
    my_addr->sin_family = AF_INET;
    my_addr->sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &my_addr->sin_addr);
    ret = bind(*sd, (struct sockaddr*)my_addr, addrlen);

    // ricevo i dati
    len = recvfrom(*sd, message, sizeof(message), 0, (struct sockaddr*)&addr, &addrlen);
*/
return 0;
}

int send_file(int* sd, FILE* fp) {
    
    char data[16380];
    int ret;
    int len;
    uint16_t lmsg;

    while(1) {
        len = fread(data, 1, 16380, fp);
        if(len <= 0) { break; }

        lmsg = htons(len);
    
        // invio la dimensione del frammento che invierò
        ret = send(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
        if(ret < 0) { perror("Error0 send_file len"); return -1; }
    
        // invio questo frammento di file
        ret = send(*sd, (void*)&data, len, 0);
        if(ret < 0) { perror("Error1 send_file data"); return -2; }
        
        // pulisco il buffer dei dati da inviare
        memset(&data, '\0', sizeof(data));
    }

    // invio 0 ad indicare che il file è finito
    lmsg = htons(0);
    ret = send(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
    if(ret < 0) { perror("Error2 send_file len"); return -1; }

    return 0;
}

int receive_file(int* sd, FILE* fp) {

    char data[16380];
    int ret;
    int len;
    uint16_t lmsg;

    while(1) {

        // ricevo la quantità di dati
        ret = recv(*sd, (void*)&lmsg, sizeof(uint16_t), 0);
        if(ret < 0) { perror("Error1 receive_file len"); return -1; }
        if(ret == 0) { printf("QUI2\n"); return -2; }

        // riconverto la dimensione in formato host
        len = ntohs(lmsg);

        // controllo se ho finito di ricevere i dati
        if(len == 0) { break; }

        // inizializzo il buffer per ricevere i dati
        memset(&data, '\0', len);

        // ricevo i dati
        ret = recv(*sd, (void*)data, len, 0);
        if(ret < 0) { perror("Error2 receive_file data"); printf("QUI3\n"); return -1; }
        if(ret == 0) { printf("QUI4\n"); return -2; }

        // scrivo i dati all'interno del file
        fwrite((void*)&data, len, 1, fp);
    }

    return 0;
}

/*
    Funzione per chiudere il socket.
*/
int disconnect_to(int* sd) {
    
    int ret; 

    // chiudo il socket
    ret = close(*sd);
    if(ret < 0) { perror("Error0 disconnect_to"); return -1; } 

    return 0;
}