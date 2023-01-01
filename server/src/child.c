#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include "./../include/child.h"

/**
 * Permette di aggiungere un figlio alla lista dei processi figli.
 * Viene invocata subito dopo la fork nel processo padre.
 * 
 * @param child_list la lista dei processi figli.
 * @param pid intero che rappresenta il process id del figlio.
*/
void addChild(struct child** child_list, int pid) {
    
    struct child* new_child;
    
    // creo il nuovo figlio
    new_child = malloc(sizeof(struct child));

    // inizializzo i dati del nuovo figlio
    new_child->pid = pid;

    // inserisco il nuovo figlio in testa
    new_child->next = *child_list;
    *child_list = new_child;

    return;
}

/**
 * Elimina tutta la lista dei processi figli.
 * 
 * @param child_list lista dei processi figli da eliminare.
 */
void delChildList(struct child** child_list) {
    
    struct child *del_child;
    
    // se la lista dei figli è vuota non faccio nulla
    if(*child_list == NULL) {
        return;
    }

    // scorro la lista ed elimino ogni figlio
    while(*child_list != NULL) {
        del_child = (*child_list)->next;

        // elimino il figlio
        *child_list = del_child;
    }

    *child_list = NULL;
}

/**
 * Funzione che elimina un figlio dalla lista
 * 
 * @param child_list la lista dei processi figli.
 * @param pid il processo id del figlio da eliminare.
 */
void delChild(struct child** child_list, int pid) {
    
    struct child* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*child_list == NULL) {
        return;
    }

    // controllo se ho trovato il processo
    if((*child_list)->pid == pid) {
        elem = *child_list;
        *child_list = (*child_list)->next;

        // chiamo la funzione in modo ricorsivo
        delChild(child_list, pid);
    } else {
        delChild(&(*child_list)->next, pid);
    }
}

/**
 * Permette di stampare il contenuto della lista dei 
 * processi figli. Utilizzata solo in fase di debug.
 * 
 * @param child_list lista dei processi figli.
 */
void printChildList(struct child** child_list) {
    
    struct child* elem = NULL;

    // se la lista è vuota non faccio nulla
    if(*child_list == NULL) {
        printf("\nLa lista è vuota.\n");
        return;
    }

    elem = *child_list;
    while(elem != NULL) {
        printf("\npid: %d\n", elem->pid);
        elem = elem->next;
    }
    printf("Fine lista.\n");
    return;
}

/**
 * Permette di killare tutti i processi presenti nella lista.
 * Prima di eseguire la kill verifica se il pid è ancora valido
 * (ovvero se il processo figlio esiste ancora)
 * 
 * @param child_list lista dei processi figli.
 * @return un numero negativo in caso di errore, zero altrimenti. 
 */
int killChildren(struct child** child_list) {
    
    struct child* elem = NULL;
    int ret;
    char command;

    // se la lista è vuota non faccio nulla
    if(*child_list == NULL) { return 0; }

    elem = *child_list;

    // nel caso esistano processi figli avverto l'utente
    if(elem != NULL) {
        printf("Attenzione: potrebbero essere presenti connessioni con device, chiudere comunque il server? [Y/n]: ");
        fflush(stdout);
        // prelievo il comando dallo standard input 
        scanf("%c", &command);
        
        // controllo che il comando sia corretto
        while(command != 'Y' && command != 'n') {
            printf("Comando non valido, inserire [Y/n]: ");
            // prelievo il comando dallo standard input 
            scanf("%c", &command);
        }
        if(command == 'n') {
            return -1;
        } 
    }

    // scorro la lista dei figli e killo tutti i processi presenti
    while(elem != NULL) {
        // killo quel processo
        ret = kill(elem->pid, SIGTERM);
        if(ret == -1) { perror("Error1 kill"); }
        elem = elem->next;
    }
    printf("Figli terminati con successo.\n");
    return 0;
}