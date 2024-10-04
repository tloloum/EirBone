#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "common.h"
#include "thpool.h"

#define MAX_WORDS 100

void remove_seed(struct Node_File *file, struct Node_File *current_file, int port_peer, char *IP, int *toremove);
void print_file_info(struct Node_File *current);
void print_peer_info(struct Node_Peers *current);
void remove_file(struct Node_File *head, struct Node_File *todelete);
void remove_leech(struct Node_File *file, struct Node_File *current_file, int port_peer, char *IP);
/*
#####################################################
##################  PEERS FUNCTIONS #################
#####################################################
*/
struct Node_Peers *init_node_peers()
{
    struct Node_Peers *peers = malloc(sizeof(struct Node_Peers));
    if (!peers)
    {
        perror("Failed to allocate memory for peers");
        exit(EXIT_FAILURE);
    }
    peers->IP_adress = NULL;
    peers->next = NULL;
    peers->port = 0;
    return peers;
}

/**
 * Checks if a peer with the given port and IP address already exists in the linked list.
 *
 * @param port_peer The port number of the peer.
 * @param IP The IP address of the peer.
 * @param head The head of the linked list.
 * @return Returns a pointer to the node containing the peer if it exists, otherwise returns NULL.
 */
struct Node_Peers *check_if_peer_exist(int port_peer, char *IP, struct Node_Peers *head)
{
    if (head == NULL)
    {
        return NULL;
    }
    while (head->next != NULL)
    {
        if (head->port == port_peer && strcmp(head->IP_adress, IP) == 0)
        {
            return head;
        }
        head = head->next;
    }
    if (head->port == port_peer && strcmp(head->IP_adress, IP) == 0)
    {
        return head;
    }
    return NULL;
}

struct Node_Peers *add_node_peers(int port_peer, char *IP, int socket, struct Node_Peers *head)
{
    printf("adding : %s, %d\n", IP, port_peer);
    struct Node_Peers *current = check_if_peer_exist(port_peer, IP, head);
    if (current != NULL)
    {
        return current;
    }
    while (head->next != NULL)
    {
        head = head->next;
    }
    struct Node_Peers *peers = malloc(sizeof(struct Node_Peers));
    peers->IP_adress = strdup(IP);
    printf("IP add: %s\n", peers->IP_adress);
    peers->next = NULL;
    peers->port = port_peer;
    peers->socket = socket;
    head->next = peers;
    return peers;
}

void free_peers(Node_Peers *head)
{
    Node_Peers *current = head;
    while (current != NULL)
    {
        Node_Peers *temp = current;
        current = current->next;
        free(temp->IP_adress); // Libérer la mémoire allouée pour le mot
        free(temp);            // Libérer la mémoire allouée pour le nœud
    }
}


struct Node_Peers *search_peer_by_ip(struct Node_Peers *peer_list, char *client_ip)
{
    struct Node_Peers *current = peer_list;
    while (current->next != NULL)
    {
        if (current->IP_adress == NULL || strlen(current->IP_adress) != strlen(client_ip))
        {
            current = current->next;
            continue;
        }
        if (strcmp(client_ip, current->IP_adress) == 0)
            return current;
        current = current->next;
    }
    if (current->IP_adress && strcmp(client_ip, current->IP_adress) == 0)
        return current;
    return NULL;
}


void delete_peer(struct Node_Peers *head, int port_peer, char *IP)
{
    struct Node_Peers *current = head;
    struct Node_Peers *last;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            if (last != NULL)
                last->next = current->next;
            free(current->IP_adress);
            free(current);
            return;
        }
        last = current;
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        last->next = NULL;
        free(current->IP_adress);
        free(current);
    }
}

struct Node_Peers *get_info_by_socket(int socket, struct Node_Peers *head)
{
    struct Node_Peers *current = head;
    while (current->next != NULL)
    {
        if (current->socket == socket)
        {
            return current;
        }
        current = current->next;
    }
    if (current->socket == socket)
    {
        return current;
    }
    return NULL;
}

void remove_client_ressources(int socket, struct Node_Peers *head, struct Node_File *files, struct Invalid_tracker *invalid_tracker) {
    struct Node_Peers *peer = get_info_by_socket(socket, head);
    if (peer == NULL)
        return;

    int port_peer = peer->port;
    char *IP = peer->IP_adress;

    if (check_if_tracker_exist(port_peer, IP, invalid_tracker))
        remove_tracker(invalid_tracker, port_peer, IP);

    struct Node_File *current = files;
    struct Node_File *prev = NULL;

    while (current != NULL) {
        int toremove = 0;
        
        if (current->seed != NULL)
            remove_seed(files, current, port_peer, IP, &toremove);
        
        if (current->leech != NULL)
            remove_leech(files, current, port_peer, IP);

        struct Node_File *temp = current;

        if (toremove) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                files = current->next;
            }
            current = current->next;
            remove_file(files, temp);
        } else {
            prev = current;
            current = current->next;
        }
    }

    delete_peer(head, port_peer, IP);
}


/*
#####################################################
##################  FILES FUNCTIONS #################
#####################################################
*/

struct Node_File *init_node_files()
{
    struct Node_File *files = malloc(sizeof(struct Node_File));
    if (!files)
    {
        perror("Failed to allocate memory for files");
        exit(EXIT_FAILURE);
    }
    files->file = malloc(sizeof(struct Single_File));
    files->file->name = NULL;
    files->file->key = NULL;
    files->file->length = 0;
    files->file->pieceSize = 0;
    files->next = NULL;
    files->seed = NULL;
    files->leech = NULL;
    return files;
}

/**
 * Frees the memory allocated for a Single_File structure.
 *
 * @param file The Single_File structure to be freed.
 */
void free_single_file(Single_File *file)
{
    free(file->name);
    free(file->key);
    free(file);
}

void free_files(Node_File *files)
{
    Node_File *current = files;
    while (current->next != NULL)
    {
        Node_File *temp = current;
        current = current->next;
        free_single_file(temp->file);
        free_peers(temp->seed);
        free_peers(temp->leech);
        free(temp);
    }
}

void add_file(struct Node_File *files_list, struct Single_File *file, struct Node_Peers *seed)
{
    struct Node_File *current = files_list;
    while (current->next != NULL)
    {
        current = current->next;
    }
    current->next = malloc(sizeof(struct Node_File));
    current = current->next;
    current->file = file;
    struct Node_Peers *copy_seed = malloc(sizeof(struct Node_Peers));
    copy_seed->IP_adress = strdup(seed->IP_adress);
    copy_seed->port = seed->port;
    copy_seed->next = NULL;
    current->seed = copy_seed;
}

struct Node_File *search_file(struct Node_File *file_list, char *key)
{
    Node_File *current = file_list;
    while (current->next != NULL)
    {
        if (current->file->key != NULL)
        {
            if (strcmp(current->file->key, key) == 0)
            {
                return current;
            }
        }
        current = current->next;
    }
    if (current->file->key != NULL)
    {
        if (strcmp(current->file->key, key) == 0)
        {
            return current;
        }
    }
    return NULL;
}

void list_all_files(struct Node_File *file_list, int sock){
    char response[4096];
    snprintf(response, sizeof(response), "list [");
    int responseLength = strlen(response);

    struct Node_File *current = file_list;
    int last = 0;
    int first = 1;

    while (current->next != NULL || !last) {
        if (current->next == NULL) last++;

        if (current->file->name == NULL) {
            if (!last) current = current->next;
            continue;
        }

        if (first) {
            first = 0;
        } else {
            strncat(response, " ", sizeof(response) - strlen(response) - 1);
        }

        snprintf(response + responseLength, sizeof(response) - responseLength,
                 "%s %d %d %s ", current->file->name, current->file->length,
                 current->file->pieceSize, current->file->key);
        responseLength = strlen(response);

        if (!last) current = current->next;
    }
    size_t len = strlen(response);
    response[len - 1] = '\0';
    strncat(response, "]\n", sizeof(response) - strlen(response) - 1);
    send(sock, response, strlen(response), 0);
    printf("> %s\n", response);
}


void search_file_by_option(int sock, struct Node_File *file_list, char *name, char *key, char fsize_comp, int fsize, char psize_comp, int psize)
{
    char response[4096] = "list ";
    int responseLength = strlen(response);

    struct Node_File *current = file_list;
    int last = 0;
    while (current->next != NULL || !last)
    {
        int match = 1;
        int found = 0;
        if (current->next == NULL)
            last++;
        if (current->file->name != NULL)
        {
            if (name && strcmp(current->file->name, name) != 0)
            {
                match = 0;
            }
            else if (name)
            {
                found++;
            }
            if (fsize_comp == '>')
            {
                if (fsize && current->file->length <= fsize)
                {
                    match = 0;
                }
                else if (fsize)
                {
                    found++;
                }
            }
            if (fsize_comp == '<')
            {
                if (fsize && current->file->length >= fsize)
                {
                    match = 0;
                }
                else if (fsize)
                {
                    found++;
                }
            }
            if (fsize_comp == '=')
            {
                if (fsize && current->file->length == fsize)
                {
                    match = 0;
                }
                else if (fsize)
                {
                    found++;
                }
            }
            if (psize_comp == '>')
            {
                if (psize && current->file->pieceSize <= psize)
                {
                    match = 0;
                }
                else if (psize)
                {
                    found++;
                }
            }
            if (psize_comp == '<')
            {
                if (psize && current->file->pieceSize >= psize)
                {
                    match = 0;
                }
                else if (psize)
                {
                    found++;
                }
            }
            if (psize_comp == '=')
            {
                if (psize && current->file->pieceSize == psize)
                {
                    match = 0;
                }
                else if (psize)
                {
                    found++;
                }
            }
            if (key && strcmp(current->file->key, key) != 0)
            {
                match = 0;
            }
            else if (key)
            {
                found++;
            }
            if (match && found > 0)
            {
                found = 1;
                snprintf(response + responseLength, sizeof(response) - responseLength,
                         "[%s %d %d %s] ", current->file->name, current->file->length,
                         current->file->pieceSize, current->file->key);
                responseLength = strlen(response);
            }
        }
        if (!last)
            current = current->next;
    }
    strcat(response, "\n");
    send(sock, response, strlen(response), 0);
    printf("> %s\n", response);
}

struct Single_File *get_new_file(char *name, int length, int pieceSize, char *key)
{
    struct Single_File *file = malloc(sizeof(struct Single_File));
    file->name = strdup(name);
    file->length = length;
    file->pieceSize = pieceSize;
    file->key = strdup(key);
    return file;
}

void remove_file(struct Node_File *head, struct Node_File *todelete){
    struct Node_File *current = head;
    struct Node_File *last = NULL;
    while (current->next != NULL)
    {
        if (current == todelete)
        {
            last->next = current->next;
            free_single_file(current->file);
            free(current->seed->IP_adress);
            free(current->seed);
            if (current->leech != NULL){
                free(current->leech->IP_adress);
                free(current->leech);
            }
            free(current);
            return;
        }
        last = current;
        current = current->next;
    }
    if (current == todelete)
    {
        last->next = NULL;
        free_single_file(current->file);
        free(current->seed->IP_adress);
        free(current->seed);
        if (current->leech != NULL){
            free(current->leech->IP_adress);
            free(current->leech);
        }
        free(current);
    }
}
void remove_seed(struct Node_File *file, struct Node_File *current_file, int port_peer, char *IP, int *toremove)
{
    int is_the_only_seed = 0;

    if (current_file->seed == NULL) {
        *toremove = 0;
        return;
    }

    if (current_file->seed != NULL && current_file->seed->port == port_peer && strcmp(current_file->seed->IP_adress, IP) == 0)
    {
        if (current_file->seed->next == NULL)
            is_the_only_seed = 1;
    }
    if (is_the_only_seed)
    {
        *toremove = 1;
    }
    else {
        struct Node_Peers *current = current_file->seed;
        struct Node_Peers *last = NULL;

        while (current != NULL)
        {
            if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
            {
                if (current != current_file->seed)
                {
                    if (last != NULL) {
                        last->next = current->next;
                    }
                    struct Node_Peers *temp = current;
                    current = current->next;
                    free(temp->IP_adress);
                    free(temp);
                    continue;
                }
                else
                {
                    current_file->seed = current->next;
                    struct Node_Peers *temp = current;
                    current = current->next;
                    free(temp->IP_adress);
                    free(temp);
                    continue;
                }
            }
            else
            {
                last = current;
                current = current->next;
            }
        }

        if (current == NULL && last != NULL && last->next != NULL && last->next->port == port_peer && strcmp(last->next->IP_adress, IP) == 0)
        {
            struct Node_Peers *temp = last->next;
            last->next = NULL;
            free(temp->IP_adress);
            free(temp);
        }
    }
}

void remove_leech(struct Node_File *file, struct Node_File *current_file, int port_peer, char *IP)
{
    int is_the_only_leech = 0;

    if (current_file->leech == NULL) {
        return;
    }

    if (current_file->leech != NULL && current_file->leech->port == port_peer && strcmp(current_file->leech->IP_adress, IP) == 0)
    {
        if (current_file->leech->next == NULL)
            is_the_only_leech = 1;
    }
    if (is_the_only_leech)
    {
        free(current_file->leech->IP_adress);
        free(current_file->leech);
        current_file->leech = NULL;
    }
    else
    {
        struct Node_Peers *current = current_file->leech;
        struct Node_Peers *last = NULL;
        while (current != NULL)
        {
            if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
            {
                if (current != current_file->leech)
                {
                    if (last != NULL) {
                        last->next = current->next;
                    }
                    struct Node_Peers *temp = current;
                    current = current->next;
                    free(temp->IP_adress);
                    free(temp);
                    continue;
                }
                else
                {
                    current_file->leech = current->next;
                    struct Node_Peers *temp = current;
                    current = current->next;
                    free(temp->IP_adress);
                    free(temp);
                    continue;
                }
            }
            else
            {
                last = current;
                current = current->next;
            }
        }

        // Vérification si le dernier nœud est celui à supprimer
        if (last != NULL && last->next == NULL && current != NULL && current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            free(current->IP_adress);
            free(current);
            last->next = NULL;
        }
    }
}


/*
#####################################################
##################  MESSAGE FUNCTIONS ###############
#####################################################
*/

Message *initMessage()
{
    Message *node = (Message *)malloc(sizeof(Message));
    if (!node)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    node->length = 0;
    node->string = NULL;
    return node;
}

void freeMessage(Message *node)
{
    if (!node)
        return;
    for (int i = 0; i < node->length; i++)
    {
        free(node->string[i]);
    }
    free(node->string);
    free(node);
}

void addStringToMessage(Message *node, char *str)
{
    node->length += 1;
    node->string = (char **)realloc(node->string, node->length * sizeof(char *));
    if (!node->string)
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
    node->string[node->length - 1] = strdup(str);
    if (!node->string[node->length - 1])
    {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }
}

/*
#####################################################
##################  TRACKER ERRORS ##################
#####################################################
*/

struct Invalid_tracker *init_invalid_tracker()
{
    struct Invalid_tracker *tracker = malloc(sizeof(struct Invalid_tracker));
    if (!tracker)
    {
        perror("Failed to allocate memory for invalid tracker");
        exit(EXIT_FAILURE);
    }
    tracker->IP_adress = NULL;
    tracker->next = NULL;
    tracker->port = 0;
    return tracker;
}

struct Invalid_tracker *add_invalid_tracker(int port_peer, char *IP, struct Invalid_tracker *head)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        current = current->next;
    }
    struct Invalid_tracker *tracker = malloc(sizeof(struct Invalid_tracker));
    tracker->IP_adress = strdup(IP);
    tracker->next = NULL;
    tracker->port = port_peer;
    tracker->last_error = 0;
    tracker->nb_error = 0;
    current->next = tracker;
    return tracker;
}

int check_if_tracker_exist(int port_peer, char *IP, struct Invalid_tracker *head)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            return 1;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        return 1;
    }
    return 0;
}

void remove_tracker(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    struct Invalid_tracker *last = NULL;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            if (last != NULL)
                last->next = current->next;
            free(current->IP_adress);
            free(current);
            return;
        }
        last = current;
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        last->next = NULL;
        free(current->IP_adress);
        free(current);
    }
}

void free_invalid_tracker(struct Invalid_tracker *head)
{
    struct Invalid_tracker *current = head;
    while (current != NULL)
    {
        struct Invalid_tracker *temp = current;
        current = current->next;
        free(temp->IP_adress);
        free(temp);
    }
}

void set_last_error_to_zero(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            current->last_error = 0;
            current->nb_error = 0;
            return;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        current->last_error = 0;
    }
}

void set_last_error_to_one(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            current->last_error = 1;
            return;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        current->last_error = 1;
    }
}

void add_error(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            current->nb_error += 1;
            return;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        current->nb_error += 1;
    }
}

struct Invalid_tracker *find_error(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
        {
            return current;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        return current;
    }
    return NULL;
}

void remove_error(struct Invalid_tracker *head, int port_peer, char *IP)
{
    struct Invalid_tracker *current = head;
    while (current->next != NULL)
    {
        if (current->next->port == port_peer && strcmp(current->next->IP_adress, IP) == 0)
        {
            struct Invalid_tracker *temp = current->next;
            current->next = current->next->next;
            free(temp->IP_adress);
            free(temp);
            return;
        }
        current = current->next;
    }
    if (current->port == port_peer && strcmp(current->IP_adress, IP) == 0)
    {
        free(current->IP_adress);
        free(current);
    }
}

/*
#####################################################
##################  COMMON FUNCTIONS ################
#####################################################
*/

void add_seed(struct Node_File *file, struct Node_Peers *peer)
{
    struct Node_Peers *check = check_if_peer_exist(peer->port, peer->IP_adress, file->seed);
    if (check != NULL)
    {
        return;
    }
    struct Node_Peers *current = file->seed;
    while (current->next != NULL)
    {
        current = current->next;
    }
    struct Node_Peers *copy_peer = malloc(sizeof(struct Node_Peers));
    copy_peer->IP_adress = strdup(peer->IP_adress);
    copy_peer->port = peer->port;
    copy_peer->next = NULL;
    current->next = copy_peer;
}

void add_leech(struct Node_File *file, struct Node_Peers *peer)
{
    struct Node_Peers *check = check_if_peer_exist(peer->port, peer->IP_adress, file->leech);
    if (check != NULL)
    {
        return;
    }
    struct Node_Peers *current = file->leech;
    if (current == NULL)
    {
        struct Node_Peers *copy_peer = malloc(sizeof(struct Node_Peers));
        copy_peer->IP_adress = strdup(peer->IP_adress);
        copy_peer->port = peer->port;
        copy_peer->next = NULL;
        file->leech = copy_peer;
        return;
    }
    while (current->next != NULL)
    {
        current = current->next;
    }
    struct Node_Peers *copy_peer = malloc(sizeof(struct Node_Peers));
    copy_peer->IP_adress = strdup(peer->IP_adress);
    copy_peer->port = peer->port;
    copy_peer->next = NULL;
    current->next = copy_peer;
}

void print_file_info(struct Node_File *current)
{
    printf("File : %s\n", current->file->name);
    printf("Key : %s\n", current->file->key);
    printf("Length : %d\n", current->file->length);
    printf("PieceSize : %d\n", current->file->pieceSize);
    printf("Seeders : \n");
    struct Node_Peers *current_seed = current->seed;
    if (current_seed != NULL)
    {
        while (current_seed->next != NULL)
        {
            printf("IP : %s\n", current_seed->IP_adress);
            printf("Port : %d\n", current_seed->port);
            current_seed = current_seed->next;
        }
        printf("IP : %s\n", current_seed->IP_adress);
        printf("Port : %d\n", current_seed->port);
    }
    else
    {
        printf("No seeders\n");
    }

    printf("Leechers : \n");
    struct Node_Peers *current_leech = current->leech;
    if (current_leech == NULL)
    {
        printf("No leechers\n");
    }
    else
    {
        while (current_leech->next != NULL)
        {
            printf("IP : %s\n", current_leech->IP_adress);
            printf("Port : %d\n", current_leech->port);
            current_leech = current_leech->next;
        }
        printf("IP : %s\n", current_leech->IP_adress);
        printf("Port : %d\n", current_leech->port);
    }
}

void print_peer_info(struct Node_Peers *current)
{
    printf("IP : %s\n", current->IP_adress);
    printf("Port : %d\n", current->port);
}

void print_files_and_peers(struct Node_File *files, struct Node_Peers *peers)
{
    struct Node_File *current = files;
    printf("------------------FILES-AND-PEERS------------------\n");
    printf("-------------FILES-----------------\n");
    while (current->next != NULL)
    {
        print_file_info(current);
        current = current->next;
        printf("\n");
    }
    print_file_info(current);

    printf("------------------PEERS------------------\n");
    struct Node_Peers *current_peer = peers;
    while (current_peer->next != NULL)
    {
        print_peer_info(current_peer);
        current_peer = current_peer->next;
    }
    print_peer_info(current_peer);
}

void reset_log(const char *filename)
{
    remove(filename);
}

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}