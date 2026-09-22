#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Helper function to trim trailing/leading whitespace and carriage returns
void trim_string(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || str[len - 1] == ' ' || str[len - 1] == '\t')) {
        str[len - 1] = '\0';
        len--;
    }
    while (*str == ' ' || *str == '\t') {
        memmove(str, str + 1, strlen(str));
    }
}

// Function to handle individual client communication in a child process
void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);

    // Read Student ID and file contents sent by the client[span_0](start_span)[span_0](end_span)
    int bytes_read = read(client_sock, buffer, BUFFER_SIZE - 1);
    if (bytes_read <= 0) {
        close(client_sock);
        exit(0);
    }

    // Parse payload: First line is Student ID, remaining is file data
    char *student_id = strtok(buffer, "\n");
    char *file_data = strtok(NULL, "");

    if (!student_id || !file_data) {
        char *msg = "Record Not Found";
        write(client_sock, msg, strlen(msg));
        close(client_sock);
        exit(0);
    }

    trim_string(student_id);

    // Search for the requested Student ID in the received data[span_1](start_span)[span_1](end_span)
    char line_copy[BUFFER_SIZE];
    strcpy(line_copy, file_data);

    char *line = strtok(line_copy, "\n");
    int found = 0;
    char found_record[256];

    while (line != NULL) {
        if (strlen(line) == 0) {
            line = strtok(NULL, "\n");
            continue;
        }

        char temp_line[256];
        strcpy(temp_line, line);
        char *id_token = strtok(temp_line, "|");
        
        if (id_token != NULL) {
            trim_string(id_token);

            if (strcmp(id_token, student_id) == 0) {
                found = 1;
                // Trim trailing carriage returns/newlines from the full line output
                trim_string(line);
                snprintf(found_record, sizeof(found_record), "Record Found: %s", line);
                break;
            }
        }
        line = strtok(NULL, "\n");
    }

    // Send the corresponding record or error back to the client[span_2](start_span)[span_2](end_span)
    if (found) {
        write(client_sock, found_record, strlen(found_record));
    } else {
        char *msg = "Record Not Found";
        write(client_sock, msg, strlen(msg));
    }

    close(client_sock);
    exit(0);
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;

    // 1. Create a TCP socket[span_3](start_span)[span_3](end_span)
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket[span_4](start_span)[span_4](end_span)
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    // Listen for incoming client connections[span_5](start_span)[span_5](end_span)
    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        // Accept incoming client connections[span_6](start_span)[span_6](end_span)
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        // Create a child process using fork() for each connected client[span_7](start_span)[span_7](end_span)
        pid_t pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(client_sock);
            continue;
        }

        if (pid == 0) {
            // Child process handles the client independently[span_8](start_span)[span_8](end_span)
            close(server_sock); 
            handle_client(client_sock);
        } else {
            // Parent process continues accepting new client connections[span_9](start_span)[span_9](end_span)
            close(client_sock);
            while (waitpid(-1, NULL, WNOHANG) > 0);
        }
    }

    close(server_sock);
    return 0;
}