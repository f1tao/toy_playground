#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h> /* Added for the nonblocking socket */

#define error_log(...)                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr);               \
        exit(1);                      \
    }
#define server_version "Welcome to Non-Blocking Echo Sever"
#define SOCK_POOL 132
#define MAX_CLIENT_COUNT 128
struct sock_info
{
    int sock_fd;
    int client_id;
    bool valid;
};

void set_sock_info(struct sock_info *arr, int id, int sock_fd, int client_id)
{
    arr[id].client_id = client_id;
    arr[id].sock_fd = sock_fd;
    arr[id].valid = true;
    return;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error_log("Usage: %s [port]\n", argv[0]);
    }

    int server_port = atoi(argv[1]);

    int server_fd, client_fd, client_len, idx = 0, flags, i, count = 0, tmp_fd, tmp_id, j, read_size, optval;
    struct sockaddr_in server, client;
    struct sock_info sock_info_arr[SOCK_POOL];
    char client_msg[2000];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    set_sock_info(sock_info_arr, idx, server_fd, -1);
    idx += 1;
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

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        error_log("Could not bind socket");
    }
    puts("Bind done");

    // Listen
    if (listen(server_fd, MAX_CLIENT_COUNT) < 0)
    {
        error_log("Could not listen on socket");
    }

    printf("%s is listening on %d\n", server_version, server_port);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");

    // set socket to non-blocking state
    flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK); /* Change the socket into non-blocking state	*/

    client_len = sizeof(struct sockaddr_in);
    while (true)
    {
        // new round
        printf("round: %d\n", count);
        for (i = 0; i < idx; i++)
        {
            if (sock_info_arr[i].valid)
            {
                tmp_fd = sock_info_arr[i].sock_fd;
                tmp_id = sock_info_arr[i].client_id;
            }
            else
            {
                continue;
            }

            // handle server
            if (tmp_fd == server_fd)
            {
                if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
                                        &client_len)) == -1)
                {
                    printf("\tno new client: %d %s\n", errno, strerror(errno));
                    continue;
                }
                else
                {
                    printf("\tserver: got connection from %s\n", inet_ntoa(client.sin_addr));
                    // set new client state to non-blocking state
                    flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    // find the empty sock index
                    for (j = 0; j < idx; j++)
                    {
                        if (!sock_info_arr[j].valid)
                            break;
                    }
                    if (j == idx && idx == 128)
                    {
                        printf("max client count reached\n");
                        send(client_fd, "max client count reached, please wait\n", strlen("max client count reached, please wait\n"), 0);
                        close(client_fd);
                    }
                    else if (j == idx)
                    {
                        // Send server version
                        send(client_fd, server_version, strlen(server_version), 0);
                        set_sock_info(sock_info_arr, idx, client_fd, idx);
                        idx += 1;
                    }
                    else
                    {
                        // Send server version
                        send(client_fd, server_version, strlen(server_version), 0);
                        set_sock_info(sock_info_arr, j, client_fd, j);
                    }
                }
            }
            else
            {
                read_size = recv(tmp_fd, client_msg, 2000, 0);
                if (read_size == -1 && (errno == EAGAIN || errno == EWOULDBLOCK))
                {
                    printf("\tclient %d: no data send, %d %s\n", tmp_id, errno, strerror(errno));
                }
                else if (read_size == 0)
                {
                    printf("\tclient %d died, %d %s\n", tmp_id, errno, strerror(errno));
                    sock_info_arr[i].valid = false;
                }
                else
                {
                    // end of string marker
                    if (client_msg[read_size - 1] == '\n')
                    {
                        client_msg[read_size - 1] = '\0';
                    }
                    client_msg[read_size] = '\0';

                    printf("\tclient %d said: %s\n", tmp_id, client_msg);

                    // Send the message back to client
                    send(tmp_fd, client_msg, strlen(client_msg), 0);

                    // clear the message buffer
                    memset(client_msg, 0, 2000);
                }
            }
        }
        sleep(1);
        count += 1;
    }
    return 0;
}
