// Roll: 25577 Service Type: B d=1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define MY_ROLL "25577"
#define PORT 30577
#define BUFFER_SIZE 2048

// Type B: Integer List Processor (SUM, MAX, MIN, AVG)
void handle_type_b(const char *cmd_text, char *response)
{
    char op[16] = {0};
    int n = 0;
    int offset = 0;

    // Command format parse karna: OP n a1 a2 ... an
    if (sscanf(cmd_text, "%15s %d%n", op, &n, &offset) < 2 || n <= 0)
    {
        snprintf(response, BUFFER_SIZE, "ERROR: Invalid Input");
        return;
    }

    const char *ptr = cmd_text + offset;
    long long sum = 0;
    int max_val = -2147483648;
    int min_val = 2147483647;

    for (int i = 0; i < n; i++)
    {
        int val = 0, bytes_read = 0;
        if (sscanf(ptr, "%d%n", &val, &bytes_read) < 1)
        {
            snprintf(response, BUFFER_SIZE, "ERROR: Count mismatch");
            return;
        }
        sum += val;
        if (val > max_val)
            max_val = val;
        if (val < min_val)
            min_val = val;
        ptr += bytes_read;
    }

    if (strcmp(op, "SUM") == 0)
    {
        snprintf(response, BUFFER_SIZE, "%lld", sum);
    }
    else if (strcmp(op, "MAX") == 0)
    {
        snprintf(response, BUFFER_SIZE, "%d", max_val);
    }
    else if (strcmp(op, "MIN") == 0)
    {
        snprintf(response, BUFFER_SIZE, "%d", min_val);
    }
    else if (strcmp(op, "AVG") == 0)
    {
        snprintf(response, BUFFER_SIZE, "%.2f", (double)sum / n);
    }
    else
    {
        snprintf(response, BUFFER_SIZE, "ERROR: Unknown Operation");
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    // UDP Socket creation
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket to port 30577
    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server running on port %d\n", PORT);

    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, BUFFER_SIZE);

        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0)
            continue;
        buffer[n] = '\0';

        // Trim trailing newline / carriage return
        buffer[strcspn(buffer, "\r\n")] = '\0';

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        // Prefix verification: Roll#
        char *hash_ptr = strchr(buffer, '#');
        if (hash_ptr == NULL)
        {
            snprintf(response, BUFFER_SIZE, "ERROR: Unauthorized client");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }

        *hash_ptr = '\0';
        char *recv_roll = buffer;
        char *payload = hash_ptr + 1;

        if (strcmp(recv_roll, MY_ROLL) != 0)
        {
            snprintf(response, BUFFER_SIZE, "ERROR: Unauthorized client");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
            continue;
        }

        // Clean EXIT handling
        if (strcmp(payload, "EXIT") == 0)
        {
            printf("[LOG] Client [%s:%d] requested EXIT. Shutting down cleanly.\n", client_ip, client_port);
            snprintf(response, BUFFER_SIZE, "Server terminating...");
            sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
            break;
        }

        // Logging according to assignment specs
        printf("[LOG] Client address: %s:%d\n", client_ip, client_port);
        printf("[LOG] Received message: %s\n", payload);

        // Type B processing
        handle_type_b(payload, response);

        printf("[LOG] Sent response: %s\n", response);

        sendto(sockfd, response, strlen(response), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}