#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "./../include/device_requests.h"
#include "./../include/server_consts.h"
#include "./../../network/include/network.h"

/**
 * Permette ad un utente di effettuare il login.
 * Si imposta il campo timestamp_logout a NULL 
 * all'interno del file db_users.txt ad indicare
 * che l'utente è online. 
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 * @param dev_username puntatore al buffer contenente il nome dell'utente.
 * @param dev_password puntatore al buffer contenente la password dell'utente.
 */
void in(int* sd, char* dev_username, char* dev_password) {
    
    char file_line[64];
    char* username;
    char* password;
    char* port;
    int username_len;
    int password_len;
    int ret;
    int position = 0;
    int last_len; 
    FILE *fp;
    time_t t;
    char timestamp_login[TIMESTAMP_SIZE]; 

    memset(&file_line, '\0', sizeof(file_line));
    
    // apro il file db_users.txt sia in lettura che in scrittura
    fp = fopen("./server/files/db_users.txt", "r+");
    if(fp == NULL) { printf("Error0 in\n"); return; }

    // verifico che il device sia effettivamente registrato
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {

        // aggiorno queste variabili che serviranno per
        // andare a sovrascrivere il timestamp_logout
        last_len = strlen(file_line);
        position += last_len;
    
        // prelevo i dati per effettuare l'autentificazione
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");

        username_len = (strlen(username) > strlen(dev_username))? strlen(username):strlen(dev_username);
        password_len = (strlen(password) > strlen(dev_password))? strlen(password):strlen(dev_password);
        
        // controllo se ho trovato l'username e la password è corretta
        if(!strncmp(dev_username, username, username_len) && !strncmp(dev_password, password, password_len)) {
        
            // calcolo il timestamp del login
            t = time(NULL);
            strftime(timestamp_login, sizeof(timestamp_login), "%H:%M:%S", localtime(&t));

            // registro che l'utente si è connesso aggiornando 
            // il timestamp_login e il timestamp_logout 
            position -= last_len;
            fseek(fp, position, SEEK_SET);
            fprintf(fp, "%s %s %s %s NULL    ", username, password, port, timestamp_login);

            // chiudo il file
            fclose(fp);

            // comunico all'utente che il login è stato effettuato
            ret = send_TCP(sd, "ok");
            if(ret < 0) { printf("Error2 in\n"); return; }

            printf("\n%s ha effettuato il login!\n", dev_username);
            return;
        }
    }

    // chiudo il file
    fclose(fp);

    // comunico all'utente che l'username o la password non sono validi
    ret = send_TCP(sd, "no");
    if(ret < 0) { printf("Error1 in\n"); } 

    return;
}

/**
 * Permette a un utente di registrarsi con un username e una password.
 * Per fare ciò si aggiunge una riga nel file db_users.txt contenente
 * le informazioni dell'utente.
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 * @param dev_username puntatore al buffer contenente il nome dell'utente.
 * @param dev_password puntatore al buffer contenente la password dell'utente.
 * @param dev_port puntatore al buffer contenente la porta associata all'utente.
 */
void signup(int* sd, char* dev_username, char* dev_password, char* dev_port) {
    
    int ret; 
    time_t t;
    char timestamp_login[TIMESTAMP_SIZE];
    FILE *fp;

    // apro il file db_users.txt in append
    fp = fopen("./server/files/db_users.txt", "a");
    if(fp == NULL) { 
        printf("Error0 signup\n"); 
        // comunico al client c'è stato un errore
        ret = send_TCP(sd, "no");
        if(ret < 0) { printf("Error1 signup\n"); return; }
        return; 
    }

    // calcolo il timestamp del login
    t = time(NULL);
    strftime(timestamp_login, sizeof(timestamp_login), "%H:%M:%S", localtime(&t));

    // inserisco il nuovo record che sarà: 
    // username password port timestamp_login timestamp_logout
    ret = fprintf(fp, "%s %s %s %s NULL    \n", dev_username, dev_password, dev_port, timestamp_login);
    if(ret < 0) { 
        printf("Error2 signup\n"); 
        // comunico al client c'è stato un errore
        ret = send_TCP(sd, "no");
        if(ret < 0) { printf("Error3 signup\n"); return; }
        return; 
    }

    // comunico al client che la registrazione è avvenuta con successo
    ret = send_TCP(sd, "ok");
    if(ret < 0) { printf("Error4 signup\n"); return; }

    // chiudo il file
    fclose(fp);

    printf("\n%s si è registrato!\n", dev_username);
}

/**
 * Scorre la lista dei messaggi pendenti e per ogni utente conta il
 * numero di messaggi pendenti inviati e il timestamp del più recente.
 * Infine, invia tutti i risultati al client.
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 */
void hanging(int* sd, struct pendingMessage** pending_message_list) {
    
    char* username[32];
    int num_messages[32];
    char* timestamp[32];
    int num_users = 0;
    int len;
    int i;
    char* message;
    int ret;
    struct pendingMessage* elem = NULL;

    // controllo se la lista dei messaggi pendenti è vuota
    if(*pending_message_list == NULL) {

        // mando al client l'informazione che nessuno gli ha inviato messaggi
        message = malloc(3);
        sprintf(message, "%d", num_users);
        ret = send_TCP(sd, message);
        if(ret < 0) { printf("Impossibile eseguire hanging.\n"); }
        free(message);
        return;
    }

    // inizializzo i vettori
    for(i = 0; i < 32; i++) {
        username[i] = NULL;
        num_messages[i] = 0;
        timestamp[i] = NULL;
    }

    // scorro la lista dei messaggi pendenti e per ogni utente
    // salvo il numero di messaggi inviati
    elem = *pending_message_list;
    while(elem != NULL) {
        for(i = 0; i < 32; i++) {
            // se l'username in lista è NULL allora mi salvo il nuovo utente
            if(username[i] == NULL) {
                username[i] = elem->username_src;
                num_messages[i]++;
                timestamp[i] = elem->timestamp;
                num_users++;
                break;
            } 

            // se trovo un utente già salvato incremento il numero di 
            // messaggi e il timestamp del messaggio più recente
            len = (strlen(username[i]) > strlen(elem->username_src))? strlen(username[i]):strlen(elem->username_src);
            if(!strncmp(username[i], elem->username_src, len)) {
                num_messages[i]++;
                timestamp[i] = elem->timestamp;
                break;
            }
        }
        elem = elem->next;
    }

    // invio al client il numero di utenti che gli hanno inviato messaggi
    message = malloc(3);
    sprintf(message, "%d", num_users);
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); free(message); return; }
    free(message);
    
    // invio al client tutte le righe
    for(i = 0; i < num_users; i++) {
        
        // creo il messaggio da inviare al client
        len = strlen(username[i]) + strlen(timestamp[i]) + 5;
        message = malloc(len);
        snprintf(message, len, "%s %d %s", username[i], num_messages[i], timestamp[i]);
        
        // invio il messaggio
        ret = send_TCP(sd, message);
        if(ret < 0) { printf("Impossibile eseguire hanging.\n"); }
        
        // libero la memoria allocata per il messaggio
        free(message);
    }
    return;
}

/**
 * Funzione per verificare se un utente è online o meno.
 * Per fare ciò, scorre il file db_users.txt e controlla
 * se il campo timestamp_logout dell'utente specificato
 * nel parametro è NULL.
 * 
 * @param dst_username puntatore al buffer contenente il nome dell'utente.
 * @param port puntatore al buffer contenente la porta associata all'utente.
 * @return vero se l'utente è online, falso altrimenti.
 */
bool checkOnline(char* dst_username, char* port) {
    
    int ret;
    FILE* fp;
    char file_line[64];
    int username_len;
    char* username;
    char* password;
    char* tmp_port;
    char* timestamp_login;
    char* timestamp_logout;

    // apro il file db_users.txt in lettura
    fp = fopen("./server/files/db_users.txt", "r"); 
    if(fp == NULL) { printf("Error0 chat\n"); return false; }

    // se l'utente è online allora avrà campo NULL nel file
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        
        // ricavo i dati del destinatario
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");

        // la porta la salvo nel parametro passato nella funzione
        tmp_port = strtok(NULL, " ");
        memcpy(port, tmp_port, strlen(tmp_port));
        *(port + strlen(tmp_port)) = '\0';

        timestamp_login = strtok(NULL, " ");
        timestamp_logout = strtok(NULL, " ");

        // controllo se il timestamp del logout dell'utente 
        // specificato per parametro è NULL, nel caso ritorno true
        username_len = (strlen(username) > strlen(dst_username))? strlen(username):strlen(dst_username);
        if(!strncmp(timestamp_logout, "NULL", 4) && !strncmp(dst_username, username, username_len)) {
            // chiudo il file
            fclose(fp);
            return true;
        }
    }

    // chiudo il file e libero la memoria allocata per la porta
    fclose(fp);
    return false;
}

/**
 * Permette all'utente di ricevere i messaggi pendenti da dst_username.
 * Per fare ciò si scorre la lista dei messaggi pendenti e si inviano 
 * al client. 
 * Informa inoltre il mittente che i suoi messaggi sono stati letti.
 * Infine elimina quei messaggi dalla lista e dal file.
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 * @param src_username puntatore al buffer contenente il nome dell'utente di cui si vogliono leggere i messaggi.
 * @param my_username puntatore al buffer contenente il nome dell'utente.
 */
void show(int* sd, struct pendingMessage** pending_message_list, char* src_username, char* my_username) {
    
    int num_messages = 0;
    char* message;
    int ret;
    int len;
    int username_len;
    struct pendingMessage* elem;
    char* port;
    char dst_port[5];
    int new_sd;
    struct sockaddr_in dst_addr;
    FILE* fp;

    // controllo se la lista è vuota
    if(*pending_message_list == NULL) {
        
        // mando al client l'informazione che questo utente non gli ha inviato messaggi
        message = malloc(3);
        sprintf(message, "%d", num_messages);
        ret = send_TCP(sd, message);
        if(ret < 0) { printf("Impossibile eseguire hanging.\n"); }
        free(message);
        return;
    }

    // conto il numero di messaggi che src_username gli ha mandato
    elem = *pending_message_list;
    while(elem != NULL) {

        // se ho trovato l'username nella lista incremento il numero di messaggi
        username_len = (strlen(elem->username_src) > strlen(src_username))? strlen(elem->username_src):strlen(src_username);
        if(!strncmp(elem->username_src, src_username, username_len)) {
            num_messages++;
        }
        elem = elem->next;
    }

    // invio il numero di messaggi inviati al client
    message = malloc(3);
    sprintf(message, "%d", num_messages);
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile eseguire hanging.\n"); free(message); return; }
    free(message);

    // se i messaggi erano zero termino
    if(num_messages == 0) { return; }

    // altrimenti invio al client tutti i messaggi ricevuti da src_username
    elem = *pending_message_list;
    while(elem != NULL) {
        username_len = (strlen(elem->username_src) > strlen(src_username))? strlen(elem->username_src):strlen(src_username);
        if(!strncmp(elem->username_src, src_username, username_len)) {
            
            // creo il messaggio da inviare al client
            len = strlen(elem->timestamp) + strlen(elem->message) + 2;
            message = malloc(len);
            snprintf(message, len, "%s %s", elem->timestamp, elem->message);

            // invio il messaggio
            ret = send_TCP(sd, message);
            if(ret < 0) { printf("Impossibile eseguire hanging.\n"); }
            
            // libero la memoria allocata per il messaggio
            free(message);
        }
        elem = elem->next;
    }

    // controllo se l'utente src_username è online: 
    // in caso affermativo gli invio la notifica di avvenuta lettura dei messaggi
    // altrimenti mantengo l'informazione che verrà inviata quando tornerà online
    if(checkOnline(src_username, dst_port)) {
        
        // creo una connessione con il mittente
        ret = connect_to(&new_sd, &dst_addr, atoi(dst_port));
        if(ret < 0) { printf("Impossibile connettersi al device\n"); }
        else {
            // invio la notifica al mittente
            ret = send_TCP(&new_sd, "notifica");
            if(ret < 0) { printf("Impossibile inviare il messaggio\n"); }

            // creo il messaggio
            len = strlen(my_username);
            message = malloc(len);
            sprintf(message, "%s", my_username);

            // invio il messaggio al mittente
            ret = send_TCP(&new_sd, message);
            if(ret < 0) { printf("Impossibile inviare il messaggio\n"); }
            
            // libero la memoria allocata per il messaggio
            free(message);
            
            // mi disconnetto dal mittente
            disconnect_to(&new_sd);
        }
    } else {
        // apro il file db_notifications.txt in append
        fp = fopen("./server/files/db_notifications.txt", "a");
        if(fp == NULL) { printf("Error0 impossibile aprire il file notifications.txt\n"); return; }

        // salvo il testo della notifica su file
        // ret = fprintf(fp, "%s:I messaggi inviati a %s sono stati letti!\n", src_username, my_username);
        ret = fprintf(fp, "%s:%s\n", src_username, my_username);
        if(ret < 0) { printf("Error1 impossibile salvare la notifica\n"); return; }

        // chiudo il file
        fclose(fp);
    }

    // ora che ho inviato tutti i messaggi elimino dalla lista e dal file
    // db_messages.txt i messaggi mandati da src_username all'utente
    delMessagesFromFile(src_username, my_username, num_messages);
    delMessagesFromPMList(pending_message_list, src_username);
    return;
}

/**
 * Scorre il file delle db_notifications.txt ed invia all'utente 
 * le sue notifiche. Una volte inviate le elimina dal file.
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 * @param my_username puntatore al buffer contenente il nome dell'utente.
 */
void sendNotifications(int* sd, char* my_username) {

    FILE* fp;
    char file_line[64];
    char* username;
    char* notification;
    int username_len;
    int num_notifications = 0;
    char* message;
    int ret;
    int len;

    // apro il file db_notifications.txt in lettura
    fp = fopen("./server/files/db_notifications.txt", "r");
    if(fp == NULL) { printf("Error0 sendNotifications\n"); return; }

    // cerco il numero di notifiche inviate all'utente
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        username = strtok(file_line, ":");
        notification = strtok(NULL, "\n");

        // se ho trovato l'username incremento il numero di notifiche
        username_len = (strlen(username) > strlen(my_username))? strlen(username):strlen(my_username);
        if(!strncmp(my_username, username, username_len)) {
            num_notifications++;
        }
    } 

    // invio il numero di notifiche ricevute
    message = malloc(3);
    sprintf(message, "%d", num_notifications);
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile inviare il numero di notifiche\n"); free(message); return; }
    free(message);

    // se il numero di notifiche è zero termino
    if(num_notifications == 0) { fclose(fp); return; }

    // riavvolgo il file
    rewind(fp);

    // invio al client tutte le notifiche 
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        username = strtok(file_line, ":");
        notification = strtok(NULL, "\n");

        // se ho trovato l'username invio la notifica
        username_len = (strlen(username) > strlen(my_username))? strlen(username):strlen(my_username);
        if(!strncmp(my_username, username, username_len)) {
            
            // creo il messaggio da inviare al client
            len = strlen(notification) + 1;
            message = malloc(len);
            snprintf(message, len, "%s", notification);

            // invio il messaggio
            ret = send_TCP(sd, message);
            if(ret < 0) { printf("Impossibile inviare la notifica\n"); }
            
            // libero la memoria allocata per il messaggio
            free(message);
        }
    } 

    // chiudo il file
    fclose(fp);
    
    // elimino le notifiche nel file
    delNotificationsFromFile(username, num_notifications);
}

/**
 * Permette ad un utente di iniziare una chat con dst_username.
 * Se questo non dovesse essere online i messaggi verranno salvati.
 * 
 * @param sd puntatore al socket descriptor dell'utente.
 * @param dst_username puntatore al buffer che contiene l'username dell'utente.
 * @return 0 se il destinatario è online, 2 se i messsaggi verranno salvati sul server.  
 */
int chat(int* sd, char* dst_username) { // QUI
    
    int ret; 
    char port[5];

    if(checkOnline(dst_username, port)) {
        // nel caso in cui il destinatario sia online invio 
        // al client la porta del destinatario in modo che questo 
        // possa instaurare una comunicazione p2p
        ret = send_TCP(sd, port);
        if(ret < 0) { printf("Impossibile iniziare la chat\n"); return -1; }
        return 0;
    } else {
        // nel caso in cui il destinatario sia offline 
        // restituisco al client l'avviso che il messaggio
        // è stato salvato
        ret = send_TCP(sd, "offline");
        if(ret < 0) { printf("Impossibile salvare la chat\n"); return -1; }
        return 2;
    }
}

/**
 * Permette ad un utente di disconnettersi.
 * Salva l'ultimo accesso sul file db_users.txt.
 * 
 * @param dev_username puntatore al buffer che contiene l'username dell'utente.
 */
void out(char* dev_username) {

    char file_line[64];
    char* username;
    char* password;
    char* port;
    char* timestamp_login;
    int username_len;
    int last_len;
    int position = 0;
    time_t t;
    char timestamp_logout[TIMESTAMP_SIZE];
    FILE *fp;

    // apro il file db_users.txt sia in lettura che in scrittura
    fp = fopen("./server/files/db_users.txt", "r+"); 
    if(fp == NULL) { printf("Error0 signup\n"); return; }

    // scorro tutto il file finchè non trovo l'username
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {

        // queste variabili servono per sovrascrivere il timestamp_logout
        last_len = strlen(file_line);
        position += last_len;

        // prelevo i dati dell'utente
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");

        // controllo se ho trovato l'username
        username_len = (strlen(username) > strlen(dev_username))? strlen(username):strlen(dev_username);
        if(!strncmp(dev_username, username, username_len)) {

            // calcolo il timestamp del logout
            t = time(NULL);
            strftime(timestamp_logout, sizeof(timestamp_logout), "%H:%M:%S", localtime(&t));

            // registro che l'utente si è connesso aggiornando il timestamp_logout
            position -= last_len;
            fseek(fp, position, SEEK_SET);

            fprintf(fp, "%s %s %s %s %s", username, password, port, timestamp_login, timestamp_logout);

            // chiudo il file
            fclose(fp);

            printf("\n%s ha effettuato il logout!\n", dev_username);
            return;
        }
    }
    return;
}

/**
 * Funzione per salvare un messaggio sul file db_messages.txt 
 * che contiene tutti i messaggi pendenti.
 * 
 * @param message puntatore al buffer che contiene il messaggio da salvare.
 */
void saveMessage(char* message) {

    FILE* fp;
    int ret;
    time_t t;
    char timestamp[TIMESTAMP_SIZE];

    // apro il file db_messages.txt in append
    fp = fopen("./server/files/db_messages.txt", "a");
    if(fp == NULL) { printf("Error0 saveMessage\n"); return; }

    // calcolo il timestamp del login
    t = time(NULL);
    strftime(timestamp, sizeof(timestamp), "%H:%M:%S", localtime(&t));

    // inserisco il nuovo record che sarà: 
    // timestamp username_src username_dst message
    ret = fprintf(fp, "%s %s\n", timestamp, message);
    if(ret < 0) { printf("Error1 saveMessage\n"); return; }

    // chiudo il file
    fclose(fp);

    return; 
}

/**
 * Stampa a video il contenuto di un file.
 * Viene utilizzata a scopo di debug.
 * 
 * @param fptr puntatore al file da stampare.
 */
void printFile(FILE *fptr) {
    
    char ch;

    // prelevo un carattere da file e lo stampo
    while((ch = fgetc(fptr)) != EOF)
        putchar(ch);
}

/**
 * Elimina le righe di un file specificate nel vettore lines.
 * 
 * @param srcFile puntatore al file da cui bisogna eliminare le righe.
 * @param tempFile puntatore al file di appoggio.
 * @param lines puntatore al vettore che contiene le righe da eliminare.
 * @param num_lines numero di righe da eliminare.
 */
void deleteLine(FILE *srcFile, FILE *tempFile, int* lines, int num_lines) {
    
    char buffer[BUFFER_SIZE];
    int count = 1;
    int i;
    bool found;

    // scorro tutto il file
    while ((fgets(buffer, BUFFER_SIZE, srcFile)) != NULL) {
        found = false;
        
        // per ogni riga se la trovo la elimino
        for(i = 0; i < num_lines; i++) {
            if (lines[i] == count) { found = true; break; } 
        }
        if(!found) { fputs(buffer, tempFile); }

        count++;
    }
}

/**
 * Permette di eliminare dal file db_messages.txt
 * i messaggi mandati da src_username a dst_username.
 * 
 * @param src_username puntatore al buffer contenente l'username dell'utente che ha inviato i messaggi.
 * @param dst_username puntatore al buffer contenente l'username dell'utente che ha ricevuto i messaggi.
 * @param num_messages numero di messaggi da eliminare.
 */
void delMessagesFromFile(char* src_username, char* dst_username, int num_messages) {

    FILE *srcFile;
    FILE *tempFile;
    int lines[num_messages];
    int line = 0;
    int i = 0;
    
    int ret;
    char file_line[BUFFER_SIZE];
    char* timestamp;
    char* username_src;
    char* username_dst;
    int username_src_len;
    int username_dst_len;

    // inizializzo il vettore delle righe
    for(i = 0; i < num_messages; i++)
        lines[i] = 0;
    i = 0;

    // apro i file uno in lettura e uno in scrittura
    srcFile = fopen("./server/files/db_messages.txt", "r");
    tempFile = fopen("./server/files/delete-line.txt", "w");
    if(srcFile == NULL || tempFile == NULL) { printf("Error0 delMessages\n"); return; }

    // scorro il file sorgente
    while(fgets(file_line, sizeof(file_line), srcFile) != NULL) {
        
        line++;

        // prelevo i dati dal file
        timestamp = strtok(file_line, " ");
        username_src = strtok(NULL, " ");
        username_dst = strtok(NULL, " ");

        username_src_len = (strlen(src_username) > strlen(username_src))? strlen(src_username):strlen(username_src);
        username_dst_len = (strlen(dst_username) > strlen(username_dst))? strlen(dst_username):strlen(username_dst);

        // controllo se ho trovato un messaggio da dst_username a src_username,
        // in caso affermativo mi salvo la riga da eliminare nel vettore
        if(!strncmp(src_username, username_src, username_src_len) && !strncmp(dst_username, username_dst, username_dst_len)) {
            lines[i] = line;
            i++;
        }
    }

    // riavvolgo il file
    rewind(srcFile);

    // elimino le righe trovate
    deleteLine(srcFile, tempFile, lines, num_messages);

    // chiudo i file
    fclose(srcFile);
    fclose(tempFile);

    // elimino srcFile e rinomino tempFile 
    remove("./server/files/db_messages.txt");
    rename("./server/files/delete-line.txt", "./server/files/db_messages.txt");
    return;
}

/**
 * Permette di eliminare dal file db_notifications.txt 
 * le notifiche ricevute da username.  
 * 
 * @param src_username puntatore al buffer che contiene l'username dell'utente.
 * @param num_notifications numero di notifiche da eliminare.
 */
void delNotificationsFromFile(char* src_username, int num_notifications) {

    FILE *srcFile;
    FILE *tempFile;
    int lines[num_notifications];
    int line = 0;
    int i = 0;
    
    int ret;
    char file_line[BUFFER_SIZE];
    char* username_src;
    int username_src_len;

    // inizializzo il vettore delle righe
    for(i = 0; i < num_notifications; i++)
        lines[i] = 0;
    i = 0;

    // apro i file uno in lettura e uno in scrittura
    srcFile = fopen("./server/files/db_notifications.txt", "r");
    tempFile = fopen("./server/files/delete-line.txt", "w");
    if(srcFile == NULL || tempFile == NULL) { printf("Error0 delMessages\n"); return; }

    while(fgets(file_line, sizeof(file_line), srcFile) != NULL) {
        
        line++;

        // prelevo l'username da file
        username_src = strtok(file_line, ":");

        username_src_len = (strlen(src_username) > strlen(username_src))? strlen(src_username):strlen(username_src);

        // controllo se ho trovato la notifica, in caso affermativo
        // mi salvo la riga da eliminare
        if(!strncmp(src_username, username_src, username_src_len)) {
            lines[i] = line;
            i++;
        }
    }

    // riavvolgo il file
    rewind(srcFile);

    // elimino le righe trovate
    deleteLine(srcFile, tempFile, lines, num_notifications);

    // chiudo i file
    fclose(srcFile);
    fclose(tempFile);

    // elimino srcFile e rinomino tempFile 
    remove("./server/files/db_notifications.txt");
    rename("./server/files/delete-line.txt", "./server/files/db_notifications.txt");
    return;
}

/**
 * Invia all'utente la lista degli online. Per fare ciò,
 * scorre il file db_users.txt e preleva gli utenti che 
 * hanno il timestamp_logout a NULL.
 * 
 * @param sd puntatore al socket descriptor associato all'utente.
 */
void sendOnlineUsers(int* sd) {
    
    int ret;
    FILE* fp;
    char file_line[64];
    char* username;
    char* password;
    char* port;
    char* timestamp_login;
    char* timestamp_logout;
    int num_online = 0;
    char* message;
    int len;

    // apro il file db_users.txt in lettura
    fp = fopen("./server/files/db_users.txt", "r"); 
    if(fp == NULL) { printf("Error0 chat\n"); return; }

    // se l'utente è online allora avrà campo NULL nel file
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        
        // prelevo i dati dell'utente
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");
        timestamp_logout = strtok(NULL, " ");

        // controllo se il timestamp del logout è NULL
        if(!strncmp(timestamp_logout, "NULL", 4)) {
            num_online++;
        }
    }

    // invio il numero di utenti online
    message = malloc(3);
    sprintf(message, "%d", num_online);
    ret = send_TCP(sd, message);
    if(ret < 0) { printf("Impossibile inviare il numero di utenti online\n"); free(message); return; }
    free(message);

    // se il numero degli online era zero termino
    if(num_online == 0) { fclose(fp); return; }

    // riavvolgo il file
    rewind(fp);

    // invio al client tutti gli utenti online 
    while (fgets(file_line, sizeof(file_line), fp) != NULL) {
        
        // prelevo i dati dell'utente
        username = strtok(file_line, " ");
        password = strtok(NULL, " ");
        port = strtok(NULL, " ");
        timestamp_login = strtok(NULL, " ");
        timestamp_logout = strtok(NULL, " ");

        // controllo se il timestamp del logout è NULL
        if(!strncmp(timestamp_logout, "NULL", 4)) {
            
            // creo il messaggio da inviare al client
            len = strlen(username) + strlen(port) + 2;
            message = malloc(len);
            snprintf(message, len, "%s %s", username, port);

            // invio il messaggio
            ret = send_TCP(sd, message);
            if(ret < 0) { printf("Impossibile inviare la notifica\n"); }
            
            // libero la memoria allocata per il messaggio
            free(message);
        }
    }
    return;
}

/**
 * Gestione della richiesta del device, a seconda del
 * comando ricevuto invoca la funzione corrispondente.
 * 
 * @param sd puntatore al socket descriptor associato all'utente che ha fatto la richiesta.
 * @param request puntatore al buffer che contiene la richiesta.
 * @param username puntatore al puntatore del buffer contentente l'username dell'utente.
 * @param pending_message_list puntatore alla lista dei messaggi pendenti.
 * @return -1 in caso di errore, 1 in caso di in, 0 in caso di chat online, 2 in caso di chat offline, 0 per gli altri comandi.
 */
int serveDeviceRequest(int* sd, char* request, char** username, struct pendingMessage** pending_message_list) {

    char* command = NULL;
    char* dev_username;
    char* dev_password;
    char* dev_port;
    int len;
    int ret;
    
    // prendo il comando inserito 
    command = strtok(request, " ");

    if(!strncmp(command, "in", 2)) {
        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");

        // salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);

        in(sd, dev_username, dev_password);
        
        return 1;
    } else if(!strncmp(command, "signup", 6)) {

        dev_username = strtok(NULL, " ");
        dev_password = strtok(NULL, " ");
        dev_port = strtok(NULL, " ");

        // salvo l'username dell'utente
        len = strlen(dev_username);
        *username = malloc(len + 1);
        strncpy(*username, dev_username, len);
        
        signup(sd, dev_username, dev_password, dev_port);

        return 1;
    } else if(!strncmp(command, "hanging", 7)) {
        hanging(sd, pending_message_list);
    } else if(!strncmp(command, "show", 4)) {
        dev_username = strtok(NULL, " ");
        
        show(sd, pending_message_list, dev_username, *username);
    } else if(!strncmp(command, "chat", 4)) {
        dev_username = strtok(NULL, " ");
        
        ret = chat(sd, dev_username);
        
        return ret;
    } else if(!strncmp(command, "\\u", 2)) {
        // invio la lista degli utenti online
        sendOnlineUsers(sd);
    } else { return -1; }
    return 0;
}