#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h> //for threading , link with lpthread

#define error_log(...)                \
    {                                 \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr);               \
        exit(1);                      \
    }
#define server_version "Welcome to Blocking Echo Sever"

#define SOCK_POOL 132
#define MAX_CLIENT_COUNT 128
struct client_info
{
    int client_fd;
    int client_id;
};

// the thread function
// This will handle connection for each client
void *echo_handler(void *client_info)
{
    // Get the socket descriptor
    int client_fd = ((struct client_info *)client_info)->client_fd;
    int client_id = ((struct client_info *)client_info)->client_id;

    int read_size;
    char client_msg[2000];
    // Send server version
    send(client_fd, server_version, strlen(server_version), 0);
    // Receive msg and repeat
    while ((read_size = recv(client_fd, client_msg, 2000, 0)) > 0)
    {
        // end of string marker
        if (client_msg[read_size - 1] == '\n')
        {
            client_msg[read_size - 1] = '\0';
        }
        client_msg[read_size] = '\0';

        printf("\tclient %d said: %s\n", client_id, client_msg);

        // Send the message back to client
        send(client_fd, client_msg, strlen(client_msg), 0);

        // clear the message buffer
        memset(client_msg, 0, 2000);
    }

    if (read_size == 0)
    {
        printf("Client %d disconnected\n", client_id);
        fflush(stdout);
        close(client_fd);
    }
    else if (read_size == -1)
    {
        error_log("recv failed");
    }

    return 0;
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        error_log("Usage: %s [port]\n", argv[0]);
    }

    int server_port = atoi(argv[1]);

    int server_fd, client_fd, client_len, optval, client_id = 0;
    struct sockaddr_in server, client;
    struct client_info client_info;

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
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
    pthread_t thread_id;

    while ((client_fd = accept(server_fd, (struct sockaddr *)&client, (socklen_t *)&client_len)))
    {
        puts("New connection accepted");

        client_info.client_id = client_id;
        client_info.client_fd = client_fd;

        if (pthread_create(&thread_id, NULL, echo_handler, (void *)&client_info) < 0)
        {
            error_log("could not create thread");
        }

        puts("Handler to client is assigned");
        client_id += 1;
    }

    if (client_fd < 0)
    {
        error_log("accept failed");
    }

    return 0;
}
