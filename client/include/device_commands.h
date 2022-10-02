#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include "user.h"

struct onlineUser {
    char* username;
    char* port;
    struct onlineUser* next; 
};


void printCommands(struct User);
int signup(char*, struct User*, int*, struct sockaddr_in*);
int in(char*, struct User*, int*, struct sockaddr_in*);
void hanging(char*, int*);
void show(char*, int*, char*);
bool checkContacts(char*, char*);
int chat(char*, int*, char*, char*);
void out(int*, struct User*);
void receiveNotifications(int*, char*);
int executeDeviceCommand(char*, struct User*, int*, struct sockaddr_in*);

void addElemToOnlineUserList(struct onlineUser**, char*, char*);
void delOnlineUserList(struct onlineUser**);
void printOnlineUserList(struct onlineUser**);
void delUserFromOnlineUserList(struct onlineUser**, char*);
char* getPortFromOnlineUserList(struct onlineUser**, char*);

void addElemToChattingWithList(struct usersChattingWith**, char*, in_port_t, int);
void delChattingWithList(struct usersChattingWith**);
void printChattingWithList(struct usersChattingWith**);
void delUserFromChattingWithList(struct usersChattingWith**, int);
int sendMessageToAll(struct usersChattingWith**, char*);
void addNewConnToChattingWithList(struct usersChattingWith**, char*, in_port_t, int*);
void delAllConnFromChattingWithList(struct usersChattingWith**, fd_set*);
void delConnFromChattingWithList(struct usersChattingWith**, int*, fd_set*);