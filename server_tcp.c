#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT     8080
#define BUF_SIZE 1024

int main(void)
{
    /* ---- Create the listening socket ---- */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    /* ---- SO_REUSEADDR: allow immediate restart after crash ---- */
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* ---- Build the server address struct ---- */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));   /* zero out all fields */
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;       /* accept on any interface */
    server_addr.sin_port        = htons(PORT);      /* convert to network byte order */

    /* ---- Bind: claim the port ---- */
    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* ---- Listen: kernel queues up to 5 pending connections ---- */
    if (listen(listen_fd, 5) == -1)
    {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }
    printf("Server: listening on port %d\n", PORT);

    /* ---- Accept: blocks until a client connects ---- */
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);    /* must be socklen_t, not int */

    int conn_fd = accept(listen_fd,
                         (struct sockaddr *)&client_addr,
                         &client_len);
    if (conn_fd == -1)
    {
        perror("accept");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    /* Print the connected client's IP address and port */
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
    printf("Server: connection from %s:%d\n",
           client_ip, ntohs(client_addr.sin_port));

    /* ---- Echo loop: receive and send back until client closes ---- */
    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = recv(conn_fd, buf, sizeof(buf) - 1, 0)) > 0)
    {
        buf[n] = '\0';
        printf("Server: received (%zd bytes): \"%s\"\n", n, buf);

        /* Send back the same data */
        ssize_t sent = send(conn_fd, buf, n, 0);
        if (sent == -1) { perror("send"); break; }
        printf("Server: echoed %zd bytes.\n", sent);
    }

    if (n == 0)
        printf("Server: client disconnected.\n");
    else if (n == -1)
        perror("recv");

    /* ---- Cleanup ---- */
    close(conn_fd);
    close(listen_fd);
    printf("Server: done.\n");
    return 0;
}
