/* 
    Struttura per i dati dei processi figli.
*/
struct child {
    int pid;
    struct child* next;
};

void addChild(struct child**, int);
void delChildList(struct child**);
void delChild(struct child**, int);
void printChildList(struct child**);
int killChildren(struct child**);