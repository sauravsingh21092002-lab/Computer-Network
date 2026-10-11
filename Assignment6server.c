#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Client request handle karne ka function
void handle_client(int client_sock)
{
    char target_id[64];
    char file_content[BUFFER_SIZE];
    char response[512];

    memset(target_id, 0, sizeof(target_id));
    memset(file_content, 0, sizeof(file_content));
    memset(response, 0, sizeof(response));

    // 1. Search ID receive karna
    read(client_sock, target_id, sizeof(target_id) - 1);

    // 2. data.txt ka content receive karna
    read(client_sock, file_content, sizeof(file_content) - 1);

    // 3. File data me search karna
    char *line = strtok(file_content, "\r\n");
    int found = 0;

    while (line != NULL)
    {
        char temp_line[256];
        strncpy(temp_line, line, sizeof(temp_line) - 1);
        temp_line[sizeof(temp_line) - 1] = '\0';

        char id[64] = {0}, name[64] = {0}, dept[64] = {0};

        // Format: ID | Name | Dept parse karna
        if (sscanf(temp_line, "%s | %s | %s", id, name, dept) == 3)
        {
            if (strcmp(id, target_id) == 0)
            {
                snprintf(response, sizeof(response), "Record Found: %s %s %s\n", id, name, dept);
                found = 1;
                break;
            }
        }
        line = strtok(NULL, "\r\n");
    }

    if (!found)
    {
        snprintf(response, sizeof(response), "Record Not Found\n");
    }

    // 4. Response client ko wapas bhejna
    write(client_sock, response, strlen(response));

    close(client_sock);
}

int main()
{
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    // Zombie processes prevent karne ke liye
    signal(SIGCHLD, SIG_IGN);

    // Socket creation
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Binding
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listening
    if (listen(server_sock, 5) < 0)
    {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Infinite loop to accept multiple clients
    while (1)
    {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);
        if (client_sock < 0)
        {
            perror("Accept failed");
            continue;
        }

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("Fork failed");
            close(client_sock);
        }
        else if (pid == 0)
        {
            // Child process: client ko handle karega
            close(server_sock); // Child ko listening socket ki jarurat nahi hai
            handle_client(client_sock);
            exit(0);
        }
        else
        {
            // Parent process: next client ke liye accept() par wapas jayega
            close(client_sock); // Parent client socket close karega
        }
    }

    close(server_sock);
    return 0;
}