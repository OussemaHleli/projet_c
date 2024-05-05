#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 12345
#define BUFFER_SIZE 1024 
#define SHA256_DIGEST_LENGTH 32
#define MAX_SENDER_LEN 40
#define MAX_RECIPIENT_LEN 40
#define MAX_TEXT_LEN 400


unsigned char hash[SHA256_DIGEST_LENGTH];
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

void disable_echo() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

// Function to turn on terminal echo
void enable_echo() {
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}
void receiveFileContentFromServer(int client_socket, const char *fileName) {
    FILE *file = fopen(fileName, "wb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    int messageReceived = 0;  

    while ((bytesRead = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, bytesRead, file);
        messageReceived = 1;  
    }

    if (messageReceived == 0) {
        printf("No message received for now.\n");
    }

    if (bytesRead < 0) {
        perror("Error receiving file content");
    }

    fclose(file);
}

void printMessageList(Message *head) {
    Message *current = head;
    while (current != NULL) {
        printf("Sender: %s\nRecipient: %s\nText: %s\n\n", current->sender, current->recipient, current->text);
        current = current->next;
    }
}


void readMessageListFromFile() {
    FILE *file = fopen("messages.bin", "rb");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }
    Message *head = NULL;
    Message *current = NULL;
    while (1) {
        Message *newMessage = (Message *)malloc(sizeof(Message));
        if (newMessage == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        size_t read_size = fread(newMessage, sizeof(Message), 1, file);
        if (read_size != 1) {
            free(newMessage);
            break; 
        }
        newMessage->next = NULL;
        if (head == NULL) {
            head = newMessage;
            current = head;
        } else {
            current->next = newMessage;
            current = newMessage;
        }
    }
    fclose(file);
    printMessageList(head); 
}








void remove_newline(char *str) {
    char *newline = strchr(str, '\n'); 
    if (newline != NULL) {
        *newline = '\0'; 
    }
}
int isValidDate(const char *birthdate) {
     
    if (strlen(birthdate) != 10) {
        return 0;   
    }

    if (birthdate[4] != '-' || birthdate[7] != '-') {
        return 0;   
    }

    for (int i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit(birthdate[i])) {
            return 0;   
        }
    }

     
    int year = atoi(birthdate);
    int month = atoi(birthdate + 5);
    int day = atoi(birthdate + 8);

     
    if (year < 1900 || year > 2100) {
        return 0;   
    }

    if (month < 1 || month > 12) {
        return 0;   
    }

    if (day < 1 || day > 31) {
        return 0;   
    }

     
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        return 0;   
    }

    if (month == 2) {
        if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
            if (day > 29) {
                return 0;   
            }
        } else {
            if (day > 28) {
                return 0;   
            }
        }
    }

    return 1;   
}

int isValidPhoneNumber(const char *phoneNumber) {
    int length = strlen(phoneNumber);

     
    if (length < 8 || length > 15) {
        return 0;   
    }

     
    if (phoneNumber[0] == '+') {
         
        for (int i = 1; i < length; i++) {
            if (!isdigit(phoneNumber[i])) {
                return 0;   
            }
        }
        return 1;   
    }

     
    for (int i = 0; i < length; i++) {
        if (!isdigit(phoneNumber[i])) {
            return 0;   
        }
    }
    return 1;   
}


int is_valid_username(const char *username) {
    int len = strlen(username);
    if (len < 3 || len > 15) {
        return 0;
    }
    for (int i = 0; i < len; i++) {
        if (!isalnum(username[i])) {
            return 0;
        }
    }
    return 1;
}

int is_valid_password(const char *password) {
    int len = strlen(password);
    int has_upper = 0;
    int has_digit = 0;
    int has_exclamation = 0;
    for (int i = 0; i < len; i++) {
        if (isupper(password[i])) {
            has_upper = 1;
        } else if (isdigit(password[i])) {
            has_digit = 1;
        } else if (password[i] == '!'||password[i] == '?'||password[i] == '$'||password[i] == '#') {
            has_exclamation = 1;
        }
    }
    return len >= 6 && len <= 15 && has_upper && has_digit && has_exclamation;
}

void option1() {

    char message[] = "Creation de Comptes...";
    int len = strlen(message);
    User user;

    printf("\033[2J\033[1;1H");
    for (int x = 0; x < len; x++) {
            printf("\x1b[33m");
            printf("\r[*] %.*s%c%s", x, message, toupper(message[x]), message + x + 1);
            fflush(stdout);
            usleep(100000);
    }




    printf("\033[2J\033[1;1H");
    printf("\n\x1b[36m");
    printf("==========================================\n");
    printf(" ______     __  __     ______     ______  \n");
    printf("/\\  ___\\   /\\ \\_\\ \\   /\\  __ \\   /\\__  _\\ \n");
    printf("\\ \\ \\____  \\ \\  __ \\  \\ \\  __ \\  \\/_/\\ \\/ \n");
    printf(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_\\ \\_\\    \\ \\_\\ \n");
    printf("  \\/_____/   \\/_/\\/_/   \\/_/\\/_/     \\/_/ \n");

        printf("==========================================\n");
        printf("\n");

    char username[40];
    char password[100];
    char confirm_password[100];
    do {
            printf("Nom d'utilisateur  : ");
            scanf("%s", user.username);
            if (!is_valid_username(user.username)) {
                printf("Invalid username. Please enter a valid username.\n");
            }
    } while (!is_valid_username(user.username));

        do {
            printf("Mot de passe  : ");
            scanf("%s", user.password);
            printf("Confirmer le mot de passe : ");
            scanf("%s", confirm_password);

            if (strcmp(user.password, confirm_password) != 0) {
                printf("Les mots de passe ne correspondent pas.\n");
                continue;
            }

            if (!is_valid_password(user.password)) {
                printf("Invalid password. Please enter a valid password.\n");
                continue;
            }

            break;
        } while (1);


        int validBirthdate = 0;
        do {
            printf("Enter your birthdate (YYYY-MM-DD): ");
            scanf("%s", user.birthdate);
            if (!isValidDate(user.birthdate)) {
                printf("Invalid birthdate format. Please enter in YYYY-MM-DD format.\n");
            } else {
                validBirthdate = 1;
            }
        } while (!validBirthdate);
        
        int validphone = 0;
        do {
    	    printf("Enter your phone : ");
            scanf("%s", user.phone);
            if (!isValidPhoneNumber(user.phone)) {
                printf("The phone number format is invalid. Please provide a valid phone number.\n");
            } else {
                validphone = 1;
            }
        } while (!validphone);




	int client_socket;
	    struct sockaddr_in server_addr;
	    char buffer[BUFFER_SIZE] = {0};
	
	     
	    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
	        perror("Socket creation failed");
	        exit(EXIT_FAILURE);
	    }
	
	     
	    server_addr.sin_family = AF_INET;
	    server_addr.sin_port = htons(PORT);
	
	     
	    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
	        perror("Invalid address/ Address not supported");
	        exit(EXIT_FAILURE);
	    }
	
	     
	    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
	        perror("Connection failed");
	        exit(EXIT_FAILURE);
	    }
	    printf("\n\x1b[31m");
        int i;
        char spinner[] = {'|', '/', '-', '\\'};

        printf("creation of your acc ... ");
        fflush(stdout); 

        for (i = 0; i < 20; i++) {
            printf("\b%c", spinner[i % 4]); 
            fflush(stdout); 
            usleep(100000); 
        }

        printf("\n");
	
	    


        int flag = 1;

        
        if (send(client_socket, &flag, sizeof(int), 0) < 0) {
            perror("Send failed");
            close(client_socket);
            exit(EXIT_FAILURE);
        }



	    
	   
	    
	    if (send(client_socket, &user, sizeof(User), 0) < 0) {
	        perror("Send failed");
	        exit(EXIT_FAILURE);
    	    }
	
	    
	    char buf[BUFFER_SIZE];
	    if (recv(client_socket, buf, BUFFER_SIZE, 0) < 0) {
	        perror("Receive failed");
	        exit(EXIT_FAILURE);
	    }
	
	    printf("\n\x1b[32m");
    	printf("%s\n", buf);
        sleep(3);
        printf("\033[2J\033[1;1H");
	
    	close(client_socket);

        



}
void option5(const char *sender){

    printf("Envoi de Messages\n");


     
    char recipient[40], text[400];

    printf("Enter recipient name: ");
    scanf("%s", recipient);
    printf("Enter message: ");
    getchar(); 
    fgets(text, 40, stdin);
    text[strcspn(text, "\n")] = '\0';  
    

    int flag = 5;
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
   
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
    }
    
    
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
        
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
    
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }
    



   
    if(send(client_socket, &flag, sizeof(int), 0) < 0) {
        printf("Send failed");
    }

     
    snprintf(buffer, BUFFER_SIZE, "%s %s %s", sender, recipient, text);
    if (send(client_socket, buffer, strlen(buffer), 0) < 0) {
        perror("Send failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

     
    if (recv(client_socket, buffer, BUFFER_SIZE, 0) < 0) {
        perror("Receive failed");
    } else {
        printf("%s\n", buffer);
    }

    close(client_socket);

}
void option6(const char *sender){

    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
     
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
    }
    
     
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
         
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
     
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    int flag = 6;
    if(send(client_socket, &flag, sizeof(int), 0) < 0) {
        printf("Send failed");
    }

    if (send(client_socket, sender, strlen(sender), 0) < 0) {
        perror("Send failed");
    }




     
    receiveFileContentFromServer(client_socket, "messages.bin");

     
    readMessageListFromFile();

     
    close(client_socket);



}
void option7(const char *sender){
        int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
     
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
    }
    
     
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
         
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }
    
     
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    int flag = 7;
    if(send(client_socket, &flag, sizeof(int), 0) < 0) {
        printf("Send failed");
    }
    if (send(client_socket, sender, strlen(sender), 0) < 0) {
        perror("Send failed");
    }




}

void option2() {
        printf("\033[2J\033[1;1H");
        char message[] = "Authentification...";
        int len = strlen(message);


        for (int x = 0; x < len; x++) {
            printf("\x1b[33m");
            printf("\r[*] %.*s%c%s", x, message, toupper(message[x]), message + x + 1);
            fflush(stdout);
            usleep(100000);
        }
        printf("\033[2J\033[1;1H");
        printf("\n\x1b[36m");

        LoginData login_data;
        printf("Enter your username : ");
        scanf("%s", login_data.username);

        // Disable terminal echo while reading password
        disable_echo();

        printf("Enter your password (hidden ): ");
        scanf("%s", login_data.password);

        // Enable terminal echo after reading password
        enable_echo();
        

        char sender[40];
        strcpy(sender, login_data.username);
        int client_socket;
        struct sockaddr_in server_addr;
        char buffer[BUFFER_SIZE] = {0};
    
         
        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
    
         
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
    
         
        if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }
    
         
        if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
    
    
         


        int flag = 2;
        if(send(client_socket, &flag, sizeof(int), 0) < 0) {
            printf("Send failed");
        }

         
        if (send(client_socket, &login_data, sizeof(LoginData), 0) < 0) {
            printf("Send failed");
        }
         
        int received_number;
        if (recv(client_socket, &received_number, sizeof(int), 0) < 0) {
            printf("Receive failed");
        }
        

        int i;
        char spinner[] = {'|', '/', '-', '\\'};
        printf("\n\n");
        printf("Loading ... ");
        fflush(stdout);  

        for (i = 0; i < 20; i++) {
            printf("\b%c", spinner[i % 4]);  
            fflush(stdout);  
            usleep(100000);  
        }

        printf("\n");


        if (received_number == 1)
        {   
            int qq;
            printf("\033[2J\033[1;1H");
            printf("\n\x1b[32m");
            printf("You are now logged in.\n\n");
            sleep(1);
            printf("\033[2J\033[1;1H");
            printf("Preparing your environment... ");

            fflush(stdout);  

            for (i = 0; i < 20; i++) {
                printf("\b%c", spinner[i % 4]);  
                fflush(stdout);  
                usleep(100000);  
            }

            printf("\n");
            sleep(1);
            printf("\033[2J\033[1;1H");
            do
            {
                printf("\n\x1b[36m");
                printf("==========================================\n");
                printf(" ______     __  __     ______     ______  \n");
                printf("/\\  ___\\   /\\ \\_\\ \\   /\\  __ \\   /\\__  _\\ \n");
                printf("\\ \\ \\____  \\ \\  __ \\  \\ \\  __ \\  \\/_/\\ \\/ \n");
                printf(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_\\ \\_\\    \\ \\_\\ \n");
                printf("  \\/_____/   \\/_/\\/_/   \\/_/\\/_/     \\/_/ \n");
                printf("\n");
                printf("\x1b[0m");
                printf("\x1b[33m");
                printf("1. Send Messages\n");
                printf("2. Check Inbox\n");
                printf("3. Delete Messages\n");
                printf("4. Logout\n");


                printf("\x1b[0m");
                printf("\x1b[36m");
                printf("==========================================\n");
                
                do{
                    printf("\x1b[0m");
                    printf("Enter your choice: ");
                    scanf("%d", &qq);
                    while ( getchar() != '\n'){continue;}
                    if ((qq<1 || qq>5) ){
                        printf("\n\x1b[31m");
                        printf("Invalid choice. Please try again.\n");
                        printf("\x1b[0m");
                     }
                }while(qq<1 || qq>5);

                    switch(qq) {
                        case 1:
                            option5(sender);
                            sleep(3);
                            printf("\033[2J\033[1;1H");
                            break;
                        case 2:
                            printf("Loading messages... ");
                            fflush(stdout);  

                            for (i = 0; i < 20; i++) {
                                printf("\b%c", spinner[i % 4]);  
                                fflush(stdout);  
                                usleep(100000);  
                            }

                            printf("\n");
                            printf("\033[2J\033[1;1H");
                            option6(sender);
                            printf("press any key to back to menu ...");
                            getchar();
                            printf("\033[2J\033[1;1H");
                            break;
                        case 3:
                            option7(sender);
                            
                            printf("Messages successfully deleted.\n");
                            sleep(2);
                            printf("\033[2J\033[1;1H");
                            break;
                        case 4:
                            printf("loging out ...\n");
                            sleep(2);
                            printf("\033[2J\033[1;1H");
                            break;
                        default:
                            printf("\n\x1b[31m");
                            printf("Invalid choice. Please try again.\n");
                            printf("\x1b[0m");
                    }
            } while(qq != 4);
        }else{
            printf("\033[2J\033[1;1H");
            printf("\n\x1b[31m");
            printf("Incorrect username or password. Please verify your login credentials.\n");
            sleep(2);
            printf("\033[2J\033[1;1H");
        }
        close(client_socket);

}

void option3() {
    printf("\033[2J\033[1;1H");
    char message[] = "Account modification section.";
    int len = strlen(message);


    for (int x = 0; x < len; x++) {
            printf("\r[*] %.*s%c%s", x, message, toupper(message[x]), message + x + 1);
            fflush(stdout);
            usleep(100000);
    }
    printf("\033[2J\033[1;1H");
    LoginData login_data;
    User user;
    printf("Enter old username: ");
    fgets(login_data.username, sizeof(login_data.username), stdin);
    
    disable_echo();


    printf("Enter old password: ");
    fgets(login_data.password, sizeof(login_data.password), stdin);

    enable_echo();
    
    printf("\n");
    remove_newline(login_data.username);
    remove_newline(login_data.password);
   
    
    
    
    
    do {
            printf("Enter new username: ");
            fgets(user.username, sizeof(user.username), stdin);
            remove_newline(user.username);
            if (!is_valid_username(user.username)) {
                printf("Invalid username. Please enter a valid username.\n");
            }
    } while (!is_valid_username(user.username));

        do {
            disable_echo();
            printf("Enter new password (hidden ): ");
            fgets(user.password, sizeof(user.password), stdin);
 
            enable_echo();
            printf("\n");
            remove_newline(user.password);

            if (!is_valid_password(user.password)) {
                printf("Invalid password. Please enter a valid password.\n");
                continue;
            }

            break;
        } while (1);


        int validBirthdate = 0;
        do {
            
            printf("Enter new birthdate (YYYY-MM-DD): ");
            fgets(user.birthdate, sizeof(user.birthdate), stdin);
            remove_newline(user.birthdate);

            if (!isValidDate(user.birthdate)) {
                printf("Invalid birthdate format. Please enter in YYYY-MM-DD format.\n");
            } else {
                validBirthdate = 1;
            }
        } while (!validBirthdate);
        
        int validphone = 0;
        do {
            printf("Enter new phone: ");
            fgets(user.phone, sizeof(user.phone), stdin);
            remove_newline(user.phone);
            if (!isValidPhoneNumber(user.phone)) {
                printf("The phone number format is invalid. Please provide a valid phone number.\n");
            } else {
                validphone = 1;
            }
        } while (!validphone);




    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
         
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
    }
    
     
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
     
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
    }
    
     
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed");
 
    }
    


    int flag = 3;

     
    if (send(client_socket, &flag, sizeof(int), 0) < 0) {
        printf("Send failed");

    }

    if (send(client_socket, &login_data, sizeof(LoginData), 0) < 0) {
        printf("Send failed");
    }

    if (send(client_socket, &user, sizeof(User), 0) < 0) {
        printf("Send failed");
    }
 

    int received_number;
    if (recv(client_socket, &received_number, sizeof(int), 0) < 0) {
            printf("Receive failed");
    }
        



    if (received_number == 1)
    {
        printf("Account modified successfully.\n");
    }
    else if(received_number == 2)
    {
        printf("New username already taken!\n");
    }
    else
    {
        printf("Check your login data (old username and password).\n");
    }

        
    close(client_socket);

}

void option4() {
    printf("\033[2J\033[1;1H");
    char message[] = "Delete your acc... !";
    int len = strlen(message);


    for (int x = 0; x < len; x++) {
            printf("\x1b[33m");
            printf("\r[*] %.*s%c%s", x, message, toupper(message[x]), message + x + 1);
            fflush(stdout);
            usleep(100000);
    }
    printf("\033[2J\033[1;1H");

    printf("\n\n");
    User user;

    printf("Enter username: ");
    fgets(user.username, sizeof(user.username), stdin);
    disable_echo();
    printf("Enter password: ");
    fgets(user.password, sizeof(user.password), stdin);

    enable_echo();
    printf("\n");
    remove_newline(user.username);
    remove_newline(user.password);
    remove_newline(user.birthdate);
    remove_newline(user.phone);
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE] = {0};
    
         
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
    }
    
     
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    
     
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
    }
    
     
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Connection failed");
 
    }



    int flag = 4;

     
    if (send(client_socket, &flag, sizeof(int), 0) < 0) {
        printf("Send failed");

    }

    if (send(client_socket, &user, sizeof(User), 0) < 0) {
        printf("Send failed");
    }
          
        int received_number;
        if (recv(client_socket, &received_number, sizeof(int), 0) < 0) {
            printf("Receive failed");
        }
        



        if (received_number == 1)
        {
            printf("Your account has been deleted.\n");
        }
        else
        {
            printf("Invalid username or password .\n");
        }
   

        
    close(client_socket);

}

int main() {

        char message[] = "starting the chat app...";
        int len = strlen(message);


        for (int x = 0; x < len; x++) {
            printf("\x1b[31m");
            printf("\r[*] %.*s%c%s", x, message, toupper(message[x]), message + x + 1);
            fflush(stdout);
            usleep(100000);
        }
        printf("\033[2J\033[1;1H");
        int choice;
        

    do {    
            printf("\n\x1b[36m");
            printf("==========================================\n");
            printf(" ______     __  __     ______     ______  \n");
            printf("/\\  ___\\   /\\ \\_\\ \\   /\\  __ \\   /\\__  _\\ \n");
            printf("\\ \\ \\____  \\ \\  __ \\  \\ \\  __ \\  \\/_/\\ \\/ \n");
            printf(" \\ \\_____\\  \\ \\_\\ \\_\\  \\ \\_\\ \\_\\    \\ \\_\\ \n");
            printf("  \\/_____/   \\/_/\\/_/   \\/_/\\/_/     \\/_/ \n");


            printf("\n");


            printf("\x1b[0m");
            printf("\x1b[33m");
            printf("1. Create Accounts\n");
            printf("2. Authentication\n");
            printf("3. Account Modification\n");
            printf("4. Account Deletion\n");
            printf("5. Exit\n");

            printf("\x1b[0m");
            printf("\x1b[36m");
            printf("==========================================\n");
            do{
                printf("\x1b[0m");
                printf("Enter your choice: ");
                scanf("%d", &choice);
                while ( getchar() != '\n'){continue;}
                if ((choice<1 || choice>5) ){
                    printf("\n\x1b[31m");
                    printf("Invalid choice. Please try again.\n");
                    printf("\x1b[0m");
                 }
            }while(choice<1 || choice>5);



        switch(choice) {
            case 1:
                option1();
                break;
            case 2:
                option2();
                break;
            case 3:
                option3();
                sleep(2);
                printf("\033[2J\033[1;1H");
                break;
            case 4:
                option4();
                sleep(2);
                printf("\033[2J\033[1;1H");
                break;
            case 5:
                printf("\n\x1b[32m");
                printf("Exiting...\n");
                printf("\x1b[0m");
                break;
            default:
                printf("\n\x1b[31m");
                printf("Invalid choice. Please try again.\n");
                printf("\x1b[0m");
        }
    } while(choice != 5);

    return 0;
}