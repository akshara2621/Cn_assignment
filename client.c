#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int count_words(char *str) {
    int count = 0, in_word = 0;
    while (*str) {
        if (*str == ' ' || *str == '\t' || *str == '\n') {
            in_word = 0;
        } else if (in_word == 0) {
            in_word = 1;
            count++;
        }
        str++;
    }
    return count;
}

int count_vowels(char *str) {
    int count = 0;
    while (*str) {
        char ch = *str;
        if (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u' ||
            ch == 'A' || ch == 'E' || ch == 'I' || ch == 'O' || ch == 'U') {
            count++;
        }
        str++;
    }
    return count;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) break;
        
        buffer[strcspn(buffer, "\n")] = 0;
        if (strcmp(buffer, "bye") == 0) break;

        char command[20], text[BUFFER_SIZE];
        if (sscanf(buffer, "%[^|]|%[^\n]", command, text) == 2) {
            // Trim leading space from text if any
            char *t = text;
            while (*t == ' ') t++;

            if (strcmp(command, "ANALYZE") == 0) {
                int chars = strlen(t);
                int words = count_words(t);
                int vowels = count_vowels(t);

                char response[BUFFER_SIZE];
                sprintf(response, "Chars=%d, Words=%d, Vowels=%d", chars, words, vowels);
                send(new_socket, response, strlen(response), 0);
            } else {
                char *msg = "Invalid Command";
                send(new_socket, msg, strlen(msg), 0);
            }
        } else {
            char *msg = "Invalid Format. Use: COMMAND|TEXT";
            send(new_socket, msg, strlen(msg), 0);
        }
    }

    close(new_socket);
    close(server_fd);
    return 0;
}