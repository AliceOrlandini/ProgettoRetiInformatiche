#include "user.h"

void printCommands(struct User);
void signup(char*, char*, char*, int*);
void in(char*, int, char*, char*, int*);
void hanging();
void show(char*);
void chat(char*);
void share(char*);
void out();
int executeDeviceCommand(char*, struct User*, int*);