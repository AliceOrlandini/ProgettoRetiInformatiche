struct onlineUser {
    char* username;
    char* port;
    struct onlineUser* next; 
};

void printCommands();
void help();
void list();
void esc(int*);
void executeServerCommand(char*, int*);

void addElemToOnlineUserList(struct onlineUser**, char*, char*);
void delOnlineUserList(struct onlineUser**);
void printOnlineUserList(struct onlineUser**);
void delUserFromOnlineUserList(struct onlineUser**, char*);