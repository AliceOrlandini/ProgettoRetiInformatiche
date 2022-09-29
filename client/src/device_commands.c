#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./../include/device_commands.h"
#include "./../include/device_consts.h"
#include "./../../network/include/network.h"

void addElemToOnlineUserList(struct onlineUser** online_user_list, char* username, char* port) {
    
    // aggiungo il nuovo utente in coda alla lista
    struct onlineUser* new_user;
    struct onlineUser* q;
    struct onlineUser* p;
    int len;

    for(q = *online_user_list; q != NULL; q = q->next) {
        p = q;
    }
    // inizializzo il nuovo utente
    new_user = malloc(sizeof(struct onlineUser));

    // inizializzo i dati del nuovo utente
    len = strlen(username);
    new_user->username = malloc(len + 1);
    strncpy(new_user->username, username, len);
    new_user->username[len] = '\0';

    len = strlen(port);
    new_user->port = malloc(len + 1);
    strncpy(new_user->port, port, len);
    new_user->port[len] = '\0';

    new_user->next = NULL;
    if(q == *online_user_list)
        *online_user_list = new_user;
    else 
        p->next = new_user;

    return;
}

void delOnlineUserList(struct onlineUser** online_user_list) {
    
    struct onlineUser *del_user;
    while(*online_user_list != NULL) {
        del_user = (*online_user_list)->next;
        free((*online_user_list)->username);
        free((*online_user_list)->port);
        free(*online_user_list);
        *online_user_list = del_user;
    }
    printf("Lista dei messaggi pendenti eliminata con successo!\n");
}

void printOnlineUserList(struct onlineUser** online_user_list) {
    
    if(*online_user_list == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    struct onlineUser* elem = *online_user_list;
    while(elem != NULL) {
        printf("\nUSERNAME: %s\n", elem->username);
        printf("PORT: %s\n", elem->port);
        elem = elem->next;
    }
    printf("FINE\n");
    return;
}

void delUserFromOnlineUserList(struct onlineUser** online_user_list, char* username) {
    
    int len;

    if(*online_user_list == NULL) {
        return;
    }

    len = (strlen((*online_user_list)->username) > strlen(username))? strlen((*online_user_list)->username):strlen(username);
    if(!strncmp((*online_user_list)->username, username, len)) {
        struct onlineUser* elem = *online_user_list;
        *online_user_list = (*online_user_list)->next;
        
        // elimino la memoria allocata per il messaggio
        free(elem->username);
        free(elem->port);
        // elimino la memoria allocata per la struttura
        free(elem);

        delUserFromOnlineUserList(online_user_list, username);
    } else {
        delUserFromOnlineUserList(&(*online_user_list)->next, username);
    }
}

char* getPortFromOnlineUserList(struct onlineUser** online_user_list, char* username) {

    int len;

    if(*online_user_list == NULL) {
        return NULL;
    }

    struct onlineUser* elem = *online_user_list;
    while(elem != NULL) {
        len = (strlen(username) > strlen(elem->username))? strlen(username):strlen(elem->username);
        if(!strncmp(username, elem->username, len)) {
            return elem->port;
        }
        elem = elem->next;
    }

    return NULL;
}




void addElemToChattingWithList(struct usersChattingWith** users_chatting_with, char* username, int port, int p2p_sd) {
    
    // aggiungo il nuovo utente in coda alla lista
    struct usersChattingWith* new_user;
    struct usersChattingWith* q;
    struct usersChattingWith* p;
    int len;

    for(q = *users_chatting_with; q != NULL; q = q->next) {
        p = q;
    }
    // inizializzo il nuovo utente
    new_user = malloc(sizeof(struct usersChattingWith));

    // inizializzo i dati del nuovo utente
    len = strlen(username);
    new_user->dst_username = malloc(len + 1);
    strncpy(new_user->dst_username, username, len);
    new_user->dst_username[len] = '\0';

    new_user->port = port;

    new_user->p2p_sd = p2p_sd;

    new_user->next = NULL;
    if(q == *users_chatting_with)
        *users_chatting_with = new_user;
    else 
        p->next = new_user;

    return;
}

void delChattingWithList(struct usersChattingWith** users_chatting_with) {
    
    struct usersChattingWith *del_user;
    while(*users_chatting_with != NULL) {
        del_user = (*users_chatting_with)->next;
        free((*users_chatting_with)->dst_username);
        free(*users_chatting_with);
        *users_chatting_with = del_user;
    }
    printf("Lista dei messaggi pendenti eliminata con successo!\n");
}

void printChattingWithList(struct usersChattingWith** users_chatting_with) {
    
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    struct usersChattingWith* elem = *users_chatting_with;
    while(elem != NULL) {
        printf("\nUSERNAME: %s\n", elem->dst_username);
        printf("PORT: %d\n", elem->port);
        elem = elem->next;
    }
    printf("FINE\n");
    return;
}

void delUserFromChattingWithList(struct usersChattingWith** users_chatting_with, int p2p_sd) {
    

    if(*users_chatting_with == NULL) {
        return;
    }

    if((*users_chatting_with)->p2p_sd == p2p_sd) {
        struct usersChattingWith* elem = *users_chatting_with;
        *users_chatting_with = (*users_chatting_with)->next;
        
        // elimino la memoria allocata per il messaggio
        free(elem->dst_username);
        // elimino la memoria allocata per la struttura
        free(elem);

        delUserFromChattingWithList(users_chatting_with, p2p_sd);
    } else {
        delUserFromChattingWithList(&(*users_chatting_with)->next, p2p_sd);
    }
}

int sendMessageToAll(struct usersChattingWith** users_chatting_with, char* message) {
    
    int ret;
    if(*users_chatting_with == NULL) {
        printf("\nLa lista è vuota.\n");
        return -1;
    }

    struct usersChattingWith* elem = *users_chatting_with;
    while(elem != NULL) {
        ret = send_TCP(&elem->p2p_sd, message);
        if(ret < 0) { return -1; }
        elem = elem->next;
    }
    printf("FINE\n");
    return 0;
}





/*
    Stampa a video i comandi disponibili a 
    seconda dello stato dell'utente.
*/
void printCommands(struct User user) {
    printf("\nI comandi disponibili sono:\n");
    if(user.user_state != LOGGED) {
        printf("1) in     --> per accedere al servizio.\n2) signup --> per creare un account.\n");
    } else if(user.user_state == LOGGED) {
        printf("1) hanging --> stampa il numero di messaggi ricevuti mentre si era offline.\n2) show    --> per ricevere i messaggi pendenti dall'utente specificato.\n3) chat    --> per chattare con un altro utente.\n4) share   --> per condividere un file.\n5) out     --> per disconnettersi.\n");
    }
}

/* 
    Permette a un utente di creare un account sul server, 
    caratterizzato da username e password.
*/
int signup(char* command, struct User* user, int* sd, struct sockaddr_in* server_addr) {

    int len;
    int ret;
    char* message; 
    
    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(user->my_username) + strlen(user->my_password) + strlen(user->srv_port) + 4;
    message = malloc(len);
    snprintf(message, len, "%s %s %s %s", command, user->my_username, user->my_password, user->srv_port);

    // stabilisco la connessione con il server
    ret = connect_to(sd, server_addr, SERVER_PORT);
    if(ret < 0) { printf("Impossibile connettersi al server.\n"); return -1; }
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire la registrazione.\n"); free(message); return -1; }

    // aspetto che il server mi comunichi che la registrazione è avvenuta con successo
    ret = receive_TCP(sd, message); 
    if(strncmp(message, "ok", 2)) {
        printf("Impossibile eseguire la registrazione.\n"); free(message); return -1;
    }

    // libero la memoria utilizzata per il messaggio
    free(message); 
    
    printf("Registrazione avvenuta con successo!\n");
    return 0;
}

/*
    Permette al device di richiedere al 
    server la connessione al servizio.
*/
int in(char* command, struct User* user, int* sd, struct sockaddr_in* server_addr) {
    
    int len;
    int ret;
    char* message; 

    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(user->my_username) + strlen(user->my_password) + 3;
    message = malloc(len);
    snprintf(message, len, "%s %s %s", command, user->my_username, user->my_password);

    // stabilisco la connessione con il server
    ret = connect_to(sd, server_addr, atoi(user->srv_port));
    if(ret < 0) { printf("Impossibile connettersi al server.\n"); return -1; }
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Errore nell'invio del messaggio al server.\n"); free(message); return -1; }

    // aspetto che il server mi comunichi che il login è avvenuto con successo
    ret = receive_TCP(sd, message); 
    if(ret < 0) { printf("Errore nella ricezione del messaggio dal server.\n"); free(message); return -1; }

    if(!strncmp(message, "no", 2)) {
        
        // l'operazione non è andata a buon fine quindi mi disconnetto dal server
        disconnect_to(sd); 
        free(message); 
        printf("Username o password non validi.\n");

    } else if(!strncmp(message, "ok", 2)) {
        printf("Login avvenuto con successo!\n"); 
        free(message); 
        return 0;
    }
    
    return -1;
}

/*
    Permette all'utente di ricevere la lista degli utenti 
    che gli hanno inviato messaggi mentre era offline.
    Per ogni utente il comando mostra username, il numero di
    messaggi pendenti in ingresso e il timestamp del più recente.
*/
void hanging(char* command, int* sd) {
    
    int ret;
    char buffer[BUFFER_SIZE];
    int num_users;
    int i;

    // invio al server il comando
    ret = send_TCP(sd, command);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); return; }

    memset(&buffer, '\0', BUFFER_SIZE);

    // ricevo dal server il numero di utenti che hanno inviato messaggi
    ret = receive_TCP(sd, buffer);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); return; }
    num_users = atoi(buffer);
    if(num_users == 0) {
        printf("\nNessuno ha inviato messaggi.\n");
        return;
    }

    printf("\nGli utenti che hanno mandato messaggi sono:\n");

    // ricevo dal server tutte le info sui messaggi inviati
    for(i = 0; i < num_users; i++) {
        
        memset(&buffer, '\0', BUFFER_SIZE);
        
        ret = receive_TCP(sd, buffer);
        if(ret < 0) { printf("Hanging: errore nella ricezione di una riga.\n"); continue; }
        
        // stampo le info
        printf("%d. %s\n", i + 1, buffer);
    }

    return;
}

/*
    Consente all'utente di ricevere i messaggi 
    pendenti dall'utente username.
*/
void show(char* command, int* sd, char* username) {
    
    int len;
    int ret;
    int num_messages;
    int i;
    char* message; 
    char buffer[BUFFER_SIZE];

    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(username) + 2; // il +2 serve per gli spazi
    message = malloc(len);
    snprintf(message, len, "%s %s", command, username);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire la show.\n"); free(message); return; }
    
    // libero la memoria utilizzata per il messaggio
    free(message);

    memset(&buffer, '\0', BUFFER_SIZE);

    // ricevo dal server il numero di messaggi ricevuti
    ret = receive_TCP(sd, buffer);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); return; }
    num_messages = atoi(buffer);
    if(num_messages == 0) {
        printf("\nQuesto utente non ha inviato messaggi.\n");
        return;
    }

    printf("\nI messaggi ricevuti sono:\n");

    // ricevo dal server tutte le info sui messaggi inviati
    for(i = 0; i < num_messages; i++) {
        
        memset(&buffer, '\0', BUFFER_SIZE);
        
        ret = receive_TCP(sd, buffer);
        if(ret < 0) { printf("Show: errore nella ricezione del messaggio.\n"); continue; }
        
        // stampo il messaggio
        printf("%s\n", buffer);
    }

    return;
}

/*
    Verifica se dst_username è presente all'interno della
    rubrica di my_username. 
*/
bool checkContacts(char* my_username, char* dst_username) {

    int len;
    FILE* fp;
    char* file_path;
    char file_line[20];
    char* file_username;

    // creo il path del file rubrica
    len = strlen(my_username) + 23;
    file_path = malloc(len);
    strncpy(file_path, "./client/contacts/", 18);
    strncat(file_path, my_username, len);
    strcat(file_path, ".txt");
    file_path[len - 1] = '\0';

    // apro la rubrica
    fp = fopen(file_path, "r");
    if(fp == NULL) { printf("Error0 chat\n"); return -1; }

    // verifico che il destinatario sia presente in rubrica
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {

        file_username = strtok(file_line, "\n");

        len = (strlen(dst_username) > strlen(file_username))? strlen(dst_username):strlen(file_username);

        if(!strncmp(file_username, dst_username, len)) {
            return true;
        }
    }

    fclose(fp);
    return false;
}

/*
    Avvia una chat con l'utente dst_username.
*/
int chat(char* command, int* sd, char* my_username, char* dst_username) {
    
    int len;
    int ret;
    char message[1024]; 
    
    // controllo se il contatto è in rubrica
    if(!checkContacts(my_username, dst_username)) { 
        printf("Il contatto indicato non è in rubrica.\n"); 
        return -1; 
    }

    // unisco le tre stringhe per inviare al server un solo messaggio
    memset(&message, '\0', sizeof(message));
    len = strlen(command) + strlen(dst_username) + 2; // il +2 serve per gli spazi
    snprintf(message, len, "%s %s", command, dst_username);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile iniziare la chat.\n"); return -1; }

    // pulisco il buffer per ricevere la risposta
    memset(&message, '\0', sizeof(message));

    // aspetto la risposta dal server
    ret = receive_TCP(sd, message);
    if(ret < 0) { printf("Errore durante la ricezione della risposta dal server\n"); return -1; }

    if(!strncmp(message, "offline", 7)) { // caso in cui il destinatario è offline
        printf("Il destinatario è offline.\n");
        return -4;
    }

    printf("Chat iniziata con successo! La porta del destinatario è: %d\n", atoi(message));
    
    // ritorno la porta del destinatario
    return atoi(message);
}

/*
    Invia il file file_name al device su cui è connesso
    l'utente o gli utenti con cui si sta chattando. 
*/
void share(char* command, int* sd, char* file_name) {
    
    int len;
    int ret;
    char* message; 

    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(file_name) + 2; // il +2 serve per gli spazi
    message = malloc(len);
    snprintf(message, len, "%s %s", command, file_name);
    
    // invio al server il messaggio
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile fare la share.\n"); free(message); return; }
    
    // libero la memoria utilizzata per il messaggio
    free(message);

    printf("Share avvenuta con successo!\n");
    return;
}

/*
    Permette al device di disconnettersi dal server.
*/
void out(int* sd, struct User* user) {
    
    int ret;
    
    // invio la richiesta di disconnessione
    ret = disconnect_to(sd);
    if(ret == -1) { printf("Impossibile disconnettersi.\n"); }
    printf("Disconnessione avvenuta con successo!\n");

    // libero la memoria allocata per i dati dell'utente
    free(user->my_username);
    free(user->my_password);
    free(user->srv_port);
    free(user->my_port);
}

/*
    Permette di ricevere le notifiche mentre si era offline.
*/
void receiveNotifications(int* sd, char* buffer) {
    
    int ret;
    int num_notifications;
    int i;

    ret = receive_TCP(sd, buffer);
    if(ret < 0) { printf("Impossibile ricevere le notifiche\n"); return; }

    num_notifications = atoi(buffer);
    if(num_notifications == 0) { return; }
    else if(num_notifications == 1) { printf("Hai ricevuto %d notifica:\n", num_notifications); }
    else { printf("Hai ricevuto %d notifiche:\n", num_notifications); }

    // ricevo le notifiche
    for(i = 0; i < num_notifications; i++) {
        // pulisco il buffer
        memset(buffer, '\0', BUFFER_SIZE);
        
        ret = receive_TCP(sd, buffer);
        if(ret < 0) { printf("Impossibile ricevere la notifica\n"); continue; }

        printf("%s\n", buffer);
    }
}

/*
    A seconda del comando digitato dall'utente
    si esegue la funzione corrispondente.
*/
int executeDeviceCommand(char* buffer, struct User* user, int* sd, struct sockaddr_in* server_addr) {

    int ret;
    int len;
    char* command = NULL;
    char* file_name = NULL;
    char* temp_username;
    char* temp_port;
    char* temp_password;
    
    // prendo il comando inserito 
    command = strtok(buffer, " ");

    // controllo che per i comandi in e signin l'utente sia disconnesso
    // per gli altri comandi l'utente deve essere connesso.
    // Poi a seconda del comando inserito prendo i parametri e chiamo la funzione
    if(user->user_state == DISCONNECTED) {
        
        if(!strncmp(command, "in", 2)) {
            
            temp_port = strtok(NULL, " ");
            temp_username = strtok(NULL, " ");
            temp_password = strtok(NULL, " ");

            // controllo che l'utente abbia inserito i dati
            if(temp_port == NULL || temp_username == NULL || temp_password == NULL) { return -1; }

            // salvo in memoria la porta del server
            len = strlen(temp_port);
            user->srv_port = malloc(len + 1);
            strncpy(user->srv_port, temp_port, len);
            user->srv_port[len] = '\0';
            
            // salvo in memoria l'username dell'utente
            len = strlen(temp_username);
            user->my_username = malloc(len + 1);
            strncpy(user->my_username, temp_username, len);
            user->my_username[len] = '\0';
            
            // salvo in memoria la password dell'utente
            len = strlen(temp_password);
            user->my_password = malloc(len + 1);
            strncpy(user->my_password, temp_password, len);
            user->my_password[len] = '\0';

            // inizializzo la lista degli utenti con cui si sta chattando
            user->users_chatting_with = NULL;
             
            ret = in(command, user, sd, server_addr);
            if(ret == 0) { user->user_state = LOGGED; }

        } else if(!strncmp(command, "signup", 6)) { 
            
            temp_username = strtok(NULL, " ");
            temp_password = strtok(NULL, " ");
            
            // controllo che l'utente abbia inserito i dati
            if(temp_username == NULL || temp_password == NULL) { return -1; }
            
            
            // salvo in memoria la porta del server
            user->srv_port = malloc(5);
            strncpy(user->srv_port, "4242", len);
            user->srv_port[len] = '\0';
            
            // salvo in memoria l'username dell'utente
            len = strlen(temp_username);
            user->my_username = malloc(len + 1);
            strncpy(user->my_username, temp_username, len);
            user->my_username[len] = '\0';
            
            // salvo in memoria la password dell'utente
            len = strlen(temp_password);
            user->my_password = malloc(len + 1);
            strncpy(user->my_password, temp_password, len);
            user->my_password[len] = '\0';

            // inizializzo la lista degli utenti con cui si sta chattando
            user->users_chatting_with = NULL;

            ret = signup(command, user, sd, server_addr);
            if(ret == 0) { user->user_state = LOGGED; }
        
        } else { // in caso di comando non valido restituisco -1
            return -1;
        }
    } else if(user->user_state == LOGGED) {
        if(!strncmp(command, "hanging", 7)) {
            
            hanging(command, sd);
        } else if(!strncmp(command, "show", 4)) {
            temp_username = strtok(NULL, " ");

            // controllo che l'utente abbia inserito i dati
            if(temp_username == NULL) { return -2; }
            
            show(command, sd, temp_username);
        } else if(!strncmp(command, "chat", 4)) {
            
            temp_username = strtok(NULL, " ");

            // controllo che l'utente abbia inserito i dati
            if(temp_username == NULL) { return -2; }

            // mi salvo il destinatario del messaggio
            len = strlen(temp_username);
            user->dst_username = malloc(len + 1);
            strncpy(user->dst_username, temp_username, len);
            user->dst_username[len] = '\0';
            
            ret = chat(command, sd, user->my_username, user->dst_username);
            
            return ret;
        } else if(!strncmp(command, "share", 5)) {
            file_name = strtok(NULL, " ");

            // controllo che l'utente abbia inserito i dati
            if(file_name == NULL) { return -2; }
            
            share(command, sd, file_name);
        } else if(!strncmp(command, "out", 3)) {
            out(sd, user);
            
            user->user_state = DISCONNECTED;
            return -3;
        } else { // in caso di comando non valido restituisco -2
            return -2;
        }
    }

    return 0;
}