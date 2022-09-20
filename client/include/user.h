enum User_State { DISCONNECTED, CONNECTED, LOGGED };
struct User {
    enum User_State user_state;
    char* my_username;
    char* my_password;
    char* my_port;
    char* dst_username;
}; 