#include <arpa/inet.h>

#define BACKLOG 10

int init_listener(int*, struct sockaddr_in*, in_port_t);
int connect_to(int*, struct sockaddr_in*, in_port_t);
int send_TCP(int*, char*);
int send_UDP(int*, in_port_t, char*);
int receive_TCP(int*, char*);
int receive_UDP(int*, in_port_t, struct sockaddr_in*, char*);
int disconnect_to(int*);