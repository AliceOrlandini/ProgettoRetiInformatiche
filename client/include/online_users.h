/**
 * Struttura con cui verrà realizzata
 * la lista degli utenti online.
 */
struct onlineUser {
    char* username;
    char* port;
    struct onlineUser* next; 
};

void addElemToOnlineUserList(struct onlineUser**, char*, char*);
void delOnlineUserList(struct onlineUser**);
void printOnlineUserList(struct onlineUser**);
void delUserFromOnlineUserList(struct onlineUser**, char*);
char* getPortFromOnlineUserList(struct onlineUser**, char*);