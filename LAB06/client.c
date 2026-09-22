#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char file_data[BUFFER_SIZE];
    char student_id[50];

    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Error opening local data.txt");
        exit(1);
    }

    memset(file_data, 0, BUFFER_SIZE);
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        strcat(file_data, line);
    }
    fclose(file);

    
    printf("Enter Student ID to search: ");
    if (scanf("%s", student_id) != 1) {
        printf("Invalid input.\n");
        exit(1);
    }

    
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        exit(1);
    }

    memset(buffer, 0, BUFFER_SIZE);
    snprintf(buffer, BUFFER_SIZE, "%s\n%s", student_id, file_data);
    write(sock, buffer, strlen(buffer));

    memset(buffer, 0, BUFFER_SIZE);
    int bytes_received = read(sock, buffer, BUFFER_SIZE - 1);
    if (bytes_received > 0) {
        printf("\nServer Response:\n%s\n", buffer);
    } else {
        printf("No response received from server.\n");
    }

    close(sock);
    return 0;
}