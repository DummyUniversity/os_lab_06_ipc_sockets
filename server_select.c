#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT        8080
#define MAX_CLIENTS 10
#define BUF_SIZE    1024

int main(void)
{
    /* ---- Array to track connected client file descriptors ---- */
    int client_fds[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_fds[i] = -1;             /* -1 means slot is free */

    /* ---- Create and configure the listening socket ---- */
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1) { perror("socket"); exit(EXIT_FAILURE); }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind"); close(listen_fd); exit(EXIT_FAILURE);
    }
    if (listen(listen_fd, 5) == -1)
    {
        perror("listen"); close(listen_fd); exit(EXIT_FAILURE);
    }
    printf("Server: listening on port %d (select-based, up to %d clients)\n",
           PORT, MAX_CLIENTS);

    /* ---- Main select() loop ---- */
    while (1)
    {
        /* Step 1: Rebuild the fd_set from scratch every iteration */
        fd_set readfds;
        FD_ZERO(&readfds);

        /* Always watch the listening socket for new connections */
        FD_SET(listen_fd, &readfds);
        int max_fd = listen_fd;

        /* Watch all active client sockets */
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (client_fds[i] != -1)
            {
                FD_SET(client_fds[i], &readfds);
                if (client_fds[i] > max_fd)
                    max_fd = client_fds[i];   /* select() needs the highest fd */
            }
        }

        /* Step 2: Sleep until at least one fd is readable */
        int ready = select(max_fd + 1, &readfds, NULL, NULL, NULL);
        if (ready == -1) { perror("select"); break; }

        /* Step 3: Check if the listening socket is ready (new connection) */
        if (FD_ISSET(listen_fd, &readfds))
        {
            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);

            int conn_fd = accept(listen_fd,
                                 (struct sockaddr *)&client_addr,
                                 &client_len);
            if (conn_fd == -1) { perror("accept"); continue; }

            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr,
                      client_ip, sizeof(client_ip));

            /* Find a free slot in the client array */
            int placed = 0;
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (client_fds[i] == -1)
                {
                    client_fds[i] = conn_fd;
                    printf("Server: new client [slot %d] from %s:%d (fd=%d)\n",
                           i, client_ip, ntohs(client_addr.sin_port), conn_fd);
                    placed = 1;
                    break;
                }
            }

            if (!placed)
            {
                /* No free slot: reject this client */
                printf("Server: max clients reached, rejecting %s\n", client_ip);
                send(conn_fd, "Server full.\n", 13, 0);
                close(conn_fd);
            }
        }

        /* Step 4: Check each client socket for incoming data */
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int fd = client_fds[i];
            if (fd == -1 || !FD_ISSET(fd, &readfds))
                continue;

            char    buf[BUF_SIZE];
            ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);

            if (n > 0)
            {
                /* Data received: echo it back */
                buf[n] = '\0';
                printf("Server: [slot %d fd=%d] received (%zd bytes): \"%s\"\n",
                       i, fd, n, buf);
                send(fd, buf, n, 0);
            }
            else
            {
                /* n == 0: client disconnected; n == -1: error */
                if (n == 0) printf("Server: [slot %d fd=%d] disconnected.\n", i, fd);
                else        perror("recv");

                close(fd);
                client_fds[i] = -1;   /* free the slot */
            }
        }
    } /* back to top of while(1): rebuild fd_set and call select() again */

    close(listen_fd);
    return 0;
}
