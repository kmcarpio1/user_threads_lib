#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include "thread.h"

#define SIZE 10000

/* Fonction qui alloue une grande quantité de mémoire sur la pile */
void * stack_overflow(void *arg) {
    (void) arg;

    printf("Thread %p alloue en dehors de sa pile et écrit dessus\n", thread_self());
    int big_array[SIZE]; // Alloue plus grand que la taille de la pile
    memset(big_array, 10, SIZE*sizeof(int)); // Essaie d'écrire en dehors de la pile

    // Ceci crée le stack overflow (sans, le compilateur n'exécute pas les lignes précédentes)
    printf("Lecture dans %p de valeur %d\n", big_array+3, big_array[3]); 
    

    return NULL;
}

/* Test du thread_self et yield du main seul, avec un thread débordant de sa pile */
int main() {
    int err;

    // Vérifie que la pile principale peut exécuter cette fonction sans déborder
    stack_overflow(0);
    printf("Pas de stack overflow pour la pile principale\n");

    // Création d'un thread débordant de sa pile
    thread_t overflow_thread;
    err = thread_create(&overflow_thread, stack_overflow, NULL);
    assert(!err);

    err = thread_yield();
    assert(!err);

    // Affichage de l'adresse du main
    printf("On retourne au main %p\n", (void*)thread_self());

    // Attente de la fin du thread débordant de sa pile
    err = thread_join(overflow_thread, NULL);
    if (err == EFAULT) {
        printf("Erreur : débordement de pile détecté dans le thread\n");
    } else if (err) {
        printf("Erreur lors de la jointure du thread : %d\n", err);
    } else {
        printf("Thread débordant de sa pile terminé avec succès\n");
    }

    return 0;
}
