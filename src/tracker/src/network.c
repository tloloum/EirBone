#include "network.h"
#include "common.h"
#include <arpa/inet.h>

int initServer(int port)
{
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    if (listen(sockfd, 5) < 0)
        error("ERROR on listen");

    return sockfd;
}

int acceptConnection(int server_sock, struct sockaddr_in *cli_addr)
{
    socklen_t clilen = sizeof(*cli_addr);
    int newsockfd = accept(server_sock, (struct sockaddr *)cli_addr, &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    return newsockfd;
}

void closeConnection(int sock)
{
    close(sock);
}

ssize_t sendMessage(int sock, const char *message, size_t len)
{
    ssize_t n = write(sock, message, len);
    if (n < 0)
        error("ERROR writing to socket");
    return n;
}

ssize_t receiveMessage(int sock, char *buffer, size_t len)
{
    ssize_t n = read(sock, buffer, len);
    if (n < 0)
        error("ERROR reading from socket");
    buffer[n] = '\0';
    return n;
}

struct sockaddr_in get_client_addr(int client_sock)
{
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    getpeername(client_sock, (struct sockaddr *)&cli_addr, &cli_len);
    return cli_addr;
}
