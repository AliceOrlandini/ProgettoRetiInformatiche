#include "user.h"

void printCommands(struct User);
void signup(char*, char*, char*, int*, struct sockaddr_in*);
void in(char*, int, char*, char*, int*, struct sockaddr_in*);
void hanging();
void show(char*);
void chat(char*);
void share(char*);
void out();
int executeDeviceCommand(char*, struct User*, int*, struct sockaddr_in*);