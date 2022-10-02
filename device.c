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
    Lista che conterrà i dati di tutti gli utenti online 
    (solo gli utenti presenti in rubrica).
*/
struct onlineUser* online_user_list = NULL;

void clearMaster(int* server_sd, int* listener, fd_set* master) {
    
    // tolgo dal set dei monitorati il socket per la comunicazione
    // con il server e il socket di ascolto
    FD_CLR(*server_sd, master);
    FD_CLR(*listener, master);
    return;
}

void chattingOffline(struct User* user, char* buffer) {
    
    // cambio lo stato dell'utente
    user->user_state = CHATTING_OFFLINE;      
    
    // pulisco il buffer
    memset(buffer, '\0', BUFFER_SIZE);
    
    // stampa di informazione
    printf("L'utente è offline, i messaggi verranno salvati sul server.\n> ");
    fflush(stdout);
    return;
}

void chattingOnline(int ret, struct User* user, fd_set* master, int* p2p_sd, struct sockaddr_in* dst_addr, int* fdmax, char* buffer) {
    
    // creo una nuova connessione con il destinatario e lo aggiungo alla lista
    addNewConnToChattingWithList(&user->users_chatting_with, user->dst_username, ret, p2p_sd);
    FD_SET(*p2p_sd, master);
    if(*p2p_sd > *fdmax) { *fdmax = *p2p_sd; } 

    // cambio lo stato dell'utente
    user->user_state = CHATTING_ONLINE;
    
    // pulisco il buffer
    memset(buffer, '\0', BUFFER_SIZE);
    
    // stampa di estetica
    printf("> ");
    fflush(stdout);
    return; 
}

int getOnlineUsers(int* sd, struct User* user, char* buffer) {
    
    int ret;
    int len;
    int num_online;
    int j;
    char* username;
    char* port = NULL;

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
        
        // ricevo l'username e la porta dell'utente online
        ret = receive_TCP(sd, buffer);
        if(ret < 0) { printf("Impossibile ricevere utente"); continue; }
    
        username = strtok(buffer, " ");
        port = strtok(NULL, " ");

        // stampo l'username (togliendo l'username del device 
        // stesso e quelli non presenti in rubrica)
        len = (strlen(username) > strlen(user->my_username))? strlen(username):strlen(user->my_username);
        if(!strncmp(username, user->my_username, len)) { continue; }
        if(!checkContacts(user->my_username, username)) { continue; } 
        
        // se ho superato i controlli salvo in lista i dati dell'utente in modo 
        // da non dover ricontattare il server per aggiungerlo alla chat di gruppo
        addElemToOnlineUserList(&online_user_list, username, port);
        
        // stampo a video l'username dell'utente
        printf("%s\n", username); 
    }

    // pulisco il buffer
    memset(buffer, '\0', BUFFER_SIZE);
    return 0; 
}

void newGroupMember(struct User* user, char* buffer, fd_set* master, int* fdmax) {
    
    int sd;
    char* username;
    char* port = NULL;
    
    username = strtok(buffer, " ");
    username = strtok(NULL, " ");
    
    // recupero la porta di username 
    port = getPortFromOnlineUserList(&online_user_list, username);
    if(port == NULL) {
        printf("Questo utente non può essere inserito nel gruppo.\n> ");
        fflush(stdout);
        return;
    }

    // printf("La porta è %d\n> ", atoi(port));
    // fflush(stdout);

    // creo una nuova connessione con il destinatario
    addNewConnToChattingWithList(&user->users_chatting_with, username, atoi(port), &sd);
    FD_SET(sd, master);
    if(sd > *fdmax) { *fdmax = sd; } 

    // pulisco il buffer
    memset(buffer, '\0', BUFFER_SIZE);

    // stampa di informazione
    printf("Utente inserito nel gruppo con successo!\n> ");
    fflush(stdout);
    return; 
}

int quitChat(struct User* user, int* p2p_sd, fd_set* master, int* sd) {
    
    int ret; 
    
    // agisco diversamente a seconda dello stato dell'utente
    if(user->user_state == CHATTING_ONLINE) {
        
        // eseguo la disconnessione da tutti i membri del gruppo
        delAllConnFromChattingWithList(&user->users_chatting_with, master);
        
        // distruggo la lista degli utenti online
        delOnlineUserList(&online_user_list);

        // distruggo la lista degli utenti con cui si sta chattando
        delChattingWithList(&user->users_chatting_with);
    } else {
        
        // comunico al server che il client ha smesso di inviare messaggi
        ret = send_TCP(sd, "\\q\0");
        if(ret < 0) { return 2; }
    }

    // libero la memoria allocata per l'username del destinatario
    free(user->dst_username);

    // cambio lo stato dell'utente
    user->user_state = LOGGED;
    printf("Chat terminata con successo!\n");
    return 1;
}

int sendMessage(struct User* user, char* buffer, int* sd) {

    int ret;
    int len;
    char* message;

    // a seconda dello stato dell'utente agisco diversamente
    if(user->user_state == CHATTING_ONLINE) {
        
        // aggiungo l'username al messaggio da inviare al device
        len = strlen(buffer) + strlen(user->my_username) + 3;
        message = malloc(len);
        snprintf(message, len, "%s: %s", user->my_username, buffer);
        
        // invio il messaggio a tutti i membri del gruppo
        ret = sendMessageToAll(&user->users_chatting_with, message);
        if(ret < 0) { return 1; }
        printf("*");
        fflush(stdout);
    } else {
        
        // aggiungo le informazioni al messaggio da inviare al server
        len = strlen(buffer) + strlen(user->my_username) + strlen(user->dst_username) + 4;
        message = malloc(len);
        snprintf(message, len, "%s %s %s", user->my_username, user->dst_username, buffer);
        
        // invio al server il messaggio
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
    return 0;
}

void sendMessageToOthers(struct usersChattingWith** users_chatting_with, int p2p_sd, char* buffer) {

    int ret;

    if(users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    // invio il messaggio ai componenti del gruppo (escluso il mittente)
    struct usersChattingWith* elem = *users_chatting_with;
    while(elem != NULL) {
        if(elem->p2p_sd == p2p_sd) {
            elem = elem->next;
            continue;
        }
        ret = send_TCP(&elem->p2p_sd, buffer);
        if(ret < 0) { printf("Impossibile inviare messaggio agli altri.\n"); }
        elem = elem->next;
    }
    return;
}

void shareFile(struct usersChattingWith** users_chatting_with, char* file_name) {

    FILE* fp;
    int ret;
    int len;
    struct usersChattingWith* elem = NULL;
    char* message;
    
    fp = fopen(file_name, "rb"); // apro il file in lettura in binario
    if(fp == NULL) { printf("Errore nell'apertura del file.\n"); return; }

    if(users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    // unisco le stringhe per inviare un solo messaggio
    len = strlen(file_name) + 7; 
    message = malloc(len);
    snprintf(message, len, "share %s", file_name);

    // invio ai componenti del gruppo 
    // l'informazione che sto per inviare un file e poi invio il file
    elem = *users_chatting_with;
    while(elem != NULL) {
        
        ret = send_TCP(&elem->p2p_sd, message);
        if(ret < 0) { printf("Impossibile inviare il file a questo utente.\n"); elem = elem->next; continue; }

        ret = send_file(&elem->p2p_sd, fp);
        if(ret < 0) { printf("Impossibile inviare il file a questo utente.\n"); elem = elem->next; continue; }

        elem = elem->next;
    }

    // libero la memoria allocata per il messaggio
    free(message);

    // chiudo il file
    fclose(fp);
  
    // stampa di informazione
    printf("File inviato con successo!\n> ");
    fflush(stdout);
    return;
}

void receiveFile(int* sd, char* file_name) {

    FILE* fp;
    int ret;
    int len;
    char* file_path;

    // creo il path di dove andrà salvato il file
    len = strlen(file_name) + 16;
    file_path = malloc(len);
    strncpy(file_path, "./client/media/", 15);
    strncat(file_path, file_name, len);
    file_path[len - 1] = '\0';

    fp = fopen(file_path, "wb"); // apro il file in scrittura binaria
    if(fp == NULL) { printf("Errore nell'apertura del file.\n"); return; }
   
    // ricevo il file
    ret = receive_file(sd, fp);
    if(ret < 0) { printf("Impossibile ricevere il file.\n"); fclose(fp); return; }
  
    // chiudo il file
    fclose(fp);

    // stampa di informazione
    printf("File ricevuto con successo!\n> ");
    fflush(stdout);
    return;
}

void sendFileToOthers() {

}

int newInput(struct User* user, int* sd, int* listener, fd_set* master, int* p2p_sd, struct sockaddr_in* dst_addr, int* fdmax, char* buffer) {

    int ret;
    char* file_name;

    if(user->user_state == LOGGED) {
        
        // eseguo l'azione prevista dal comando
        ret = executeDeviceCommand((char*)buffer, user, sd, NULL);
        if(ret == -2) {         // in questo caso il comando non è valido
            printf("Comando non valido.\n"); 
        } else if(ret == -3) {  // se il comando era out allora tolgo i 
                                // socket dal set dei monitorati
            clearMaster(sd, listener, master);
            return -1;
        } else if(ret == -4) {  // in questo caso il destinatario non è online
                                // quindi i messaggi verranno salvati sul server
            chattingOffline(user, buffer);
            return 1;
        } else if(ret > 0) {    // in questo caso ret continene 
                                // la porta del destinatario
            chattingOnline(ret, user, master, p2p_sd, dst_addr, fdmax, buffer);
            return 1;
        }
        // pulisco il buffer
        memset(buffer, '\0', BUFFER_SIZE);

    } else if(user->user_state == CHATTING_ONLINE || user->user_state == CHATTING_OFFLINE) {
        
        // per prima cosa controllo se l'utente ha digitato un comando particolare:
        // 1. controllo se l'utente ha richiesto una chat di gruppo (solo nella chat online)
        if(!strncmp(buffer, "\\u", 2) && user->user_state == CHATTING_ONLINE) {

            // distruggo la lista degli utenti online perchè verrà ricreata
            delOnlineUserList(&online_user_list);
            
            // ricevo dal server la lista degli utenti online
            ret = getOnlineUsers(sd, user, buffer);
            if(ret == 1) { return 1; }
            
            // stampo delle informazioni
            printf("\nPer aggiungere un utente alla chat di gruppo digitare: \\a username + INVIO\n> ");
            fflush(stdout);
            return 1;
        } 

        // 2. controllo se l'utente ha richiesto di aggiungere un membro al gruppo
        if(!strncmp(buffer, "\\a", 2) && user->user_state == CHATTING_ONLINE) {
            
            // aggiungo l'utente al gruppo
            newGroupMember(user, buffer, master, fdmax);
            return 1;
        }
        
        // 3. controllo se l'utente ha richiesto di terminare la chat
        if(!strncmp(buffer, "\\q", 2)) {
            
            // termino la chat
            ret = quitChat(user, p2p_sd, master, sd);
            return ret;
        }

        // 4. controllo se l'utente vuole inviare un file
        if(!strncmp(buffer, "share", 5)) {

            file_name = strtok(buffer, " ");
            file_name = strtok(NULL, " ");

            // controllo che l'utente abbia inserito il nome del file
            if(file_name == NULL) { printf("Nome del file non specificato.\n"); return 1; }
            
            // invio il file a tutti i componenti del gruppo
            shareFile(&user->users_chatting_with, file_name);
            return 1;
        }
        
        // 5. se ho passato tutti i controlli allora invio il
        //    messaggio o al server o a tutti i membri del gruppo
        ret = sendMessage(user, buffer, sd);
        return ret;
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

                    // aggiungo questo utente alla lista di quelli con cui si sta chattando
                    addElemToChattingWithList(&user.users_chatting_with, "undefined", 0, p2p_sd);

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
                    
                } else {
                    
                    // pulisco il buffer
                    memset(buffer, '\0', BUFFER_SIZE);

                    // ricevo il messaggio dal device
                    ret = receive_TCP(&i, (char*)buffer);
                    
                    // in questo caso ho ricevuto 0 byte quindi chiudo il socket di
                    // comunicazione con il device e riporto lo stato utente a LOGGED
                    // nel caso in cui non stia chattando con nessun altro
                    if(ret == -2) { 
                        
                        // elimino la connessione con questo device
                        delConnFromChattingWithList(&user.users_chatting_with, &i, &master);
                        printf("Il device ha chiuso la comunicazione.\n"); 

                        // elimino questo elemento dalla lista di quelli con cui si sta chattando
                        delUserFromChattingWithList(&user.users_chatting_with, i);

                        // se la lista degli utenti con cui si sta chattando è vuota
                        // riporto lo stato dell'utente a LOGGED
                        if(user.users_chatting_with == NULL) { user.user_state = LOGGED; }
                        else { printf("> "); fflush(stdout); }
                        
                        // pulisco il buffer
                        memset(buffer, '\0', BUFFER_SIZE);

                        continue;
                    }

                    // controllo se l'utente mi vuole inviare un file
                    if(!strncmp(buffer, "share", 5)) {
                        
                        // recupero il nome del file
                        char* file_name = strtok(buffer, " ");
                        file_name = strtok(NULL, " ");
                        
                        // ricevo il file
                        receiveFile(&i, file_name);
                        
                        // invio il file agli altri componenti del gruppo
                        // sendFileToOthers();
                        continue;
                    }
                    
                    // se non avevo ricevuto 0 byte allora invio il
                    // messaggio agli altri componenti del gruppo
                    sendMessageToOthers(&user.users_chatting_with, i, buffer);
                    
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

    // disconnetto il socket di ascolto del device
    ret = disconnect_to(&listener);
    if(ret == 0) { printf("Disconnessione listener avvenuta con successo!\n"); }

    return 0;
}