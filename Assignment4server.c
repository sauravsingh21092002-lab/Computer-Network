#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void find_student(char *reg_no, char *response)
{
    FILE *fp = fopen("students.txt", "r");

    if (fp == NULL)
    {
        strcpy(response, "Error opening file");
        return;
    }

    char file_reg[50], name[50], branch[50], college[100];
    int found = 0;

    while (fscanf(fp, "%s %s %s %s",
                  file_reg, name, branch, college) != EOF)
    {
        if (strcmp(file_reg, reg_no) == 0)
        {
            sprintf(response,
                    "Name: %s, Branch: %s, College: %s",
                    name, branch, college);

            found = 1;
            break;
        }
    }

    if (!found)
    {
        strcpy(response, "Data not found");
    }

    fclose(fp);
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(server_fd,
             (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    // Accept client
    new_socket = accept(server_fd,
                        (struct sockaddr *)&address,
                        (socklen_t *)&addrlen);

    if (new_socket < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    // Continuous communication
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);

        int valread = read(new_socket, buffer, BUFFER_SIZE - 1);

        if (valread <= 0)
        {
            break;
        }

        buffer[valread] = '\0';

        // Remove newline
        buffer[strcspn(buffer, "\n")] = '\0';

        // Exit condition
        if (strcmp(buffer, "bye") == 0)
        {
            printf("Client terminated the connection.\n");
            break;
        }

        // Search student
        find_student(buffer, response);

        // Send response
        send(new_socket, response, strlen(response), 0);
    }

    close(new_socket);
    close(server_fd);

    printf("Server terminated.\n");

    return 0;
}