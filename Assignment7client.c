#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFFER_SIZE 2048

int get_port(const char *roll)
{
    int len = strlen(roll);
    int last3 = (len >= 3) ? atoi(roll + len - 3) : atoi(roll);
    return 30000 + last3;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <roll>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    char *roll = argv[2];
    int port = get_port(roll);

    int sockfd;
    struct sockaddr_in server_addr;
    char input_line[BUFFER_SIZE];
    char send_packet[BUFFER_SIZE * 2];
    char response[BUFFER_SIZE];

    // UDP Socket creation
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
    {
        perror("Invalid server address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to UDP service on port %d for Roll: %s\n", port, roll);
    printf("Enter command (e.g. 'SUM 5 1 2 3 4 5' or 'EXIT'):\n");

    while (1)
    {
        printf("> ");
        fflush(stdout);

        // Read user input / check EOF
        if (fgets(input_line, sizeof(input_line), stdin) == NULL)
        {
            snprintf(send_packet, sizeof(send_packet), "%s#EXIT", roll);
            sendto(sockfd, send_packet, strlen(send_packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            break;
        }

        input_line[strcspn(input_line, "\r\n")] = '\0';
        if (strlen(input_line) == 0)
            continue;

        // Auto-prepend roll number: <roll>#<input>
        snprintf(send_packet, sizeof(send_packet), "%s#%s", roll, input_line);

        // Send to server
        sendto(sockfd, send_packet, strlen(send_packet), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

        if (strcmp(input_line, "EXIT") == 0)
        {
            printf("Exiting client...\n");
            break;
        }

        // Wait and receive response
        socklen_t addr_len = sizeof(server_addr);
        memset(response, 0, BUFFER_SIZE);
        int n = recvfrom(sockfd, response, BUFFER_SIZE - 1, 0, (struct sockaddr *)&server_addr, &addr_len);
        if (n >= 0)
        {
            response[n] = '\0';
            printf("Response: %s\n", response);
        }
    }

    close(sockfd);
    return 0;
}