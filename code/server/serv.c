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
Message *head = NULL;
void sendMessageFileContentToClient(const char *fileName, int client_socket) {
    FILE *file = fopen(fileName, "rb");
    if (file == NULL) {
        return;  
    }

     
    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytesRead, 0) != bytesRead) {
            perror("Send failed");
            fclose(file);
            return;
        }
    }

     
    fclose(file);
}


void writeMessageListToFile(const char *senderName, const char *receiverName) {
    char senderFileName[100];  
    char receiverFileName[100];  
    sprintf(senderFileName, "%s_messages.bin", senderName);  
    sprintf(receiverFileName, "%s_messages.bin", receiverName);  

    FILE *senderFile = fopen(senderFileName, "ab");
    FILE *receiverFile = fopen(receiverFileName, "ab");
    if (senderFile == NULL || receiverFile == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    Message *current = head;
    while (current != NULL) {
        
        if (strcmp(current->sender, senderName) == 0 || strcmp(current->recipient, senderName) == 0) {
            fwrite(current, sizeof(Message), 1, senderFile);
        }
         
        if (strcmp(current->sender, receiverName) == 0 || strcmp(current->recipient, receiverName) == 0) {
            fwrite(current, sizeof(Message), 1, receiverFile);
        }
        current = current->next;
    }
    fclose(senderFile);
    fclose(receiverFile);
}


void emptyMessageFile(const char *senderName) {
    char fileName[100];  
    sprintf(fileName, "%s_messages.bin", senderName);  
    FILE *file = fopen(fileName, "wb");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fclose(file);
}

void addMessage(const char *sender, const char *recipient, const char *text) {
    Message *newMessage = (Message *)malloc(sizeof(Message));
    if (newMessage == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(newMessage->sender, sender);
    strcpy(newMessage->recipient, recipient);
    strcpy(newMessage->text, text);
    newMessage->next = head;
    head = newMessage;
     
    writeMessageListToFile(sender , recipient);
}




void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    User received_user;
    LoginData login_data;
    User rsearch;
    User user;

    int flag;
    char buffer[BUFFER_SIZE];
    char username[100];
    char userdel[100];

     
    if (recv(client_socket, &flag, sizeof(int), 0) < 0) {
        perror("Receive failed");
        close(client_socket);
        pthread_exit(NULL);
    }
    switch (flag) {
            case 1: 
                     
                     
                    if (recv(client_socket, &received_user, sizeof(User), 0) < 0) {
                        perror("Receive failed");
                        close(client_socket);
                        pthread_exit(NULL);
                    }

                     
                    FILE *file = fopen("users.data", "rb");
                    if (file == NULL) {
                        perror("Error opening file");
                        close(client_socket);
                        pthread_exit(NULL);
                    }

                    User existing_user;
                    int username_taken = 0;
                    while (fread(&existing_user, sizeof(User), 1, file) == 1) {
                        if (strcmp(existing_user.username, received_user.username) == 0) {
                             
                            username_taken = 1;
                            break;
                        }
                    }
                    fclose(file);

                    if (username_taken) {
                         
                        const char *message = "Username already taken. Please choose another username.";
                        if (send(client_socket, message, strlen(message), 0) < 0) {
                            perror("Send failed");
                        }
                    } else {
                         
                        FILE *file = fopen("users.data", "ab");  
                        if (file == NULL) {
                            perror("Error opening file");
                            close(client_socket);
                            pthread_exit(NULL);
                        }
                        fwrite(&received_user, sizeof(User), 1, file);
                        fclose(file);

                        printf("new User '%s' saved .\n" , received_user.username);

                         
                        const char *message = "User registered successfully.";
                        if (send(client_socket, message, strlen(message), 0) < 0) {
                            perror("Send failed");
                        }
                    }

                    close(client_socket);
                    pthread_exit(NULL);
                break;
            case 2: 


                if (recv(client_socket, &login_data, sizeof(LoginData), 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                    pthread_exit(NULL);
                }

                 
                

                FILE *file_read = fopen("users.data", "rb");
                if (file_read == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }


                
                int login_successful = 0;  
                while (fread(&user, sizeof(User), 1, file_read) == 1) {
                    if (strcmp(user.username, login_data.username) == 0 &&
                        strcmp(user.password, login_data.password) == 0) {
                         
                        login_successful = 1;
                        break;
                    }
                }
                fclose(file_read);

                if (login_successful) {
                        printf("Login successful. Username '%s' has logged in.\n", login_data.username);
                } else {
                        printf("Login failed: Username or password incorrect for user '%s'.\n",login_data.username);
                }

                 
                if (send(client_socket, &login_successful, sizeof(int), 0) < 0) {
                    printf("Send failed");

                }

                close(client_socket);
                pthread_exit(NULL);
                break;
            



            case 3: 


                if (recv(client_socket, &login_data, sizeof(LoginData), 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                    pthread_exit(NULL);
                }


                 

                if (recv(client_socket, &received_user, sizeof(User), 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                    pthread_exit(NULL);
                }

                 
                FILE *file_s = fopen("users.data", "rb+");
                if (file_s == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }

                int f = 0;  
                long int position = -1;  

                while (fread(&user, sizeof(User), 1, file_s) == 1) {
                     
                    if (strcmp(user.username, login_data.username) == 0 &&
                        strcmp(user.password, login_data.password) == 0) {
                        f = 1;
                        break;
                    }
                     
                    position = ftell(file_s);
                }
                int successful = 0;
                 
                if (f) {
                     
                    fseek(file_s, position, SEEK_SET);
                     
                    FILE *file_ver = fopen("users.data", "rb+");
                    if (file_s == NULL) {
                        perror("Error opening file");
                        exit(EXIT_FAILURE);
                    }

                    int v = 0; 
                    

                    while (fread(&user, sizeof(User), 1, file_ver) == 1) {
                        
                        if (strcmp(user.username, received_user.username) == 0) {
                            
                            v = 1;
                            break;
                        }
                        
                    }

                    fclose(file_ver);

                    if (v==0)
                    {
                        if (fwrite(&received_user, sizeof(User), 1, file_s) != 1) {
                        perror("Error writing to file");
                        
                        } else {
                            printf("User data modified successfully.\n");
                            successful = 1;
                        }
                    }else{
                        successful = 2;
                    }
                    
                } else {
                    printf("User data to modifiy not found.\n");
                }

                fclose(file_s);

                if (send(client_socket, &successful, sizeof(int), 0) < 0) {
                    printf("Send failed");

                }

                close(client_socket);
                pthread_exit(NULL);





                break;
            case 4:
                    
                    if (recv(client_socket, &rsearch, sizeof(User), 0) < 0) {
                        perror("Receive failed");
                        close(client_socket);
                        pthread_exit(NULL);
                    }


                     
                    FILE *file_se = fopen("users.data", "rb+");
                    if (file_se == NULL) {
                        perror("Error opening file");
                        exit(EXIT_FAILURE);
                    }
                     

                    long int current_position;  

                     
                    int found = 0;
                    while (fread(&user, sizeof(User), 1, file_se) == 1) {
                         
                        if (strcmp(user.username, rsearch.username) == 0 &&
                            strcmp(user.password, rsearch.password) == 0) {
                            
                            found = 1;
                            break;
                        }
                         
                        current_position = ftell(file_se);
                    }
                    int del_successful = 0;
                    if (found) {
                             
                            fseek(file_se, current_position, SEEK_SET);
                             
                            fseek(file_se, -sizeof(User), SEEK_END);
                            fread(&user, sizeof(User), 1, file_se);
                            fseek(file_se, current_position, SEEK_SET);
                            fwrite(&user, sizeof(User), 1, file_se);
                             
                            fseek(file_se, 0, SEEK_END);
                            long int new_size = ftell(file_se) - sizeof(User);
                            ftruncate(fileno(file_se), new_size);
                            printf("User data deleted successfully.\n");
                            del_successful = 1;
                    } else {
                            printf("User data not found.\n");
                    }




                 
                if (send(client_socket, &del_successful, sizeof(int), 0) < 0) {
                    printf("Send failed");

                }

                 
                fclose(file_se);
                close(client_socket);
                pthread_exit(NULL);
                break;

            case 5:


                if (recv(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                }
                char sender[40], recipient[40], text[400];
                sscanf(buffer, "%s %s %[^\n]", sender, recipient, text);
                



                FILE *file_ser = fopen("users.data", "rb");
                if (file_ser == NULL) {
                    perror("Error opening file");
                    exit(EXIT_FAILURE);
                }


                
                int userin = 0;  
                while (fread(&user, sizeof(User), 1, file_ser) == 1) {
                    if (strcmp(user.username, recipient) == 0){
                         
                        userin = 1;
                        break;
                    }
                }
                fclose(file_ser);


                if (userin)
                {
                     
                    printf("========================\n");
                    printf("%s send msg to %s \n", sender , recipient);
                    printf("========================\n");
                    
                     
                    addMessage(sender, recipient, text);

                     
                    const char *ack = "Message received and stored";
                    send(client_socket, ack, strlen(ack), 0);
                }
                else{
                    const char *ack = "user dest not found";
                    send(client_socket, ack, strlen(ack), 0);
                }

                
                
                





                close(client_socket);
                pthread_exit(NULL);
               break;
            case 6:
                if (recv(client_socket, username, sizeof(username), 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                    pthread_exit(NULL);
                }
                sprintf(username, "%s_messages.bin", username);
                sendMessageFileContentToClient(username, client_socket);
                close(client_socket);
                pthread_exit(NULL);
                break;
            case 7:

                if (recv(client_socket, userdel, sizeof(userdel), 0) < 0) {
                    perror("Receive failed");
                    close(client_socket);
                    pthread_exit(NULL);
                }
                

                emptyMessageFile(userdel);
                close(client_socket);
                pthread_exit(NULL);
                break;
            default:
                printf("Invalid flag\n");
                close(client_socket);
                exit(EXIT_FAILURE);
        }

}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

     
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

     
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

     
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

     
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

     
    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len)) < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Connection accepted\n");

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Thread creation failed");
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}