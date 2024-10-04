#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#include "thpool.h"
#include "handler.h"
#include "network.h"

#define DEFAULT_PORT 8000
#define MAX_CLIENTS 3

int server_fd = -1;
fd_set master_fds;
threadpool thpool = NULL;
struct Node_Peers *peers = NULL;
struct Node_File *files = NULL;
struct Invalid_tracker *invalid_tracker = NULL;
int *fd_to_remove = NULL;

void handle_signal(int signum)
{
    printf("\nReceived signal %d. Closing tracker...\n", signum);

    if (thpool)
        thpool_destroy(thpool);

    if (server_fd >= 0)
        close(server_fd);

    if (peers)
        free_peers(peers);

    if (files)
        free_files(files);

    if (invalid_tracker)
        free_invalid_tracker(invalid_tracker);

    if (fd_to_remove)
        free(fd_to_remove);

    exit(0);
}

int chose_port(int argc, char *argv[])
{
    int port;
    if (argc < 2)
    {
        port = DEFAULT_PORT;
    }
    else
    {
        port = atoi(argv[1]);
    }
    return port;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, handle_signal);

    peers = init_node_peers();
    files = init_node_files();
    invalid_tracker = init_invalid_tracker();

    server_fd = initServer(chose_port(argc, argv));
    printf("Server started on port %d\n", chose_port(argc, argv));

    fd_to_remove = malloc(sizeof(int));
    *fd_to_remove = -1;

    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    int max_sd = server_fd;

    thpool = thpool_init(MAX_CLIENTS);
    reset_log("log.txt");

    while (1)
    {
        if (*fd_to_remove != -1)
        {
            close(*fd_to_remove);
            FD_CLR(*fd_to_remove, &master_fds);
            *fd_to_remove = -1;
        }

        fd_set read_fds = master_fds;

        if (select(max_sd + 1, &read_fds, NULL, NULL, NULL) < 0)
        {
            perror("select error");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i <= max_sd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == server_fd)
                {
                    struct sockaddr_in address;
                    socklen_t addrlen = sizeof(address);
                    int new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
                    printf("New connection, socket fd is %d\n", new_socket);
                    if (new_socket < 0)
                    {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }
                    FD_SET(new_socket, &master_fds);
                    if (new_socket > max_sd)
                    {
                        max_sd = new_socket; // Mettre à jour max_sd
                    }
                }
                else
                {
                    struct HandleArgs *args = create_handle_args(i, get_client_addr(i), peers, files, invalid_tracker, fd_to_remove);
                    ssize_t read_bytes = recv(i, args->message, sizeof(args->message), 0);
                    args->message[read_bytes] = '\0';
                    if (read_bytes <= 0)
                    {
                        printf("Client %d disconnected\n", i);
                        remove_client_ressources(i, peers, files, invalid_tracker);
                        close(i);
                        FD_CLR(i, &master_fds);
                        free(args);
                    }
                    else
                    {
                        thpool_add_work(thpool, thread_work, args);
                    }
                    // print_files_and_peers(files, peers);
                }
            }
        }
    }

    return 0;
}
