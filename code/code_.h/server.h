#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>

#define PORT 12345
#define BUFFER_SIZE 1024

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
    char sender[40];
    char recipient[40];
    char text[400];
    struct Message *next;
} Message;

void sendMessageFileContentToClient(const char *fileName, int client_socket);
void writeMessageListToFile(const char *senderName, const char *receiverName);
void emptyMessageFile(const char *senderName);
void addMessage(const char *sender, const char *recipient, const char *text);
void *handle_client(void *arg);

#endif 