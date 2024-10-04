#ifndef NETWORK_H
#define NETWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * @brief Initializes a server socket on the specified port.
 *
 * @param port The port number to bind the server socket to.
 * @return The server socket file descriptor if successful, -1 otherwise.
 */
int initServer(int port);

/**
 * @brief Accepts a connection on the server socket.
 *
 * @param server_sock The server socket file descriptor.
 * @param cli_addr Pointer to a sockaddr_in structure that will contain the client address information.
 * @return The client socket file descriptor if successful, -1 otherwise.
 */
int acceptConnection(int server_sock, struct sockaddr_in *cli_addr);

/**
 * @brief Closes the connection on the specified socket.
 *
 * @param sock The socket file descriptor.
 */
void closeConnection(int sock);

/**
 * @brief Sends a message on the specified socket.
 *
 * @param sock The socket file descriptor.
 * @param message The message to send.
 * @param len The length of the message.
 * @return The number of bytes sent if successful, -1 otherwise.
 */
ssize_t sendMessage(int sock, const char *message, size_t len);

/**
 * @brief Receives a message on the specified socket.
 *
 * @param sock The socket file descriptor.
 * @param buffer The buffer to store the received message.
 * @param len The length of the buffer.
 * @return The number of bytes received if successful, -1 otherwise.
 */
ssize_t receiveMessage(int sock, char *buffer, size_t len);

/**
 * Retrieves the client's address associated with the given socket.
 *
 * @param client_sock The socket descriptor of the client connection.
 * @return The sockaddr_in structure representing the client's address.
 */
struct sockaddr_in get_client_addr(int client_sock);

#endif // NETWORK_H
