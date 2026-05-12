#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"   /* loopback: same machine */
#define PORT      8080
#define BUF_SIZE  1024

int main(void)
{
    /* ---- Create the client socket ---- */
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    /* ---- Build the server address struct ---- */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(PORT);

    /* Convert "127.0.0.1" from text to binary and store in sin_addr */
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) != 1)
    {
        fprintf(stderr, "Invalid address: %s\n", SERVER_IP);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    /* ---- Connect: completes the TCP three-way handshake ---- */
    if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client: connected to %s:%d\n", SERVER_IP, PORT);

    /* ---- Send messages and receive echoes ---- */
    const char *messages[] = {
        "Hello, server!",
        "This is a TCP test.",
        "Goodbye.",
    };
    int num_msgs = sizeof(messages) / sizeof(messages[0]);

    char buf[BUF_SIZE];

    for (int i = 0; i < num_msgs; i++)
    {
        size_t  msg_len = strlen(messages[i]);
        ssize_t sent    = send(sock_fd, messages[i], msg_len, 0);
        if (sent == -1) { perror("send"); break; }
        printf("Client: sent \"%s\"\n", messages[i]);

        ssize_t n = recv(sock_fd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("Client: echoed back \"%s\"\n", buf);
        }
        else if (n == 0) { printf("Client: server closed connection.\n"); break; }
        else             { perror("recv"); break; }
    }

    /* ---- Close: sends TCP FIN to server; server's recv() returns 0 ---- */
    close(sock_fd);
    printf("Client: done.\n");
    return 0;
}
