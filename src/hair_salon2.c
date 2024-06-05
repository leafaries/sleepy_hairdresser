#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "queue.h"

#define NUM_CHAIRS 5
#define TOTAL_CUSTOMERS 10

/**
 * Prints the current status of the barbershop.
 * This function acquires a mutex to safely access shared state.
 */
void print_barbershop_status();

/**
 * Simulates work by sleeping for a random amount of time up to max_seconds.
 * @param max_seconds The maximum number of seconds the function can sleep.
 */
void simulate_work(int max_seconds);

/**
 * Routine for barber thread that handles customer haircuts.
 * This function waits for customers and simulates haircuts.
 * @param arg Unused parameter, included for compatibility with pthread_create.
 * @return Always returns NULL.
 */
void *barber_thread_routine(void *arg);

/**
 * Routine for customer thread that simulates a customer visiting the barbershop.
 * @param arg Pointer to an integer representing the customer's ID.
 * @return Always returns NULL.
 */
void *customer_thread_routine(void *arg);

/**
 * Generates customer threads.
 * @param arg Unused parameter, included for compatibility with pthread_create.
 * @return Always returns NULL.
 */
void *customer_generator_thread_routine(void *arg);

pthread_mutex_t mutex;         // Global mutex for critical sections
pthread_cond_t barber_ready;   // Condition variable to notify barber is ready
pthread_cond_t customer_ready; // Condition variable to notify customer is ready
pthread_cond_t haircut_done;   // Condition variable to notify haircut is done

int available_seats = NUM_CHAIRS;
int current_customer_on_barberchair = 0;
Queue *customer_queue;
int resigned_customers[TOTAL_CUSTOMERS];
int resigned_customers_counter = 0;
bool print_info = false;

__thread unsigned int thread_seed = 0;

/**
 * Initializes resources used in the barbershop simulation, such as mutexes and condition variables.
 */
void initialize_resources();

/**
 * Destroys resources used in the barbershop simulation to prevent resource leaks.
 */
void destroy_resources();

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-info") == 0)
    {
        print_info = true;
    }

    initialize_resources();

    pthread_t barber_thread;
    pthread_t customer_generator_thread;

    pthread_create(&barber_thread, NULL, barber_thread_routine, NULL);
    pthread_create(&customer_generator_thread, NULL, customer_generator_thread_routine, NULL);

    pthread_join(barber_thread, NULL);
    pthread_join(customer_generator_thread, NULL);

    destroy_resources();

    return 0;
}

void initialize_resources()
{
    // Initialize mutex
    if (pthread_mutex_init(&mutex, NULL) != 0)
    {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

    // Initialize conditional variables
    if (pthread_cond_init(&barber_ready, NULL) != 0 ||
        pthread_cond_init(&customer_ready, NULL) != 0 ||
        pthread_cond_init(&haircut_done, NULL) != 0)
    {
        perror("Failed to condition variables");
        exit(EXIT_FAILURE);
    }

    // Create the customer queue
    customer_queue = create_queue();
    if (customer_queue == NULL)
    {
        perror("Failed to create customer queue");
        exit(EXIT_FAILURE);
    }
}

void destroy_resources()
{
    pthread_cond_destroy(&barber_ready);
    pthread_cond_destroy(&customer_ready);
    pthread_cond_destroy(&haircut_done);
    pthread_mutex_destroy(&mutex);
    destroy_queue(customer_queue);
}

void print_barbershop_status()
{
    printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", resigned_customers_counter,
                                                              NUM_CHAIRS - available_seats,
                                                              NUM_CHAIRS,
                                                              current_customer_on_barberchair);
}

void simulate_work(int max_seconds)
{
    unsigned int seed = (thread_seed == 0) ? (unsigned int)time(NULL) ^ (unsigned int)getpid() : thread_seed;
    usleep(rand_r(&seed) % (max_seconds * 1000000));
}

void *barber_thread_routine(void *arg)
{
    while (1)
    {
        //===== ENTRY SECTION =====//

        pthread_mutex_lock(&mutex);

        while (queue_is_empty(customer_queue))
        {
            pthread_cond_wait(&customer_ready, &mutex);
        }

        //===== CRITICAL SECTION =====//

        current_customer_on_barberchair = dequeue(customer_queue);
        available_seats++;
        print_barbershop_status();
        if (print_info)
        {
            print_waiting_queue(customer_queue);
        }
        pthread_cond_broadcast(&barber_ready); // Wake up all the customer threads for a moment
        pthread_mutex_unlock(&mutex);

        simulate_work(5);

        //===== EXIT SECTION =====//

        pthread_mutex_lock(&mutex);
        current_customer_on_barberchair = 0;
        print_barbershop_status();
        if (print_info)
        {
            print_waiting_queue(customer_queue);
        }
        pthread_cond_signal(&haircut_done);
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *customer_thread_routine(void *arg)
{
    int id = *(int *)arg;
    free(arg);

    //===== ENTRY SECTION =====//

    pthread_mutex_lock(&mutex);

    //===== CRITICAL SECTION =====//

    // Check if the waiting room is full
    if (available_seats == 0)
    {
        resigned_customers[resigned_customers_counter++] = id;
        print_resigned_customers(resigned_customers_counter, resigned_customers);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    // Enqueue the customer since there are available seats
    enqueue(customer_queue, id);
    available_seats--;
    print_barbershop_status();
    if (print_info)
    {
        print_waiting_queue(customer_queue);
    }

    // Signal the barber that a custom is ready, if the barber is sleeping
    if (current_customer_on_barberchair == 0)
    {
        pthread_cond_signal(&customer_ready);
    }

    // Wait until the barber is ready for this customer
    while (current_customer_on_barberchair != id)
    {
        pthread_cond_wait(&barber_ready, &mutex);
    }

    // Wait for the haircut to complete
    pthread_cond_wait(&haircut_done, &mutex);

    //===== EXIT SECTION =====//

    pthread_mutex_unlock(&mutex);
    return NULL;
}

void *customer_generator_thread_routine(void *arg)
{
    pthread_t customer_threads[TOTAL_CUSTOMERS];

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
    {
        int *id = malloc(sizeof(int));
        if (id == NULL)
        {
            perror("Failed to allocate memory for ID");
            exit(EXIT_FAILURE);
        }
        *id = i + 1;
        if (pthread_create(&customer_threads[i], NULL, customer_thread_routine, id) != 0)
        {
            perror("Failed to create thread");
            free(id);
            exit(EXIT_FAILURE);
        }

        simulate_work(1);
    }

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    return NULL;
}

