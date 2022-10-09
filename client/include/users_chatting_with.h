#include <sys/types.h>
#include <arpa/inet.h>

struct usersChattingWith {
    char* dst_username;
    struct sockaddr_in addr;
    in_port_t port;
    int p2p_sd;
    struct usersChattingWith* next;
};

void addElemToChattingWithList(struct usersChattingWith**, char*, in_port_t, int);
void delChattingWithList(struct usersChattingWith**);
void printChattingWithList(struct usersChattingWith**);
void delUserFromChattingWithList(struct usersChattingWith**, int);
int sendMessageToAll(struct usersChattingWith**, char*);
void addNewConnToChattingWithList(struct usersChattingWith**, char*, in_port_t, int*);
void delAllConnFromChattingWithList(struct usersChattingWith**, fd_set*);
void delConnFromChattingWithList(struct usersChattingWith**, int*, fd_set*);
bool isInTheGroupYet(struct usersChattingWith**, char*);