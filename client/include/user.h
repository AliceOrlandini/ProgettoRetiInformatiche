/**
 * Enumerato per gli stati dell'utente.
 */
enum User_State { DISCONNECTED, LOGGED, CHATTING_ONLINE, CHATTING_OFFLINE };

/**
 * Struttura dati che contiene le informazioni associate all'utente.
 */
struct User {
    enum User_State user_state;
    char* my_username;
    char* my_password;
    char* my_port;
    char* srv_port;
    char* dst_username;
    struct usersChattingWith* users_chatting_with;
}; 
