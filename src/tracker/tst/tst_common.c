#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../src/include/common.h"

//test que tout est bien free, vérifié avec valgrind
void tst_free_node_char(){
    // Créer une liste de mots à libérer
    struct Node_char *word_list = malloc(sizeof(struct Node_char));
    word_list->word = strdup("Test");
    word_list->next = malloc(sizeof(struct Node_char));
    word_list->next->word = strdup("libération");
    word_list->next->next = NULL;

    // Appeler la fonction à tester
    free_node_char(word_list);
}

void tst_parse_message(){
    char message[] = "Hello world!";
    struct Node_char *word_list = parse_message(message);
    
    // Tester le contenu de la liste de mots
    assert(strcmp(word_list->word, "Hello") == 0);
    assert(strcmp(word_list->next->word, "world!") == 0);
    assert(word_list->next->next == NULL);

    // Libération de la mémoire allouée pour la liste de mots
    free_node_char(word_list);
    
    printf("Tests passés avec succès.\n");


}


int main() {
    tst_free_node_char();
    tst_parse_message();
    return 0;
}