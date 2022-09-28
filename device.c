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

/*
    Struttura dati che conterrà i dati dell'utente.
*/
struct User user;

/*
    Struttura con cui verrà realizzato il vettore 
    degli utenti online.
*/
struct UsersOnline {
    char* username;
    int port;
};

int newInput(struct User* user, int* sd, int* listener, fd_set* master, int* p2p_sd, struct sockaddr_in* dst_addr, int* fdmax, char* buffer) {

    int ret;
    int len;
    int num_online;
    int j;
    char* message;

    if(user->user_state == LOGGED) {
        
        // eseguo l'azione prevista dal comando
        ret = executeDeviceCommand((char*)buffer, user, sd, NULL);
        if(ret == -2) { 
            printf("Comando non valido.\n"); 
        } else if(ret == -3) {  // se il comando era out allora tolgo il 
                                // socket dal set dei monitorati
            disconnect_to(listener);
            FD_CLR(*sd, master);
            FD_CLR(*listener, master);
            
            return -1;
        } else if(ret == -4) {  // in questo caso il destinatario non è online
                                // quindi i messaggi verranno salvati sul server
            user->user_state = CHATTING_OFFLINE;
            
            // pulisco il buffer
            memset(buffer, '\0', BUFFER_SIZE);
            
            printf("L'utente è offline, i messaggi verranno salvati sul server.\n> ");
            fflush(stdout);
            return 1;
        } else if(ret > 0) {    // in questo caso ret continene 
                                // la porta del destinatario
            // creo una nuova connessione con il destinatario
            ret = connect_to(p2p_sd, dst_addr, ret);
            FD_SET(*p2p_sd, master);
            if(*p2p_sd > *fdmax) { *fdmax = *p2p_sd; } 
            
            // cambio lo stato dell'utente
            user->user_state = CHATTING_ONLINE;
            
            // pulisco il buffer
            memset(buffer, '\0', BUFFER_SIZE);
            
            // stampa di estetica
            printf("> ");
            fflush(stdout);
            return 1;
        }
        
        // pulisco il buffer
        memset(buffer, '\0', BUFFER_SIZE);

        // stampo il menu dei comandi disponibili
        printCommands(*user);
    } else if(user->user_state == CHATTING_ONLINE || user->user_state == CHATTING_OFFLINE) {
        
        // per prima cosa controllo se l'utente ha digitato un comando particolare:
        // 1. controllo se l'utente ha richiesto una chat di gruppo
        if(!strncmp(buffer, "\\u", 2) && user->user_state == CHATTING_ONLINE) {
            
            // invio al server la richiesta di ricevere gli utenti online
            ret = send_TCP(sd, buffer);
            if(ret < 0) { printf("Impossibile ricevere la lista degli online"); return 1; }

            // pulisco il buffer
            memset(buffer, '\0', BUFFER_SIZE);
            
            // ricevo il numero di utenti online
            ret = receive_TCP(sd, buffer);
            if(ret < 0) { printf("Impossibile ricevere la lista degli online"); return 1; }
            
            num_online = atoi(buffer);
            
            // stampo la lista degli online
            printf("\nGli utenti online sono:\n");

            for(j = 0; j < num_online; j++) {
                
                // pulisco il buffer
                memset(buffer, '\0', BUFFER_SIZE);
                
                // ricevo l'username dell'utente online
                ret = receive_TCP(sd, buffer);
                if(ret < 0) { printf("Impossibile ricevere utente"); return 1; }
            
                // stampo l'username (togliendo l'username del device stesso)
                len = (strlen(buffer) > strlen(user->my_username))? strlen(buffer):strlen(user->my_username);
                if(!strncmp(buffer, user->my_username, len)) { return 1; }
                else printf("%s\n", buffer);
            }

            printf("\nPer aggiungere un utente alla chat di gruppo digitare: \\a username + INVIO\n> ");
            fflush(stdout);
            return 1;
        } 

        // 2. controllo se l'utente ha richiesto di aggiungere un membro al gruppo
        if(!strncmp(buffer, "\\a", 2) && user->user_state == CHATTING_ONLINE) {
            char* username;
            username = strtok(buffer, " ");
            username = strtok(NULL, " ");
            printf("%s\n", username);

            return 1;
        }
        
        // 3. controllo se l'utente ha richiesto di terminare la chat
        if(!strncmp(buffer, "\\q", 2)) {

            if(user->user_state == CHATTING_ONLINE) {
                // eseguo la disconnessione dal destinatario
                disconnect_to(p2p_sd);
                FD_CLR(*p2p_sd, master);
            } else {
                // comunico al server che il client ha smesso di inviare messaggi
                ret = send_TCP(sd, "\\q\0");
                if(ret < 0) { return 2; }
            }
            
            // libero la memoria allocata per l'username del destinatario
            free(user->dst_username);
            user->user_state = LOGGED;
            printf("Chat terminata con successo!\n");
            return 1;
        }
        
        // se ho passato i controlli allora l'utente ha scritto
        // un messaggio da inviare al destinatario o al server
        if(user->user_state == CHATTING_ONLINE) {
            
            // aggiungo l'username al messaggio da inviare al device
            len = strlen(buffer) + strlen(user->my_username) + 3;
            message = malloc(len);
            snprintf(message, len, "%s: %s", user->my_username, buffer);
            
            // invio il messaggio
            ret = send_TCP(p2p_sd, message);
            if(ret < 0) { return 1; }
            printf("*");
            fflush(stdout);
        } else {
            
            // aggiungo le informazioni al messaggio da inviare al server
            len = strlen(buffer) + strlen(user->my_username) + strlen(user->dst_username) + 4;
            message = malloc(len);
            snprintf(message, len, "%s %s %s", user->my_username, user->dst_username, buffer);
            
            ret = send_TCP(sd, message);
            if(ret < 0) { return 1; }
        }
        
        // stampa di estetica
        printf("* %s\n> ", buffer);
        fflush(stdout);
        
        // pulisco il buffer
        memset(buffer, '\0', BUFFER_SIZE);
        
        // distruggo il messaggio 
        free(message);
    }  
    return 0; 
}

/* 
    Gestione dei descrittori pronti 
    tramite l'io multiplexing 
*/
void ioMultiplexing(int listener, int* sd, char* buffer) {
    
    int p2p_sd;
    struct sockaddr_in src_addr;
    struct sockaddr_in dst_addr;
    int addrlen = sizeof(struct sockaddr_in);
    int ret;
    int len;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;
    // char* message;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(listener, &master);
    FD_SET(*sd, &master);
    fdmax = *sd;

    for(;;) {
        read_fds = master;
        ret = select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        if(ret < 0) { perror("Error0 select"); }
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == listener) {
                    
                    // accetto la nuova richiesta di connessione da
                    // un device per stabilire una comunicazione p2p
                    p2p_sd = accept(listener, (struct sockaddr*)&src_addr, (socklen_t*)&addrlen);
                    if(p2p_sd < 0) { perror("Error0 accept"); }
                    else {
                        printf("Stabilita una connessione con un device!\n");
                        FD_SET(p2p_sd, &master);
                        if(p2p_sd > fdmax) { fdmax = p2p_sd; } 
                    }

                    // cambio lo stato dell'utente
                    user.user_state = CHATTING_ONLINE;
                    
                    // stampa di estetica
                    printf("> ");
                    fflush(stdout);
                } else if(i == STANDARD_INPUT) {
                    
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)buffer, BUFFER_SIZE);
                    len = strlen(buffer);
                    buffer[len - 1] = '\0';

                    // eseguo la funzione che gestirà l'input
                    ret = newInput(&user, sd, &listener, &master, &p2p_sd, &dst_addr, &fdmax, buffer);
                    if(ret == -1) { return; }
                    else if(ret == 1) { continue; }
                    else if(ret == 2) { break; }
                    
                } else if(i == p2p_sd) {
                    
                    // pulisco il buffer
                    memset(buffer, '\0', BUFFER_SIZE);

                    // ricevo il messaggio dal device
                    ret = receive_TCP(&i, (char*)buffer);
                    
                    // in questo caso ho ricevuto 0 byte quindi chiudo il socket di
                    // comunicazione con il device e riporto lo stato utente a LOGGED
                    if(ret == -2) { 
                        disconnect_to(&i);
                        FD_CLR(i, &master); 
                        printf("Il device ha chiuso la comunicazione.\n"); 
                        user.user_state = LOGGED;
                        
                        // pulisco il buffer
                        memset(buffer, '\0', BUFFER_SIZE);
                        continue;
                    }
                    
                    // stampo a video il messaggio ricevuto
                    printf("%s\n> ", buffer);
                    fflush(stdout);
                    
                    // pulisco il buffer
                    memset(buffer, '\0', BUFFER_SIZE);
                }
            }
        }
    }
}

int main(int argc, char *argv[]) {

    int sd;
    struct sockaddr_in server_addr;
    int listener;
    struct sockaddr_in my_addr;
    int ret;
    int len;
    char buffer[BUFFER_SIZE];

    // se l'utente non ha specificato la porta termino 
    if(argv[1] == NULL) {
        printf("Error: porta device non specificata.\n");
        return 0;
    } 

    // imposto lo stato e la porta dell'utente
    user.user_state = DISCONNECTED;
    len = strlen(argv[1]);
    user.my_port = malloc(len + 1);
    strncpy(user.my_port, argv[1], len);
    user.my_port[len] = '\0'; 

    // imposto il listener per le comunicazioni con gli altri device
    ret = init_listener(&listener, &my_addr, atoi(user.my_port));
    if(ret < 0) { exit(0); }

    // pulisco il buffer
    memset(&buffer, '\0', BUFFER_SIZE);

    // stampo i comandi disponibili
    printCommands(user);

    // finchè l'utente non si connette non faccio partire l'io multiplexing 
    while(user.user_state == DISCONNECTED) {
        // prelievo il comando dallo standard input e lo salvo nel buffer
        read(STANDARD_INPUT, (void*)&buffer, BUFFER_SIZE);
        len = strlen(buffer);
        buffer[len-1] = '\0'; // per togliere il \n
        
        // eseguo l'azione prevista dal comando
        ret = executeDeviceCommand((char*)&buffer, &user, &sd, &server_addr);
        if(ret == -1) { printf("Comando non valido.\n"); }
        
        // pulisco il buffer
        memset(&buffer, '\0', BUFFER_SIZE);

        // stampo i comandi disponibili
        printCommands(user);
    }

    // ricevo le notifiche mentre ero offline
    receiveNotifications(&sd, (char*)&buffer);

    // pulisco il buffer
    memset(&buffer, '\0', BUFFER_SIZE);

    // faccio partire l'io multiplexing
    ioMultiplexing(listener, &sd, (char*)&buffer);
    
    return 0;
}