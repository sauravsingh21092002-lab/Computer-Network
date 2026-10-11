#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 4096

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    char target_id[64];
    char file_content[BUFFER_SIZE] = {0};
    char server_response[512] = {0};

    // 1. data.txt read karna
    FILE *file = fopen("data.txt", "r");
    if (file == NULL)
    {
        perror("Error opening data.txt");
        return 1;
    }

    size_t bytes_read = fread(file_content, 1, sizeof(file_content) - 1, file);
    file_content[bytes_read] = '\0';
    fclose(file);

    // 2. User se Student ID input lena
    printf("Enter Student ID to search: ");
    if (scanf("%63s", target_id) != 1)
    {
        fprintf(stderr, "Invalid input\n");
        return 1;
    }

    // 3. Socket create karna
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Server se connect karna
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    // 4. ID aur File Data server ko bhejna
    write(sock, target_id, strlen(target_id) + 1);
    usleep(10000); // Packet separation ke liye chhota delay
    write(sock, file_content, strlen(file_content) + 1);

    // 5. Server response wait aur read karna
    read(sock, server_response, sizeof(server_response) - 1);
    printf("Server Response: %s", server_response);

    close(sock);
    return 0;
}