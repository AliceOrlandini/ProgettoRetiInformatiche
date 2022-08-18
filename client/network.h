void connect_to_server(int*, struct sockaddr_in*);
int send_TCP(int*, char*);
void send_UDP();
void receive_TCP(int*, char*);
void receive_UDP();
void server_disconnect(int*);