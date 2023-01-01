#include <arpa/inet.h>
#include <stdbool.h>
#include <sys/types.h>
#include "user.h"

void printCommands(struct User);
int signup(char*, struct User*, int*, struct sockaddr_in*);
int in(char*, struct User*, int*, struct sockaddr_in*);
void hanging(char*, int*);
void show(char*, int*, char*);
bool checkContacts(char*, char*);
int chat(char*, int*, char*, char*);
void out(int*, struct User*);
void updateSavedMessages(struct User*, char*);
void receiveNotifications(struct User*, int*, char*);
int executeDeviceCommand(char*, struct User*, int*, struct sockaddr_in*);