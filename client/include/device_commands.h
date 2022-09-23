#include <arpa/inet.h>
#include "user.h"

void printCommands(struct User);
int signup(char*, struct User*, int*, struct sockaddr_in*);
int in(char*, struct User*, int*, struct sockaddr_in*);
void hanging(char*, int*);
void show(char*, int*, char*);
void chat(char*, int*, char*, char*);
void share(char*, int*, char*);
void out(int*, struct User*);
int executeDeviceCommand(char*, struct User*, int*, struct sockaddr_in*);