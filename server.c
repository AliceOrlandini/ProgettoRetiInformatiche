#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <unistd.h>

/*#include <sys/select.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>*/

#define SERVER_COMMAND_SIZE 5
#define LOCALHOST "127.0.0.1"
#define SERVER_PORT 4242
#define BACKLOG 10
#define STANDARD_INPUT 0
#define BUFFER_SIZE 1024

/* stampa i comandi che il server ha a disposizione */
void printCommands() {
    
    printf("comandi disponibili:\n1) help --> mostra i dettagli dei comandi\n2) list --> mostra un elenco degli utenti connessi\n3) esc  --> chiude il server\n");
    return;
}

/* stampa le info dei comandi disponibili */
void help() {
    printf("\nEcco le info dei comandi:\n-list: mostra l'elenco degli utenti connessi nel formato \"username*timestamp*porta\".\n-esc: termina il server.\n\n");
    return;
}

void list() {
    
}

void esc() {
    
}

/* verifica che il comando sia valido ed esecuzione della funzione corrispondente */
void executeServerCommand(char* buffer) {

    char server_command[SERVER_COMMAND_SIZE];
    sscanf(buffer, "%s", server_command);

    // Controllo che il comando sia valido 
    if(strcmp("help", server_command) && strcmp("list", server_command) && strcmp("esc", server_command)) {
        printf("\nComando non valido, ecco i ");
        // mostro di nuovo i comandi disponibili
        printCommands();
        return;
    }

    // Eseguo la funzione corrispondente al comando
    if(!strcmp("help", server_command)) {
        help();
        printCommands();
    }
    else if(!strcmp("list", server_command)) {
        list();
        printCommands();
    }
    else
        esc();
    return;
}

/* gestione dei descrittori pronti tramite l'io multiplexing */
void ioMultiplexing(int listener, char* buffer) {
    
    int new_sd;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(struct sockaddr_in);
    pid_t pid;
    fd_set master;
    fd_set read_fds;
    int fdmax;
    int i;
    int ret;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(STANDARD_INPUT, &master);
    FD_SET(listener, &master);
    fdmax = listener;

    for(;;) {
        read_fds = master;
        select(fdmax + 1, &read_fds, NULL, NULL, NULL);
        for(i = 0; i <= fdmax; i++) {
            if(FD_ISSET(i, &read_fds)) {
                if(i == listener) {
                    printf("Accetto nuove richieste di connessione\n");
                    new_sd = accept(listener, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
                    if(new_sd < 0) {
                        perror("Error4");
                        // exit(0); // togliere
                    } else {
                        printf("Stabilita una connessione\n");
                        
                        FD_SET(new_sd, &master);
                        fdmax = (new_sd > fdmax)? new_sd:fdmax;

                        // uso la fork per una gestione concorrente delle richieste
                        pid = fork(); 
                        if(pid < 0) {
                            perror("Error5");
                            exit(0);
                        } else if(pid == 0) {
                            // siamo nel figlio quindi chiudo il socket di ascolto
                            close(listener);
                            // elaboro la richiesta 
                            memset(&buffer, '\0', sizeof(buffer));
                            int len = 1;
                            ret = recv(new_sd, (void*)&buffer, len, 0);
                            if(ret < len) {
                                perror("Error6");
                                break;
                            }
                            printf("Richiesta ricevuta da un client %s\n", buffer);
                        
                            
                            // chiudo il socket di comunicazione e termino il figlio
                            // close(new_sd);
                            // exit(0);
                        } else {
                            // siamo nel padre quindi chiudo il socket di comunicazione
                            close(new_sd);
                        }
                        
                    }
                } else if(i == STANDARD_INPUT){
                    // prelievo il comando dallo standard input e lo salvo nel buffer
                    read(STANDARD_INPUT, (void*)&buffer, SERVER_COMMAND_SIZE);
                    // eseguo l'azione prevista dal comando
                    executeServerCommand((char*)&buffer);
                }
            }
        }
    }
}

/*void initServer(int listener, struct sockaddr_in *server_addr) {

    int ret;
    int addrlen = sizeof(struct sockaddr_in);
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("Error");
        exit(0);
    }
    
    memset(server_addr, 0, addrlen);
    server_addr->sin_family = AF_INET;
    server_addr->sin_port = htons(4242);
    inet_pton(AF_INET, LOCALHOST, (void*)server_addr->sin_addr);

    ret = bind(listener, (struct sockaddr*)&server_addr, addrlen);
    if(ret < 0) {
        perror("Error");
        exit(0);
    }
    
    ret = listen(listener, BACKLOG);
    if(ret < 0) {
        perror("Error");
        exit(0);
    }
}*/

int main(int argc, char *argv[]) {

    // variabili di utilita'
    int listener = NULL;
    struct sockaddr_in server_addr;
    int ret;
    char buffer[BUFFER_SIZE];

    printf("***** SERVER STARTED *****\n");
    printCommands();
    
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0) {
        perror("Error1");
        exit(0);
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(4242);
    inet_pton(AF_INET, LOCALHOST, &server_addr.sin_addr);

    ret = bind(listener, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(ret < 0) {
        perror("Error2");
        exit(0);
    }
    
    ret = listen(listener, BACKLOG);
    if(ret < 0) {
        perror("Error3");
        exit(0);
    }

    ioMultiplexing(listener, (char*)&buffer);
    
    return  0;
}