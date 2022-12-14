#include <arpa/inet.h>

#define BACKLOG 10

int init_listener(int*, struct sockaddr_in*, in_port_t);
int connect_to(int*, struct sockaddr_in*, in_port_t);
int send_TCP(int*, char*);
int send_file(int*, FILE*);
int receive_file(int*, FILE*);
int receive_TCP(int*, char*);
int disconnect_to(int*);