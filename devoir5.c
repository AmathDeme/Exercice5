#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// Définition des variables globales
#define THREAD_MAX 100

int n;                           // Nombre de threads
sem_t semaphores[THREAD_MAX];   // Tableau de sémaphores pour gérer l'ordre
int iteration = 0;               // Nombre de boucles effectuées
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex pour protéger l'accès à iteration

// Fonction exécutée par chaque thread
void *thread_Ex(void *arg) {
    int id = *(int *)arg; // Récupération de l'ID du thread

    while (1) {
        // Attendre son tour
        sem_wait(&semaphores[id]);

        pthread_mutex_lock(&mutex); // Protection de la variable partagée
        if (iteration >= 10) {
            pthread_mutex_unlock(&mutex);
            break;  // Arrêt après 10 itérations complètes
        }
        printf("Thread %d s'exécute (Itération %d)\n", id + 1, iteration + 1);

        // On incrémente l'itération si cùest le dernier thread
        if (id == n - 1) {
            iteration++;
        }
        pthread_mutex_unlock(&mutex);

        // On passe qu thread suivant
        sem_post(&semaphores[(id + 1) % n]);
    }
    // On Permet au dernier thread de débloquer les autres
    sem_post(&semaphores[(id + 1) % n]);
    return NULL;
}
int main(int argc, char *argv[]) {
    int i;
    // Vérification des arguments
    if (argc != 2) {
        printf("Usage : %s <nombre_de_threads>\n", argv[0]);
        return 1;
    }

    // Récupération du nombre de threads depuis les arguments
    n = atoi(argv[1]);
    if (n <= 0 || n > THREAD_MAX) {
        printf("Erreur : le nombre de threads doit être entre 1 et %d.\n", THREAD_MAX);
        return 1;
    }

    pthread_t threads[n];      // Tableau pour stocker les threads
    int thread_ids[n];         // Identifiants des threads

    // Initialisation des sémaphores
    for (i = 0; i < n; i++) {
        sem_init(&semaphores[i], 0, 0);
    }
    // Début du premier thread
    sem_post(&semaphores[0]);

    // Création des threads
    for (i = 0; i < n; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, thread_Ex, &thread_ids[i]) != 0) {
            perror("Erreur lors de la création d'un thread");
            return 1;
        }
    }
    //On Attend la fin de tous les threads
    for (i = 0; i < n; i++) {
        pthread_join(threads[i], NULL);
    }

    // Destruction des semaphores
    for (i = 0; i < n; i++) {
        sem_destroy(&semaphores[i]);
    }

    // Destruction du mutex
    pthread_mutex_destroy(&mutex);

    printf("Exécution terminée après %d itérations.\n", iteration);
    return 0;
}