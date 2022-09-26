enum User_State { DISCONNECTED, LOGGED, CHATTING_ONLINE, CHATTING_OFFLINE };
struct User {
    enum User_State user_state;
    char* my_username;
    char* my_password;
    char* my_port;
    char* srv_port;
    char* dst_username;
}; 