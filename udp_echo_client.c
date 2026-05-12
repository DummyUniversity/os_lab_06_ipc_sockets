#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr;
    socklen_t serverAddrLen = sizeof(serverAddr);

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address structure
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr);
    serverAddr.sin_port = htons(PORT);

    // Send "ping" to server
    char message[BUFFER_SIZE] = "ping\n";
    printf("Sending: %s", message);

    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&serverAddr, serverAddrLen) == -1)
    {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    // Receive reply from server
    memset(buffer, 0, BUFFER_SIZE);
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                     (struct sockaddr *)&serverAddr, &serverAddrLen);
    if (n == -1)
    {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }

    buffer[n] = '\0';
    printf("Received: %s", buffer);

    close(sockfd);
    return 0;
}
