#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/lab07_echo.sock"
#define BUF_SIZE    256

int main(void)
{
    /* ---- Create the client socket ---- */
    int sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    /* ---- Build the server address struct ---- */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /* ---- Connect: blocks until server calls accept() ---- */
    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("connect");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client: connected to server.\n");

    const char *messages[] =
    {
        "Message one",
        "Message two",
        "Message three",
        "Message four",
        "Message five",
    };

    char buf[BUF_SIZE];

    for (int i = 0; i < 5; i++)
    {
        send(sock_fd, messages[i], strlen(messages[i]), 0);
        printf("Client: sent \"%s\"\n", messages[i]);

        ssize_t n = recv(sock_fd, buf, sizeof(buf) - 1, 0);
        if (n > 0) { buf[n] = '\0'; printf("Client: received \"%s\"\n", buf); }
    }

    /* close() causes server's recv() to return 0 */
    close(sock_fd);
    printf("Client: closed connection.\n");

    return 0;
}
