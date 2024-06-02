#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_CHAIRS 5     // Maximum number of chairs in the waiting room
#define NUM_CUSTOMERS 10 // Number of customers
#define HAIRCUT_TIME 3   // Duration of a haircut in seconds

pthread_mutex_t accessQueue;   // Mutex for accessing the queue
pthread_mutex_t accessWRSeats; // Mutex for accessing number of free waiting room seats
sem_t barberReady;             // Semaphore indicating the barber is ready to cut hair
sem_t customerReady;           // Semaphore indicating a customer is ready
int numberOfFreeWRSeats;       // Tracks the number of free waiting room seats

int lastResignedCustomer;
int currentCustomerOnChair;

void *barber(void *arg);
void *customer(void *arg);
void printStatus();

// Queue for waiting customers
int queue[MAX_CHAIRS]; // Queue of waiting customers
int front = 0;         // Queue front index
int rear = 0;          // Queue rear index

void enqueue(int customerId) {
    queue[rear] = customerId;
    rear = (rear + 1) % MAX_CHAIRS;
}

int dequeue() {
    int customerId = queue[front];
    front = (front + 1) % MAX_CHAIRS;
    return customerId;
}

int main(int argc, char *argv[]) {
    pthread_t barberThread;
    pthread_t customerThreads[NUM_CUSTOMERS];
    numberOfFreeWRSeats = MAX_CHAIRS;

    // Initialize semaphores and mutexes
    sem_init(&barberReady, 0, 0);
    sem_init(&customerReady, 0, 0);
    pthread_mutex_init(&accessQueue, NULL);
    pthread_mutex_init(&accessWRSeats, NULL);

    srand(time(NULL));

    // Create barber thread
    pthread_create(&barberThread, NULL, barber, NULL);

    // Create customer threads with random arrival times
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1; // Customer numbering starts at 1
        pthread_create(&customerThreads[i], NULL, customer, id);
        usleep(rand() % (HAIRCUT_TIME * 1000000)); // Random arrival within a haircute duration
    }

    // Join customer threads
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        pthread_join(customerThreads[i], NULL);
    }

    // Cancel and join barber thread
    pthread_cancel(barberThread);
    pthread_join(barberThread, NULL);

    // Clean up resources
    pthread_mutex_destroy(&accessQueue);
    pthread_mutex_destroy(&accessWRSeats);
    sem_destroy(&barberReady);
    sem_destroy(&customerReady);

    return 0;
}

void printStatus() {
    pthread_mutex_lock(&accessWRSeats);
    printf("Rezygnacja:%d Poczekalnia: %d/%d [Fotel: %d]\n",
           lastResignedCustomer,
           MAX_CHAIRS - numberOfFreeWRSeats,
           MAX_CHAIRS,
           currentCustomerOnChair);
    pthread_mutex_unlock(&accessWRSeats);
}

// pseudocode
// def Barber():
//      while true:
//          wait(customerReady)
//          wait(accessWRSeats)
//          numberOfFreeWRSeats++
//          signal(barberReady)
//          signal(accessWRSeats)
//          # Cut hair
//

void *barber(void *arg) {
    while (1) {
        sem_wait(&customerReady);             // Wait for a customer to be ready
        pthread_mutex_lock(&accessQueue);     // Lock access to queue

        int customerId = dequeue();           // Get the customer from the queue

        pthread_mutex_lock(&accessWRSeats);   // Lock access to waiting room seats
        numberOfFreeWRSeats++;                // Increase the count of free waiting room seats
        pthread_mutex_unlock(&accessWRSeats); // Unlock access to waiting room seats

        pthread_mutex_unlock(&accessQueue);   // Release the queue
        sem_post(&barberReady);               // Signal that the barber is ready to cut hair

        // pthread_mutex_lock(&accessWRSeats);   // Lock access to waiting room seats
        // printf("Strzyżenie:%d Poczekalnia %d/%d [Fotel: %d]\n", customerId, MAX_CHAIRS - numberOfFreeWRSeats, MAX_CHAIRS, customerId);
        // pthread_mutex_unlock(&accessWRSeats); // Unlock access to waiting room seats
        displayInfo(customerId);
        sleep(HAIRCUT_TIME);                  // Simulate cutting hair
    }
    return NULL;
}

// pseudocode
// def Customer():
//      while true:
//          wait(accessWRSeats)
//          if numberOfFreeWRSeats > 0:
//              numberOfFreeWRSeats--
//              signal(customerReady)
//              signal(accessWRSeats)
//              wait(barberReady)
//              # Have cut hair
//          else:
//              signal(accessWRSeats)
//              # Leave without a haircut
//

void *customer(void *arg) {
    int customerId = *(int *)arg;
    while (1) {
        pthread_mutex_lock(&accessWRSeats);

        if (numberOfFreeWRSeats > 0) {
            numberOfFreeWRSeats--;                // Take a seat

            pthread_mutex_lock(&accessQueue);     // Lock access to queue
            enqueue(customerId);                  // Enqueue customer ID
            pthread_mutex_unlock(&accessQueue);   // Unlock access to queue

            sem_post(&customerReady);             // Notify the barber
            pthread_mutex_unlock(&accessWRSeats); // Unlock access to waiting room seats

            sem_wait(&barberReady);               // Wait for the barber to be ready
            printf("Customer %d is having a haircut.\n", customerId);
            sleep(2);                             // Simulate time spent having haircut
            break;                                // Exit after a haircut
        } else {
            printf("Rezygnacja:%d Poczekalnia: %d/%d\n", customerId, MAX_CHAIRS - numberOfFreeWRSeats, MAX_CHAIRS);
            pthread_mutex_unlock(&accessWRSeats);
            break;                                // Exit without a haircut
        }
    }
    free(arg);
    return NULL;
}

