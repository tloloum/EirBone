#ifndef HANDLER_H
#define HANDLER_H

#include "common.h"

/**
 * @struct HandleArgs
 * @brief Structure representing the arguments for handling a client connection.
 *
 * This structure contains the necessary information for handling a client connection,
 * including the client socket, client address, list of peers, list of files, invalid tracker,
 * file descriptor to remove, and a message buffer.
 */
struct HandleArgs
{
    int client_sock;                         /**< The client socket. */
    struct sockaddr_in cli_addr;             /**< The client address. */
    struct Node_Peers *peers;                /**< The list of peers. */
    struct Node_File *files;                 /**< The list of files. */
    struct Invalid_tracker *invalid_tracker; /**< The invalid tracker. */
    int *fd_to_remove;                       /**< The file descriptor to remove. */
    char message[1024];                      /**< The message buffer. */
};

/**
 * @brief Handles the client connection.
 *
 * This function is responsible for handling the client connection. It receives the socket
 * descriptor as a parameter and performs the necessary operations to handle the client.
 *
 * @param sock The socket descriptor for the client connection.
 * @param cli_addr The client address structure.
 * @param peer_list The list of peers.
 * @param file_list The list of files.
 * @param buffer The buffer to store the received message.
 * @param invalid_tracker The structure to keep track of invalid trackers message.
 * @param fd_to_remove The file descriptor to remove if necessary.
 */
void handleClient(int sock, struct sockaddr_in cli_addr, Node_Peers *peer_list, Node_File *file_list, char *buffer, struct Invalid_tracker *invalid_tracker, int *fd_to_remove);

/**
 * @brief Function that represents the work to be done by a thread.
 *
 * @param arg The argument passed to the thread.
 */
void thread_work(void *arg);

/**
 * @brief Function that creates and initializes a HandleArgs structure.
 *
 * @param sock The socket descriptor.
 * @param cli_addr The client address structure.
 * @param peer_list The list of peers.
 * @param file_list The list of files.
 * @param invalid_tracker The structure to keep track of invalid trackers.
 * @param fd_to_remove The file descriptor to remove.
 * @return struct HandleArgs* A pointer to the created HandleArgs structure.
 */
struct HandleArgs *create_handle_args(int sock, struct sockaddr_in cli_addr, Node_Peers *peer_list, Node_File *file_list, struct Invalid_tracker *invalid_tracker, int *fd_to_remove);

#endif // HANDLER_H
