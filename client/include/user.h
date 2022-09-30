enum User_State { DISCONNECTED, LOGGED, CHATTING_ONLINE, CHATTING_OFFLINE };

struct usersChattingWith {
    char* dst_username;
    struct sockaddr_in addr;
    in_port_t port;
    int p2p_sd;
    struct usersChattingWith* next;
};

struct User {
    enum User_State user_state;
    char* my_username;
    char* my_password;
    char* my_port;
    char* srv_port;
    char* dst_username;
    struct usersChattingWith* users_chatting_with;
}; 
