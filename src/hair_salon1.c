#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_CHAIRS 5
#define NUM_CUSTOMERS 10

sem_t waitingRoom;            // Semafor kontrolujący dostęp do krzeseł w poczekalni
pthread_mutex_t barberChair;  // Mutex ochrony dostępu do fotela fryzjerskiego
int freeChairs = MAX_CHAIRS;  // Liczba wolnych krzeseł

void *barber(void *arg);
void *customer(void *num);

int main(int argc, char *argv[]) {
    pthread_t barberThread;
    pthread_t customerThreads[NUM_CUSTOMERS];

    sem_init(&waitingRoom, 0, MAX_CHAIRS);
    pthread_mutex_init(&barberChair, NULL);

    pthread_create(&barberThread, NULL, barber, NULL);

    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&customerThreads[i], NULL, customer, id);
        usleep(rand() % 100000); // Klienci przychodzą w losowych momentach
    }

    void *ret_val;
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customerThreads[i], &ret_val);
        free(ret_val);
    }

    pthread_cancel(barberThread);
    pthread_join(barberThread, NULL);

    sem_destroy(&waitingRoom);
    pthread_mutex_destroy(&barberChair);

    return 0;
}

void *barber(void *arg) {
    while (1) {
        // Czeka na klienta
        sem_wait(&waitingRoom);
        pthread_mutex_lock(&barberChair);
        printf("Fryzjer strzyże klienta.\n");
        usleep(rand() % 500000); // Symulacja strzyżenia, czas losowy
        printf("Fryzjer skończył strzyżenie klienta.\n");
        pthread_mutex_unlock(&barberChair);
        sem_post(&waitingRoom);
    }
    return NULL;
}

void *customer(void *num) {
    int id = *(int *)num;
    if (sem_trywait(&waitingRoom) == 0) { // Próba zajęcia miejsca
        pthread_mutex_lock(&barberChair);
        printf("Klient %d jest strzyżony.\n", id);
        sem_post(&waitingRoom);
    } else {
        printf("Klient %d rezygnuje z powodu braku miejsc.\n", id);
    }
    return num;
}

