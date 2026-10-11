#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8081
#define BUFFER_SIZE 1024

int main()
{
    int sock;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

    // Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Server IP address
    if (inet_pton(AF_INET, "127.0.0.1",
                  &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address\n");
        close(sock);
        return -1;
    }

    // Connect to server
    if (connect(sock,
                (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to server.\n");

    while (1)
    {
        printf("\nEnter command (ANALYZE | text) or 'bye': ");

        fgets(input, BUFFER_SIZE, stdin);

        input[strcspn(input, "\n")] = 0;

        // Send request
        send(sock, input, strlen(input), 0);

        // Exit condition
        if (strcmp(input, "bye") == 0)
        {
            printf("Connection terminated.\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

        // Receive response
        int valread = read(sock, buffer, BUFFER_SIZE - 1);

        if (valread <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        buffer[valread] = '\0';

        printf("Server Response: %s\n", buffer);
    }

    close(sock);

    return 0;
}