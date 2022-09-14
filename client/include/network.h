int connect_to_server(int*, struct sockaddr_in*, in_port_t);
int send_TCP(int*, char*);
void send_UDP();
void receive_TCP(int*, char*);
void receive_UDP();
int disconnect_to_server(int*);