#ifndef COMMON_H
#define COMMON_H
/**
 * @file common.c
 * @brief Contains the definitions of common data structures used in the tracker application.
 */

#include <stdint.h>

/**
 * @struct Message
 * @brief Represents a message containing a string.
 * @param length Length of the string
 * @param string Pointer to the string table
 */
typedef struct Message
{
    int length;    // Length of the string
    char **string; // Pointer to the string
} Message;

/**
 * @struct Invalid_tracker
 * @brief Represents an invalid tracker.
 * @param last_error Indicates if the last message was an error
 * @param nb_error Number of errors
 * @param IP_adress IP address of the tracker
 * @param port Port number of the tracker
 * @param next Pointer to the next invalid tracker
 */

typedef struct Invalid_tracker
{
    int last_error;               // Indicates if the last message was an error
    int nb_error;                 // Number of errors
    char *IP_adress;              // IP address of the tracker
    int port;                     // Port number of the tracker
    struct Invalid_tracker *next; // Pointer to the next invalid tracker
} Invalid_tracker;

/**
 * @struct Node_Peers
 * @brief Represents a node in a linked list of peers.
 * @param IP_adress IP address of the peer
 * @param port Port number of the peer
 * @param next Pointer to the next node
 */
typedef struct Node_Peers
{
    char *IP_adress;         // IP address of the peer
    int port;                // Port number of the peer
    int socket;              // Socket of the peer
    struct Node_Peers *next; // Pointer to the next node
} Node_Peers;

/**
 * @struct Single_File
 * @brief Represents a file.
 * @param name Pointer to the file name
 * @param length Length of the file
 * @param pieceSize Size of each piece
 * @param key Key of the file
 */
typedef struct Single_File
{
    char *name;    // Pointer to the file name
    int length;    // Length of the file
    int pieceSize; // Size of each piece
    char *key;     // Key of the file
} Single_File;

/**
 * @struct Node_File
 * @brief Represents a node in a linked list of files.
 * @param file Pointer to the file
 * @param next Pointer to the next file
 * @param seed Pointer to the list of seeders
 * @param leech Pointer to the list of leechers
 */
typedef struct Node_File
{
    struct Single_File *file; // Pointer to the file
    struct Node_File *next;   // Pointer to the next file
    struct Node_Peers *seed;  // Pointer to the list of seeders
    struct Node_Peers *leech; // Pointer to the list of leechers
} Node_File;

/*
-----------------------PEER FUNCTIONS----------------------
*/

/**
 * @brief Initializes a new node for the linked list of peers.
 * @return A pointer to the newly initialized node.
 */
struct Node_Peers *init_node_peers();

/**
 * @brief Adds a new peer to the linked list of peers.
 * @param port_peer The port number of the peer.
 * @param IP The IP address of the peer.
 * @param head The head of the linked list of peers.
 * @return A pointer to the updated head of the linked list.
 */
struct Node_Peers *add_node_peers(int port_peer, char *IP, int socket, struct Node_Peers *head);

/**
 * @brief Frees the memory allocated for the linked list of peers.
 * @param head The head of the linked list of peers.
 */
void free_peers(Node_Peers *head);

/**
 * @brief Searches for a peer in the linked list of peers based on its IP address.
 * @param peer_list The head of the linked list of peers.
 * @param client_ip The IP address of the peer to search for.
 * @return A pointer to the found peer node, or NULL if not found.
 */
struct Node_Peers *search_peer_by_ip(struct Node_Peers *peer_list, char *client_ip);

struct Node_Peers *check_if_peer_exist(int port_peer, char *IP, struct Node_Peers *head);

struct Node_Peers *get_info_by_socket(int socket, struct Node_Peers *head);

void remove_client_ressources(int socket, struct Node_Peers *head, struct Node_File *files, struct Invalid_tracker *invalid_tracker);

/*
-----------------------FILE FUNCTIONS----------------------
*/
/**
 * @brief Initializes a new node for the linked list of files.
 * @return A pointer to the newly initialized node.
 */
struct Node_File *init_node_files();

/**
 * @brief Frees the memory allocated for the linked list of files.
 * @param files The head of the linked list of files.
 */
void free_files(Node_File *files);

/**
 * @brief Adds a new file to the linked list of files.
 * @param files_list The head of the linked list of files.
 * @param file The file to add.
 * @param seed The list of seeders for the file.
 * @param leech The list of leechers for the file.
 */
void add_file(struct Node_File *files_list, struct Single_File *file, struct Node_Peers *seed);

/**
 * @brief Searches for a file in the linked list of files based on its key.
 * @param file_list The head of the linked list of files.
 * @param key The key of the file to search for.
 * @return A pointer to the found file node, or NULL if not found.
 */
struct Node_File *search_file(struct Node_File *file_list, char *key);

void list_all_files(struct Node_File *file_list, int sock);

/**
 * @brief Searches for a file in the linked list of files based on search options.
 * @param sock The socket to send the search results to.
 * @param file_list The head of the linked list of files.
 * @param name The name of the file to search for.
 * @param key The key of the file to search for.
 * @param fsize_comp The comparison operator for file size.
 * @param fsize The file size to compare with.
 * @param psize_comp The comparison operator for piece size.
 * @param psize The piece size to compare with.
 */
void search_file_by_option(int sock, struct Node_File *file_list, char *name, char *key, char fsize_comp, int fsize, char psize_comp, int psize);

/**
 * @brief Creates a new Single_File structure.
 * @param name The name of the file.
 * @param length The length of the file.
 * @param pieceSize The size of each piece.
 * @param key The key of the file.
 * @return A pointer to the newly created Single_File structure.
 */
struct Single_File *get_new_file(char *name, int length, int pieceSize, char *key);

/*
-----------------------MESSAGE FUNCTIONS----------------------
*/

/**
 * @brief Initializes a new Message structure.
 * @return A pointer to the newly initialized Message structure.
 */
Message *initMessage();

/**
 * @brief Frees the memory allocated for a Message structure.
 * @param node The Message structure to free.
 */
void freeMessage(Message *node);

/**
 * @brief Adds a string to a Message structure.
 * @param node The Message structure to add the string to.
 * @param str The string to add.
 */
void addStringToMessage(Message *node, char *str);

/*
-----------------------TRACKER ERROR FUNCTIONS-------------------
*/

/**
 * @brief Initializes a new Invalid_tracker structure.
 * @return A pointer to the newly initialized Invalid_tracker structure.
 */
struct Invalid_tracker *init_invalid_tracker();

/**
 * @brief Adds a new invalid tracker to the linked list of invalid trackers.
 * @param port_peer The port number of the invalid tracker.
 * @param IP The IP address of the invalid tracker.
 * @param head The head of the linked list of invalid trackers.
 * @return A pointer to the updated head of the linked list.
 */
struct Invalid_tracker *add_invalid_tracker(int port_peer, char *IP, struct Invalid_tracker *head);

/**
 * @brief Checks if a tracker with the given port number and IP address exists in the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 * @param head The head of the linked list of invalid trackers.
 * @return 1 if the tracker exists, 0 otherwise.
 */
int check_if_tracker_exist(int port_peer, char *IP, struct Invalid_tracker *head);

void remove_tracker(struct Invalid_tracker *head, int port_peer, char *IP);

/**
 * @brief Frees the memory allocated for the linked list of invalid trackers.
 * @param head The head of the linked list of invalid trackers.
 */
void free_invalid_tracker(struct Invalid_tracker *head);

/**
 * @brief Sets the last error flag to zero for the specified tracker.
 * @param head The head of the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 */
void set_last_error_to_zero(struct Invalid_tracker *head, int port_peer, char *IP);

/**
 * @brief Sets the last error flag to one for the specified tracker.
 * @param head The head of the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 */
void set_last_error_to_one(struct Invalid_tracker *head, int port_peer, char *IP);

/**
 * @brief Adds an error to the specified tracker.
 * @param head The head of the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 */
void add_error(struct Invalid_tracker *head, int port_peer, char *IP);

/**
 * @brief Finds the specified tracker in the linked list of invalid trackers.
 * @param head The head of the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 * @return A pointer to the found tracker node, or NULL if not found.
 */
struct Invalid_tracker *find_error(struct Invalid_tracker *head, int port_peer, char *IP);

/**
 * @brief Removes the specified tracker from the linked list of invalid trackers.
 * @param head The head of the linked list of invalid trackers.
 * @param port_peer The port number of the tracker.
 * @param IP The IP address of the tracker.
 */
void remove_error(struct Invalid_tracker *head, int port_peer, char *IP);

/*
-----------------------OTHER FUNCTIONS----------------------
*/

/**
 * @brief Adds a seeder to the specified file.
 * @param file The file to add the seeder to.
 * @param seed The seeder to add.
 */
void add_seed(struct Node_File *file, struct Node_Peers *seed);

/**
 * @brief Adds a leecher to the specified file.
 * @param file The file to add the leecher to.
 * @param leech The leecher to add.
 */
void add_leech(struct Node_File *file, struct Node_Peers *leech);

/**
 * @brief Prints the list of files and peers.
 * @param files The head of the linked list of files.
 * @param peers The head of the linked list of peers.
 */
void print_files_and_peers(struct Node_File *files, struct Node_Peers *peers);

/**
 * @brief Prints an error message.
 * @param msg The error message to print.
 */
void error(const char *msg);

/**
 * @brief Resets the log file.
 * @param filename The name of the log file.
 */
void reset_log(const char *filename);

#endif // COMMON_H
