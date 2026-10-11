#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;

    char reg_no[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("Socket creation error\n");
        return -1;
    }

    // Set server address
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
        printf("Connection failed\n");
        close(sock);
        return -1;
    }

    printf("Connected to server.\n");

    // Continuous communication
    while (1)
    {
        printf("\nEnter Registration Number (or type 'bye' to exit): ");

        fgets(reg_no, BUFFER_SIZE, stdin);

        // Remove newline
        reg_no[strcspn(reg_no, "\n")] = '\0';

        // Send registration number
        send(sock, reg_no, strlen(reg_no), 0);

        // Exit condition
        if (strcmp(reg_no, "bye") == 0)
        {
            printf("Connection closed.\n");
            break;
        }

        // Clear buffer
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