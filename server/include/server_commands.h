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