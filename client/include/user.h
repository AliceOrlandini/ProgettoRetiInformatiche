#include <arpa/inet.h>

enum User_State { DISCONNECTED, CONNECTED, LOGGED };
struct User {
    enum User_State user_state;
    char* my_username;
    char* my_password;
    in_port_t my_port; 
    int srv_port;
    char* dst_username;
}; 