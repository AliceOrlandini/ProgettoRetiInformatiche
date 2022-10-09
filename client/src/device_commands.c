#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "./../include/device_commands.h"
#include "./../include/device_consts.h"
#include "./../../network/include/network.h"

/*
    Stampa a video i comandi disponibili a 
    seconda dello stato dell'utente.
*/
void printCommands(struct User user) {
    
    printf("\nI comandi disponibili sono:\n");
    if(user.user_state != LOGGED) {
        printf("1) in     --> per accedere al servizio.\n2) signup --> per creare un account.\n");
    } else if(user.user_state == LOGGED) {
        printf("1) hanging --> stampa il numero di messaggi ricevuti mentre si era offline.\n2) show    --> per ricevere i messaggi pendenti dall'utente specificato.\n3) chat    --> per chattare con un altro utente.\n4) share   --> per condividere un file. (solo durante la chat)\n5) out     --> per disconnettersi.\n");
    }
}

/* 
    Permette a un utente di creare un account sul server caratterizzato 
    da username e password. Viene stabilita una connessione con il server 
    e successivamente viene inviato il comando. Se si riceve un "ok" come 
    risposta allora la registrazione è avvenuta con successo.
*/
int signup(char* command, struct User* user, int* server_sd, struct sockaddr_in* server_addr) {

    int len;
    int ret;
    char* message; 
    
    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(user->my_username) + strlen(user->my_password) + strlen(user->srv_port) + 4;
    message = malloc(len);
    snprintf(message, len, "%s %s %s %s", command, user->my_username, user->my_password, user->srv_port);

    // stabilisco la connessione con il server
    ret = connect_to(server_sd, server_addr, SERVER_PORT);
    if(ret < 0) { printf("Impossibile connettersi al server.\n"); return -1; }
    
    // invio al server il messaggio
    ret = send_TCP(server_sd, message);
    if(ret < 0) { printf("Impossibile eseguire la registrazione.\n"); free(message); return -1; }

    // aspetto che il server mi comunichi che la registrazione è avvenuta con successo
    ret = receive_TCP(server_sd, message); 
    if(ret == -2) { 
        printf("Il server si è disconnesso.\n"); 
        disconnect_to(server_sd); 
        free(message);
        return -1; 
    }
    if(strncmp(message, "ok", 2)) {
        printf("Impossibile eseguire la registrazione.\n"); free(message); return -1;
    }

    // libero la memoria utilizzata per il messaggio
    free(message); 
    
    printf("Registrazione avvenuta con successo!\n");
    return 0;
}

/*
    Permette al device di richiedere al server la connessione al servizio.
    Viene stabilita una connessione con il server e successivamente vengono 
    inviati i dati per l'autenticazione. Se si riceve un "ok" come 
    risposta allora la registrazione è avvenuta con successo.
*/
int in(char* command, struct User* user, int* server_sd, struct sockaddr_in* server_addr) {
    
    int len;
    int ret;
    char* message; 

    // unisco le tre stringhe per inviare un solo messaggio
    len = strlen(command) + strlen(user->my_username) + strlen(user->my_password) + 3;
    message = malloc(len);
    snprintf(message, len, "%s %s %s", command, user->my_username, user->my_password);

    // stabilisco la connessione con il server
    ret = connect_to(server_sd, server_addr, atoi(user->srv_port));
    if(ret < 0) { printf("Impossibile connettersi al server.\n"); return -1; }
    
    // invio al server il messaggio
    ret = send_TCP(server_sd, message);
    if(ret < 0) { printf("Errore nell'invio del messaggio al server.\n"); free(message); return -1; }

    // aspetto che il server mi comunichi che il login è avvenuto con successo
    ret = receive_TCP(server_sd, message); 
    if(ret == -2) { 
        printf("Il server si è disconnesso.\n"); 
        disconnect_to(server_sd); 
        free(message); 
        return -1; 
    }
    if(ret < 0) { printf("Errore nella ricezione del messaggio dal server.\n"); free(message); return -1; }

    if(!strncmp(message, "no", 2)) {
        
        // l'operazione non è andata a buon fine quindi mi disconnetto dal server
        disconnect_to(server_sd); 
        printf("Username o password non validi.\n");

    } else if(!strncmp(message, "ok", 2)) {
        printf("Login avvenuto con successo!\n"); 
        return 0;
    }

    // libero la memoria allocata per il messaggio
    free(message);   
    return -1;
}

/*
    Permette all'utente di ricevere la lista degli utenti 
    che gli hanno inviato messaggi mentre era offline.
    Per ogni utente il comando mostra username, il numero di
    messaggi pendenti in ingresso e il timestamp del più recente.
*/
void hanging(char* command, int* server_sd) {
    
    int ret;
    char buffer[BUFFER_SIZE];
    int num_users;
    int i;

    // invio al server il comando
    ret = send_TCP(server_sd, command);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); return; }

    // pulisco il buffer
    memset(&buffer, '\0', BUFFER_SIZE);

    // ricevo dal server il numero di utenti che hanno inviato messaggi
    ret = receive_TCP(server_sd, buffer);
    if(ret == -2) { printf("Il server si è disconnesso.\n"); disconnect_to(server_sd); return; }
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
        
        // ricevo l'username dell'utente
        ret = receive_TCP(server_sd, buffer);
        if(ret == -2) { 
            printf("Il server si è disconnesso.\n"); 
            disconnect_to(server_sd); 
            return; 
        }
        if(ret < 0) { printf("Hanging: errore nella ricezione di una riga.\n"); continue; }
        
        // stampo le info
        printf("%d. %s\n", i + 1, buffer);
    }

    return;
}

/*
    Consente all'utente di ricevere i messaggi 
    pendenti dall'utente specificato come parametro.
*/
void show(char* command, int* server_sd, char* username) {
    
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
    ret = send_TCP(server_sd, message);
    if(ret < 0) { printf("Impossibile eseguire la show.\n"); free(message); return; }
    
    // libero la memoria utilizzata per il messaggio
    free(message);

    // pulisco il buffer
    memset(&buffer, '\0', BUFFER_SIZE);

    // ricevo dal server il numero di messaggi ricevuti
    ret = receive_TCP(server_sd, buffer);
    if(ret == -2) { printf("Il server si è disconnesso.\n"); disconnect_to(server_sd); return; }
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); return; }
    num_messages = atoi(buffer);
    if(num_messages == 0) {
        printf("\nQuesto utente non ha inviato messaggi.\n");
        return;
    }

    printf("\nI messaggi ricevuti sono:\n");

    // ricevo dal server tutte le info sui messaggi inviati
    for(i = 0; i < num_messages; i++) {
        
        // pulisco il buffer
        memset(&buffer, '\0', BUFFER_SIZE);
        
        // ricevo il messaggio
        ret = receive_TCP(server_sd, buffer);
        if(ret == -2) { 
            printf("Il server si è disconnesso.\n"); 
            disconnect_to(server_sd); 
            return; 
        }
        if(ret < 0) { printf("Show: errore nella ricezione del messaggio.\n"); continue; }
        
        // stampo il messaggio
        printf("%s\n", buffer);
    }

    return;
}

/*
    Verifica se dst_username è presente all'interno della
    rubrica di my_username. Per fare ciò, scorre il file
    dei contatti presente nella cartella contacts.
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

    // chiudo il file
    fclose(fp);
    return false;
}

/*
    Avvia una chat con l'utente dst_username. Prima controlla se l'utente
    è presente in rubrica, poi invia la richiesta al server.
    Se l'utente è online il server invierà la porta dell'utente, 
    altrimenti i messaggi verranno salvati sul server. 
*/
int chat(char* command, int* server_sd, char* my_username, char* dst_username) {
    
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
    ret = send_TCP(server_sd, message);
    if(ret < 0) { printf("Impossibile iniziare la chat.\n"); return -1; }

    // pulisco il buffer per ricevere la risposta
    memset(&message, '\0', sizeof(message));

    // aspetto la risposta dal server
    ret = receive_TCP(server_sd, message);
    if(ret == -2) { 
        printf("Il server si è disconnesso.\n"); 
        disconnect_to(server_sd); 
        return -1; 
    }
    if(ret < 0) { printf("Errore durante la ricezione della risposta dal server\n"); return -1; }
    
    // controllo se il destinatario è offline
    if(!strncmp(message, "offline", 7)) { 
        printf("Il destinatario è offline.\n");
        return -4;
    }

    printf("Chat iniziata con successo! La porta del destinatario è: %d\n", atoi(message));
    
    // se il destinatario è online ritorno la sua porta 
    return atoi(message);
}

/*
    Permette al device di disconnettersi dal server.
    Inoltre libera anche la memoria allocata per l'utente.
*/
void out(int* server_sd, struct User* user) {
    
    int ret;

    // controllo se il descrittore del socket è attivo
    // (potrebbe non essere attivo in caso di disconnessione del server)
    ret = fcntl(*server_sd, F_GETFD);
    if(ret == -1) { printf("Il server era già disconnesso\n"); return; }
    
    // invio la richiesta di disconnessione
    ret = disconnect_to(server_sd);
    if(ret == -1) { printf("Impossibile disconnettersi.\n"); }
    printf("Disconnessione avvenuta con successo!\n");

}

/*
    Permette di ricevere le notifiche mentre si era offline.
    Questa funzione viene eseguita quando il device inizia 
    la sua esecuzione.
*/
void receiveNotifications(int* server_sd, char* buffer) {
    
    int ret;
    int num_notifications;
    int i;

    // ricevo il numero di notifiche
    ret = receive_TCP(server_sd, buffer);
    if(ret == -2) { 
        printf("Il server si è disconnesso.\n"); 
        disconnect_to(server_sd); 
        return; 
    }
    if(ret < 0) { printf("Impossibile ricevere le notifiche\n"); return; }

    num_notifications = atoi(buffer);

    // se si hanno zero notifiche ritorno
    if(num_notifications == 0) { return; }
    else if(num_notifications == 1) { printf("Hai ricevuto %d notifica:\n", num_notifications); }
    else { printf("Hai ricevuto %d notifiche:\n", num_notifications); }

    // ricevo le notifiche
    for(i = 0; i < num_notifications; i++) {
        // pulisco il buffer
        memset(buffer, '\0', BUFFER_SIZE);
        
        // ricevo la notifica
        ret = receive_TCP(server_sd, buffer);
        if(ret == -2) { 
            printf("Il server si è disconnesso.\n"); 
            disconnect_to(server_sd); 
            return; 
        }
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
            // stampa di informazione
            printf("Questo comando è invocabile solo quando si sta chattando.\n");
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