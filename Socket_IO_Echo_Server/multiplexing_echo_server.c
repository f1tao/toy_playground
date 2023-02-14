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

#define error_log(...)                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr);               \
        exit(1);                      \
    }
#define server_version "Welcome to Multiplexing Echo Sever"
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

    int server_fd, client_fd, client_len, max_idx = 0, i, count = 0, tmp_fd, tmp_id, j, read_size, optval, active_count, max_fd;
    struct sockaddr_in server, client;
    struct sock_info client_info_arr[SOCK_POOL];
    char client_msg[2000];
    struct timeval tv;

    // set of socket descriptors
    fd_set readfds;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    max_fd = server_fd;
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

    client_len = sizeof(struct sockaddr_in);
    while (true)
    {
        // clear the fd set first
        FD_ZERO(&readfds);

        // set the server fd to fd set
        FD_SET(server_fd, &readfds);

        // set the valid client fd to fd set
        for (i = 0; i < max_idx; i++)
        {
            if (client_info_arr[i].valid)
            {
                tmp_fd = client_info_arr[i].sock_fd;
                FD_SET(tmp_fd, &readfds);
                if (max_fd < tmp_fd)
                {
                    max_fd = tmp_fd;
                }
            }
            else
            {
                continue;
            }
        }

        // timeout is 30s
        tv.tv_sec = 30;
        tv.tv_usec = 0;

        printf("round %d begin, timeout left:%lds-%ldms.\n", count, tv.tv_sec, tv.tv_usec);

        // wait for an activity on one of the sockets, timeout is tv;
        active_count = select(max_fd + 1, &readfds, NULL, NULL, &tv);

        // return value < 0, error happend;
        if ((active_count < 0) && (errno != EINTR))
        {
            error_log("select error");
        }
        // return value = 0, timeout;
        else if (active_count == 0)
        {
            // no event happend in specific time, timeout;
            printf("select timeout, nothing happened\n");
            count += 1;
            continue;
        }

        // If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((client_fd = accept(server_fd, (struct sockaddr *)&client,
                                    &client_len)) == -1)
            {
                error_log("\tno new client: %d %s\n", errno, strerror(errno));
            }
            else
            {
                printf("\tserver: got connection from %s\n", inet_ntoa(client.sin_addr));

                for (j = 0; j < max_idx; j++)
                {
                    if (!client_info_arr[j].valid)
                        break;
                }
                if (j == max_idx && max_idx == MAX_CLIENT_COUNT)
                {
                    printf("max client count reached\n");
                    send(client_fd, "max client count reached, please wait\n", strlen("max client count reached, please wait\n"), 0);
                    close(client_fd);
                }
                else if (j == max_idx)
                {
                    // Send server version
                    send(client_fd, server_version, strlen(server_version), 0);
                    set_sock_info(client_info_arr, max_idx, client_fd, max_idx);
                    max_idx += 1;
                }
                else
                {
                    // Send server version
                    send(client_fd, server_version, strlen(server_version), 0);
                    set_sock_info(client_info_arr, j, client_fd, j);
                }
            }
        }
        else
        {
            for (i = 0; i < max_idx; i++)
            {
                if (client_info_arr[i].valid)
                {
                    tmp_fd = client_info_arr[i].sock_fd;
                    tmp_id = client_info_arr[i].client_id;
                }
                else
                {
                    continue;
                }

                if (FD_ISSET(tmp_fd, &readfds))
                {

                    read_size = recv(tmp_fd, client_msg, 2000, 0);

                    if (read_size <= 0)
                    {
                        printf("\tclient %d died, %d %s\n", tmp_id, errno, strerror(errno));
                        close(tmp_fd);
                        client_info_arr[i].valid = false;
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
        }
        count += 1;
    }
    return 0;
}
