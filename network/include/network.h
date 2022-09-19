#include <arpa/inet.h>

int init_server(int*, struct sockaddr_in*, in_port_t);
int connect_to_server(int*, struct sockaddr_in*, in_port_t);
int send_TCP(int*, char*);
int send_UDP();
int receive_TCP(int*, char*);
int receive_UDP();
int disconnect_to_server(int*);