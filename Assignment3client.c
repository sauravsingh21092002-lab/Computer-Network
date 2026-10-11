#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Change IP here if using different machine
    if (inet_pton(AF_INET, "127.0.0.1",
                  &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address\n");
        close(sock);
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        printf("Connection failed\n");
        close(sock);
        return -1;
    }

    printf("Connected to server.\n");

    while (1)
    {
        printf("\nEnter command (UP|text, LOW|text, REV|text) or 'bye':\n");

        fgets(input, BUFFER_SIZE, stdin);

        // Remove newline
        input[strcspn(input, "\n")] = 0;

        send(sock, input, strlen(input), 0);

        // End client when bye is entered
        if (strcmp(input, "bye") == 0)
        {
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

        int valread = read(sock, buffer, BUFFER_SIZE - 1);

        if (valread <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[valread] = '\0';

        printf("Server response: %s\n", buffer);
    }

    close(sock);

    return 0;
}