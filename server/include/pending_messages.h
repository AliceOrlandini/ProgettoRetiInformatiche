/**
* Struttura per creare la lista dei messaggi pendenti.
*/
struct pendingMessage {
    char* username_src;
    char* timestamp;
    char* message;
    struct pendingMessage* next; 
};

void createPMList(struct pendingMessage**, char*);
void delPMList(struct pendingMessage**);
void printPMList(struct pendingMessage**);
void addElemToPMList(struct pendingMessage**, char*, char*, char*);
void delMessagesFromPMList(struct pendingMessage**, char*);