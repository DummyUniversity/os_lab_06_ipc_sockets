#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define PORT 5000
#define BUFFER_SIZE 1024

int main()
{
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    fd_set readfds;
    struct timeval tv;

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set socket to non-blocking mode
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL");
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

    printf("Non-blocking UDP Echo Server listening on port %d\n", PORT);

    // Main event loop
    while (1)
    {
        // Set up file descriptor set for select()
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Set timeout to 2 seconds
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        // Wait for activity on socket
        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (activity == -1)
        {
            perror("select");
            exit(EXIT_FAILURE);
        }
        else if (activity == 0)
        {
            // Timeout occurred, no data available
            printf("Waiting for client...\n");
            continue;
        }

        // Check if socket is ready for reading
        if (FD_ISSET(sockfd, &readfds))
        {
            memset(buffer, 0, BUFFER_SIZE);

            // Receive datagram from client (non-blocking)
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                             (struct sockaddr *)&clientAddr, &clientAddrLen);

            if (n == -1)
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    printf("No data available (non-blocking call)\n");
                }
                else
                {
                    perror("recvfrom");
                }
                continue;
            }

            buffer[n] = '\0';
            printf("Received from %s:%d: %s",
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
    }

    close(sockfd);
    return 0;
}
