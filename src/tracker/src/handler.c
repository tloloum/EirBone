#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>

#include "common.h"
#include "handler.h"

pthread_mutex_t peers_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t files_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t invalid_tracker_mutex = PTHREAD_MUTEX_INITIALIZER;

int got_char(char *str, char c)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (str[i] == c)
        {
            return 1;
        }
    }
    return 0;
}

// Fonction pour parser le message et créer une liste chaînée de mots
Message *parse_message(char *str)
{
    Message *head = NULL, *currentNode = NULL;
    char tempBuffer[1024] = {0};
    int bufferIndex = 0, insideBrackets = 0;

    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == '\r' || str[i] == '\n')
        {
            continue; // Skip newline and carriage return characters
        }
        else if (str[i] == ' ' && !insideBrackets)
        {
            if (bufferIndex > 0)
            {                                   // We have a token to add
                tempBuffer[bufferIndex] = '\0'; // Terminate the current token
                if (!currentNode)
                { // If no current node, initialize it
                    currentNode = initMessage();
                    head = currentNode; // Set head if it's the first node
                }
                addStringToMessage(currentNode, tempBuffer);
                bufferIndex = 0; // Reset buffer index for next token
            }
            continue; // Skip spaces outside brackets
        }
        else if (str[i] == '[')
        {
            insideBrackets = 1; // Mark that we're inside brackets
            if (bufferIndex > 0)
            { // If we have a token before [, add it
                tempBuffer[bufferIndex] = '\0';
                if (!currentNode)
                {
                    currentNode = initMessage();
                    head = currentNode;
                }
                addStringToMessage(currentNode, tempBuffer);
                bufferIndex = 0;
            }
        }
        else if (str[i] == ']')
        {
            insideBrackets = 0; // End of bracketed section
        }
        else
        {
            tempBuffer[bufferIndex++] = str[i]; // Add char to buffer
        }
    }
    // Check for and remove a trailing newline character before adding the last token
    if (bufferIndex > 0)
    {
        if (tempBuffer[bufferIndex - 1] == '\n')
        {
            bufferIndex--; // Decrement bufferIndex to remove newline character
        }
        if (bufferIndex > 0)
        { // Check again in case the only character was a newline
            tempBuffer[bufferIndex] = '\0';
            if (!currentNode)
            {
                currentNode = initMessage();
                head = currentNode;
            }
            addStringToMessage(currentNode, tempBuffer);
        }
    }

    return head; // Return the head of the list
}

void print_parsed_message(Message *node)
{
    if (node != NULL)
    {
        for (int i = 0; i < node->length; ++i)
        {
            printf("%s\n", node->string[i]);
        }
    }
    else
    {
        printf("Node is empty or does not exist.\n");
    }
}

void create_log(Message *node, FILE *fp)
{
    if (fp == NULL)
    {
        printf("Error while opening log file.\n");
        return;
    }
    if (node != NULL)
    {
        fprintf(fp, "> ");
        for (int i = 0; i < node->length; ++i)
        {
            fprintf(fp, "%s ", node->string[i]);
        }
        fprintf(fp, "\n");
    }
    else
    {
        printf("Node is empty or does not exist.\n");
    }

    fclose(fp);
}

void create_response_log(const char *response, FILE *fp)
{
    if (fp == NULL)
    {
        printf("Error while opening log file.\n");
        return;
    }
    fprintf(fp, "< %s\n", response);
    fclose(fp);
}

void sendResponse(int sock, const char *message)
{
    send(sock, message, strlen(message), 0);
    printf("> %s\n", message);
    create_response_log(message, fopen("log.txt", "a"));
}

void HandleError(struct Invalid_tracker *invalid_tracker, int sock, struct sockaddr_in cli_addr, int port, int *fd_to_remove)
{
    sendResponse(sock, "Invalid Command\n");
    struct Invalid_tracker *error = find_error(invalid_tracker, port, inet_ntoa(cli_addr.sin_addr));
    if (error != NULL)
    {
        if (error->last_error == 0)
        {
            set_last_error_to_one(invalid_tracker, port, inet_ntoa(cli_addr.sin_addr));
            add_error(invalid_tracker, port, inet_ntoa(cli_addr.sin_addr));
        }
        else if (error->last_error == 1)
        {
            add_error(invalid_tracker, port, inet_ntoa(cli_addr.sin_addr));
            if (error->nb_error >= 3)
            {
                sendResponse(sock, "You have made 3 errors in a row, end of connection\n");
                remove_error(invalid_tracker, port, inet_ntoa(cli_addr.sin_addr));
                *fd_to_remove = sock;
            }
        }
    }
}

int addseed(char *argument, struct Node_File *file, struct Node_Peers *peer, int sock){
    struct Message *seed_parse = parse_message(argument);
    if (seed_parse->length % 4 == 0)
    {
        for (int i = 0; i < seed_parse->length; i++)
        {
            struct Node_File *check = search_file(file, seed_parse->string[i + 3]);
            if (check == NULL)
            {
                if (atoi(seed_parse->string[i + 1]) == 0 || atoi(seed_parse->string[i + 2]) == 0)
                {
                    sendResponse(sock, "Invalid announce (wrong seed, Length or PieceSize incorrect)\n");
                    return 0;
                }
                struct Single_File *new_file = get_new_file(seed_parse->string[i], atoi(seed_parse->string[i + 1]), atoi(seed_parse->string[i + 2]), seed_parse->string[i + 3]);
                add_file(file, new_file, peer);
            }
            else
            {
                add_seed(check, peer);
            }
            i += 3;
        }
    } else {
        sendResponse(sock, "Invalid announce (wrong seed)\n");
        return 0;
    }
    freeMessage(seed_parse);
    return 1;
}

int addleech(char *argument, struct Node_File *file, struct Node_Peers *peer, int sock){
    struct Message *leech_parse = parse_message(argument);
    if (leech_parse->length % 4 == 0)
    {
        for (int i = 0; i < leech_parse->length; i++)
        {
            struct Node_File *check = search_file(file, leech_parse->string[i + 3]);
            if (check != NULL)
            {
                if (atoi(leech_parse->string[i + 1]) == 0 || atoi(leech_parse->string[i + 2]) == 0)
                {
                    sendResponse(sock, "Invalid announce (wrong seed, Length or PieceSize incorrect)\n");
                    return 0;
                }
                add_leech(check, peer);
            }
            else
            {
                sendResponse(sock, "Invalid announce, a file cannot be leech without being seed\n");
            }
            i += 3;
        }
    } else {
        sendResponse(sock, "Invalid announce (wrong leech)\n");
        return 0;
    }
    freeMessage(leech_parse);
    return 1;
}

// Traitement spécifique de la commande announce
void handleAnnounce(int sock, char *Ip, Message *parsed_message, Node_Peers *peer_list, Node_File *file_list)
{
    int port = atoi(parsed_message->string[2]);
    if (port == 0)
    {
        sendResponse(sock, "Invalid port\n");
        return;
    }
    struct Node_Peers *new_peer = add_node_peers(port, Ip, sock, peer_list);
    // print_parsed_message(parsed_message);
    if (parsed_message->length >= 5) {
        char *type = parsed_message->string[3];
        if (strcmp(type, "seed") == 0)
        {
            int seed_rep = addseed(parsed_message->string[4], file_list, new_peer, sock);
            if (seed_rep == 0)
                return;
        }
        else if (strcmp(type, "leech") == 0)
        {
            int leech_rep = addleech(parsed_message->string[4], file_list, new_peer, sock);
            if (leech_rep == 0)
                return;
        }
        else
        {
            sendResponse(sock, "Invalid announce (wrong type)\n");
            return;
        }
    } 
    if (parsed_message->length >= 7) {
        char *type = parsed_message->string[5];
        if (strcmp(type, "leech") == 0)
        {
            int leech_rep = addleech(parsed_message->string[6], file_list, new_peer, sock);
            if (leech_rep == 0)
                return;
        }
        else {
            sendResponse(sock, "Invalid announce, verify the order of the arguments (seed before leech) or you're syntax\n");
            return;
        }
    }
    
    sendResponse(sock, "ok\n");
    // print_files_and_peers(file_list, peer_list);
}

char *removeQuotes(char *filename)
{
    char *cleaned = malloc(strlen(filename) + 1);
    int j = 0;
    for (int i = 0; i < strlen(filename); i++)
    {
        if (filename[i] != '"')
        {
            cleaned[j] = filename[i];
            j++;
        }
    }
    cleaned[j] = '\0';
    return cleaned;
}

void responseAllFile(Message *all_file, int sock)
{
    char response[4096] = {0};
    int responseLength = strlen(response);

    for (int i = 0; i < all_file->length; i++)
    {
        strcat(response + responseLength, all_file->string[i]);
    }

    sendResponse(sock, response);
}

void handleLook(Message *parsed_message, int sock, Node_File *file_list)
{
    struct Message *param = parse_message(parsed_message->string[1]);
    if (strncmp("*", param->string[0], 1) == 0)
    {
        list_all_files(file_list, sock);
        freeMessage(param);
        return;
    }

    char *name = NULL;
    char fsize_comp = '\0';
    char psize_comp = '\0';
    char *key = NULL;
    int psize = 0;
    int fsize = 0;

    for (int i = 0; i < param->length; i++)
    {
        if (strncmp("filename", param->string[i], 8) == 0)
        {
            char comparator = param->string[i][8];
            if (comparator == '=')
            {
                char *filename = param->string[i] + 9;
                name = removeQuotes(filename);
            }
        }
        else if (strncmp("filesize", param->string[i], 8) == 0)
        {
            char comparator = param->string[i][8];
            if (comparator == '=' || comparator == '>' || comparator == '<')
            {
                fsize_comp = comparator;
                char *cleanfsize = removeQuotes(param->string[i] + 9);
                if (cleanfsize != NULL)
                    fsize = atoi(cleanfsize);
                free(cleanfsize);
            }
        }
        else if (strncmp("key", param->string[i], 3) == 0)
        {
            char comparator = param->string[i][3];
            if (comparator == '=')
            {
                key = removeQuotes(param->string[i] + 4);
            }
        }
        else if (strncmp("piecesize", param->string[i], 9) == 0)
        {
            char comparator = param->string[i][9];
            if (comparator == '=' || comparator == '>' || comparator == '<')
            {
                psize_comp = comparator;
                char *cleanpsize = removeQuotes(param->string[i] + 10);
                if (cleanpsize != NULL)
                    psize = atoi(cleanpsize);
                free(cleanpsize);
            }
        }
    }
    search_file_by_option(sock, file_list, name, key, fsize_comp, fsize, psize_comp, psize);
    if (name != NULL)
        free(name);
    if (key != NULL)
        free(key);
    freeMessage(param);
}

void handleGetfile(Message *parsed_message, int sock, struct Node_File *file_list)
{
    char *key = parsed_message->string[1];
    struct Node_File *requestedFile = search_file(file_list, key);
    char response[1024] = {0};

    if (requestedFile)
    {
        snprintf(response, sizeof(response), "peers %s [", key);
        int responseLength = strlen(response);

        struct Node_Peers *currentPeer = requestedFile->seed;
        while (currentPeer != NULL)
        {
            snprintf(response + responseLength, sizeof(response) - responseLength,
                     "%s:%d ", currentPeer->IP_adress, currentPeer->port);
            responseLength = strlen(response);
            currentPeer = currentPeer->next;
        }

        currentPeer = requestedFile->leech;
        while (currentPeer != NULL)
        {
            snprintf(response + responseLength, sizeof(response) - responseLength,
                     "%s:%d ", currentPeer->IP_adress, currentPeer->port);
            responseLength = strlen(response);
            currentPeer = currentPeer->next;
        }

        snprintf(response + responseLength - 1, sizeof(response) - responseLength, "]");
    }
    else
    {
        snprintf(response, sizeof(response), "peers %s", key);
    }

    strcat(response, "\n");
    sendResponse(sock, response);
}

void handleUpdate(char *Ip, Message *parsed_message, int sock, struct Node_File *files_list, struct Node_Peers *peer_list)
{
    int isSeeder = 0;
    struct Node_Peers *peer = search_peer_by_ip(peer_list, Ip);
    if (peer == NULL)
    {
        sendResponse(sock, "Peer not announced");
        return;
    }
    for (int i = 1; i < parsed_message->length; i++)
    {
        char *token = parsed_message->string[i];
        if (strcmp(token, "seed") == 0)
        {
            isSeeder = 1;
            continue;
        }
        else if (strcmp(token, "leech") == 0)
        {
            isSeeder = 0;
            continue;
        }
        if (isSeeder && token)
        {
            struct Node_File *fileNode = search_file(files_list, token);
            if (fileNode != NULL)
            {
                add_seed(fileNode, peer);
            }
        }
        else if (token && !isSeeder)
        {
            struct Node_File *fileNode = search_file(files_list, token);
            if (fileNode != NULL)
            {
                add_leech(fileNode, peer);
            }
        }
    }
    // print_files_and_peers(files_list, peer_list);
    sendResponse(sock, "ok\n");
}

void thread_work(void *arg)
{
    struct HandleArgs *args = (struct HandleArgs *)arg;

    pthread_mutex_lock(&peers_mutex);
    pthread_mutex_lock(&files_mutex);
    pthread_mutex_lock(&invalid_tracker_mutex);

    handleClient(args->client_sock, args->cli_addr, args->peers, args->files, args->message, args->invalid_tracker, args->fd_to_remove);

    pthread_mutex_unlock(&peers_mutex);
    pthread_mutex_unlock(&files_mutex);
    pthread_mutex_unlock(&invalid_tracker_mutex);

    free(args);
}

struct HandleArgs *create_handle_args(int sock, struct sockaddr_in cli_addr, Node_Peers *peer_list, Node_File *file_list, struct Invalid_tracker *invalid_tracker, int *fd_to_remove)
{
    struct HandleArgs *args = malloc(sizeof(struct HandleArgs));
    args->cli_addr = cli_addr;
    args->client_sock = sock;
    args->files = file_list;
    args->peers = peer_list;
    args->invalid_tracker = invalid_tracker;
    args->fd_to_remove = fd_to_remove;
    return args;
}

void printVisibleChars(const char* str) {
    while (*str) {
        switch (*str) {
            case '\n': printf("\\n"); break; // Nouvelle ligne
            case '\t': printf("\\t"); break; // Tabulation horizontale
            case '\v': printf("\\v"); break; // Tabulation verticale
            case '\b': printf("\\b"); break; // Retour arrière
            case '\r': printf("\\r"); break; // Retour chariot
            case '\f': printf("\\f"); break; // Saut de page
            case '\a': printf("\\a"); break; // Alert (bell)
            case '\\': printf("\\\\"); break; // Backslash
            case '\"': printf("\\\""); break; // Double guillemet
            case '\'': printf("\\\'"); break; // Guillemet simple
            default:
                // Affiche les caractères non-imprimables comme hexadécimaux
                if ((unsigned char)*str < 32 || (unsigned char)*str > 126) {
                    printf("\\x%02x", (unsigned char)*str);
                } else {
                    putchar(*str);
                }
        }
        str++;
    }
    printf("\n"); // Nouvelle ligne à la fin de l'affichage
}

void handleClient(int sock, struct sockaddr_in cli_addr, Node_Peers *peer_list, Node_File *file_list, char *buffer, struct Invalid_tracker *invalid_tracker, int *fd_to_remove)
{  
    printVisibleChars(buffer);
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    printf("< %s\n", buffer);
    FILE *fp;
    fp = fopen("log.txt", "a"); // Ouvre le fichier en mode append
    if (!check_if_tracker_exist(cli_addr.sin_port, client_ip, invalid_tracker))
        add_invalid_tracker(cli_addr.sin_port, client_ip, invalid_tracker);
    if (strcmp(buffer, "\r\n") == 0 || strcmp(buffer, "\n") == 0) {
        HandleError(invalid_tracker, sock, cli_addr, cli_addr.sin_port, fd_to_remove);
        return;
    }

    struct Message *parsed_message = parse_message(buffer);
    if (parsed_message == NULL)
    {
        sendResponse(sock, "Message null\n");
        return;
    }
    // print_parsed_message(parsed_message);

    create_log(parsed_message, fp);

    if (search_peer_by_ip(peer_list, client_ip) == NULL && strncmp("announce", parsed_message->string[0], 8) != 0){
        print_files_and_peers(file_list, peer_list);
        struct Invalid_tracker *error = find_error(invalid_tracker,cli_addr.sin_port , inet_ntoa(cli_addr.sin_addr));
        set_last_error_to_one(invalid_tracker, cli_addr.sin_port, inet_ntoa(cli_addr.sin_addr));
        add_error(invalid_tracker, cli_addr.sin_port, inet_ntoa(cli_addr.sin_addr));
        sendResponse(sock, "Invalid Command, peer not announced\n");
        if (error->nb_error >= 3)
            {
                sendResponse(sock, "You have made 3 errors in a row, end of connection\n");
                remove_error(invalid_tracker, cli_addr.sin_port, inet_ntoa(cli_addr.sin_addr));
                *fd_to_remove = sock;
            }
        return;
    }

    if (strncmp("announce", parsed_message->string[0], 8) == 0 && parsed_message->length >= 3)
    {
        handleAnnounce(sock, client_ip, parsed_message, peer_list, file_list);
        set_last_error_to_zero(invalid_tracker, cli_addr.sin_port, client_ip);
    }
    else if (strncmp("look", parsed_message->string[0], 4) == 0 && parsed_message->length == 2)
    {
        handleLook(parsed_message, sock, file_list);
        set_last_error_to_zero(invalid_tracker, cli_addr.sin_port, client_ip);
    }
    else if (strncmp("getfile", parsed_message->string[0], 7) == 0 && parsed_message->length == 2)
    {
        handleGetfile(parsed_message, sock, file_list);
        set_last_error_to_zero(invalid_tracker, cli_addr.sin_port, client_ip);
    }
    else if (strncmp("update", parsed_message->string[0], 6) == 0 && parsed_message->length >= 3)
    {
        handleUpdate(client_ip, parsed_message, sock, file_list, peer_list);
        set_last_error_to_zero(invalid_tracker, cli_addr.sin_port, client_ip);
    }
    else if (strncmp("exit", buffer, 4) == 0)
    {
        *fd_to_remove = sock;
        return;
    }
    else
    {
        HandleError(invalid_tracker, sock, cli_addr, cli_addr.sin_port, fd_to_remove);
    }

    // print_files_and_peers(file_list, peer_list);
    freeMessage(parsed_message);
    // close(sock);
}
