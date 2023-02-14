#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>    /* Added for the nonblocking socket */
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>

#define error_log(...)                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr);               \
        exit(1);                      \
    }

int server_fd;

void echo_handler(int sig)
{
    struct sockaddr_in client;

    int read_size, client_len;
    char client_msg[2000];

    read_size = recvfrom(server_fd, client_msg, 2000, 0, (struct sockaddr *)&client, &client_len);
    if (read_size <= 0)
    {
        printf("\trecv error, %d %s\n", errno, strerror(errno));
    }
    else
    {
        // end of string marker
        if (client_msg[read_size - 1] == '\n')
        {
            client_msg[read_size - 1] = '\0';
        }
        client_msg[read_size] = '\0';

        printf("\tclient %s said: %s\n", inet_ntoa(client.sin_addr), client_msg);

        // Send the message back to client
        sendto(server_fd, client_msg, strlen(client_msg), 0, (struct sockaddr *)&client, sizeof(client));
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error_log("Usage: %s [port]\n", argv[0]);
    }

    int server_port = atoi(argv[1]);

    int optval, flags;

    struct sockaddr_in server, client;

    // Create socket
    server_fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_fd < 0)
    {
        error_log("Could not create socket");
    }
    puts("Socket created");

    optval = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    // Prepare the server sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(server_port);

    struct sigaction sigio_action;
    memset(&sigio_action, 0, sizeof(sigio_action));
    sigio_action.sa_flags = 0;
    sigio_action.sa_handler = echo_handler;
    sigaction(SIGIO, &sigio_action, NULL);

    // set process to own the fd
    fcntl(server_fd, F_SETOWN, getpid());

    // set socket to O_ASYNC state
    flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_ASYNC); /* Change the socket into O_ASYNC state	*/

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        error_log("Could not bind socket");
    }
    puts("Bind done");

    while (1)
    {
        sleep(2);
    }
    close(server_fd);

    return 0;
}
