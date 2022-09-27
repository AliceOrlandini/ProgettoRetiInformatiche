#include "./pending_messages.h"

void in(int*, char*, char*);
void signup(int*, char*, char*, char*);
void hanging(int*, struct pendingMessage**);
void show(int*, struct pendingMessage**, char*, char*);
void share();
void chat(int*, char*);
void out(char*);
void saveMessage(char*);
void delMessages(char*, char*, int);
int serveDeviceRequest(int*, char*, char**, struct pendingMessage**);