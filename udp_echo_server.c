#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

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
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("UDP Echo Server listening on port %d\n", PORT);

    // Receive datagrams and send replies
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);

        // Receive datagram from client
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr *)&clientAddr, &clientAddrLen);
        if (n == -1)
        {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';
        printf("Received from %s:%d: %s\n",
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port),
               buffer);

        // Send reply
        char response[BUFFER_SIZE] = "pong\n";
        if (sendto(sockfd, response, strlen(response), 0,
                   (struct sockaddr *)&clientAddr, clientAddrLen) == -1)
        {
            perror("sendto");
            continue;
        }

        printf("Sent reply: %s", response);
    }

    close(sockfd);
    return 0;
}
