#include "./pending_messages.h"

void in(int*, char*, char*);
void signup(int*, char*, char*, char*);
void hanging(int*, struct pendingMessage**);
void show(int*, struct pendingMessage**, char*, char*);
int chat(int*, char*);
void out(char*);
void saveMessage(char*);
void delMessagesFromFile(char*, char*, int);
void delNotificationsFromFile(char*, int);
void sendNotifications(int*, char*);
void sendOnlineUsers(int*);
int serveDeviceRequest(int*, char*, char**, struct pendingMessage**);