#ifndef CHAT_APP_H
#define CHAT_APP_H

#define PORT 12345
#define BUFFER_SIZE 1024 
#define SHA256_DIGEST_LENGTH 32
#define MAX_SENDER_LEN 40
#define MAX_RECIPIENT_LEN 40
#define MAX_TEXT_LEN 400

typedef struct {
    char username[40];
    char password[100];
    char birthdate[100];
    char phone[11];
} User;

typedef struct {
    char username[40];
    char password[100];
} LoginData;

typedef struct Message {
    char sender[MAX_SENDER_LEN];
    char recipient[MAX_RECIPIENT_LEN];
    char text[MAX_TEXT_LEN];
    struct Message *next;
} Message;

void disable_echo();
void enable_echo();
void receiveFileContentFromServer(int client_socket, const char *fileName);
void printMessageList(Message *head);
void readMessageListFromFile();
void remove_newline(char *str);
int isValidDate(const char *birthdate);
int isValidPhoneNumber(const char *phoneNumber);
int is_valid_username(const char *username);
int is_valid_password(const char *password);
void option1();
void option5(const char *sender);
void option6(const char *sender);
void option7(const char *sender);
void option2();
void option3();
void option4();

#endif 
