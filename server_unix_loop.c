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
    /* ---- Create the listening socket ---- */
    int listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    /* ---- Build the address struct ---- */
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));   /* zero-initialise — important! */
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    /* Remove any leftover socket file from a previous run */
    unlink(SOCKET_PATH);

    /* ---- Bind: create the socket file on the filesystem ---- */
    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* ---- Listen: allow up to 5 pending connections ---- */
    if (listen(listen_fd, 5) == -1)
    {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server: listening on %s\n", SOCKET_PATH);

    /* ---- Accept: blocks until the client connects ---- */
    int conn_fd = accept(listen_fd, NULL, NULL);  /* NULL: we don't need client addr */
    if (conn_fd == -1)
    {
        perror("accept");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server: client connected.\n");

    char buf[BUF_SIZE];
    char response[BUF_SIZE + 8];
    ssize_t n;

    /* Loop: keep receiving until client closes the connection */
    while ((n = recv(conn_fd, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("Server: received \"%s\"\n", buf);

        int resp_len = snprintf(response, sizeof(response), "Echo: %s", buf);
        send(conn_fd, response, resp_len, 0);
    }

    if (n == 0)
    {
        printf("Server: client closed the connection.\n");
    }
    else
    {
        perror("recv");
    }

    /* ---- Cleanup ---- */
    close(conn_fd);
    close(listen_fd);
    unlink(SOCKET_PATH);    /* remove the socket file from the filesystem */
    printf("Server: done.\n");
    return 0;
}
