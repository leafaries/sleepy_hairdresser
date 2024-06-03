#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/time.h>

#include "queue.h"

#define NUM_CHAIRS 5
#define TOTAL_CUSTOMERS 10

void print_barbershop_status();
void simulate_work(unsigned int *seed);
void *barber_thread_routine(void *arg);
void *customer_thread_routine(void *arg);
void *customer_generator_thread_routine(void *arg);

sem_t barber_ready;
sem_t customer_ready;
pthread_mutex_t seats_mutex;
pthread_mutex_t barberchair_mutex;
pthread_mutex_t resigned_customer_counter_mutex;
pthread_mutex_t queue_mutex;

int available_seats = NUM_CHAIRS;
int resigned_customers_counter = 0;
int current_customer_on_barberchair = 0;
Queue *customer_queue;

__thread unsigned int thread_seed = 0;

unsigned int generate_initial_seed()
{
    // Using time and process ID as a simple seed generator method
    return (unsigned int) time(NULL) ^ (unsigned int )getpid();
}

unsigned int get_my_seed()
{
    // Check if the seed has been initialized
    if (thread_seed == 0)
    {
        thread_seed = generate_initial_seed();
    }
    return thread_seed;
}

int main(int argc, char *argv[])
{
    pthread_t barber_thread;
    pthread_t customer_generator_thread;

    // Initialize semaphores
    sem_init(&barber_ready, 0, 0);
    sem_init(&customer_ready, 0, 0);

    // Initialize mutex
    pthread_mutex_init(&seats_mutex, NULL);
    pthread_mutex_init(&barberchair_mutex, NULL);
    pthread_mutex_init(&resigned_customer_counter_mutex, NULL);
    pthread_mutex_init(&queue_mutex, NULL);

    // Create the customer queue
    customer_queue = create_queue();

    // Create threads
    pthread_create(&barber_thread, NULL, barber_thread_routine, NULL);
    pthread_create(&customer_generator_thread, NULL, customer_generator_thread_routine, NULL);

    // Wait for threads to finish
    pthread_join(barber_thread, NULL);
    pthread_join(customer_generator_thread, NULL);

    // Clean up resources
    sem_destroy(&barber_ready);
    sem_destroy(&customer_ready);

    pthread_mutex_destroy(&seats_mutex);
    pthread_mutex_destroy(&barberchair_mutex);
    pthread_mutex_destroy(&resigned_customer_counter_mutex);
    pthread_mutex_destroy(&queue_mutex);

    return 0;
}

void print_barbershop_status()
{
    pthread_mutex_lock(&resigned_customer_counter_mutex);
    pthread_mutex_lock(&seats_mutex);
    pthread_mutex_lock(&barberchair_mutex);
    printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n",
           resigned_customers_counter,
           NUM_CHAIRS - available_seats,
           NUM_CHAIRS,
           current_customer_on_barberchair);
    pthread_mutex_unlock(&resigned_customer_counter_mutex);
    pthread_mutex_unlock(&seats_mutex);
    pthread_mutex_unlock(&barberchair_mutex);
}

void simulate_work(unsigned int *seed)
{
    int time = rand_r(seed) % 5000000; // [0; 5 seconds)
    usleep(time);
}

void simulate_short_work(unsigned int *seed)
{
    int time = rand_r(seed) % 1000000; // [0; 1 second)
    usleep(time);
}

void *barber_thread_routine(void *arg)
{
    while (1)
    {
        sem_wait(&customer_ready);

        pthread_mutex_lock(&seats_mutex);
        available_seats++;
        pthread_mutex_unlock(&seats_mutex);
        print_barbershop_status();

        pthread_mutex_lock(&queue_mutex);
        int customer_id = dequeue(customer_queue);
        pthread_mutex_unlock(&queue_mutex);

        sem_post(&barber_ready);

        pthread_mutex_lock(&barberchair_mutex);
        current_customer_on_barberchair = customer_id;
        pthread_mutex_unlock(&barberchair_mutex);
        print_barbershop_status();
        unsigned int my_seed = get_my_seed();
        simulate_work(&my_seed);

        pthread_mutex_lock(&barberchair_mutex);
        current_customer_on_barberchair = 0;
        pthread_mutex_unlock(&barberchair_mutex);
        print_barbershop_status();
    }

    return NULL;
}

void *customer_thread_routine(void *arg)
{
    int id = *(int *)arg;
    free(arg);
    pthread_mutex_lock(&seats_mutex);

    if (available_seats > 0)
    {
        available_seats--;
        pthread_mutex_unlock(&seats_mutex);
        print_barbershop_status();

        pthread_mutex_lock(&queue_mutex);
        enqueue(customer_queue, id);
        pthread_mutex_unlock(&queue_mutex);

        sem_post(&customer_ready);
        sem_wait(&barber_ready);
    }
    else
    {
        pthread_mutex_unlock(&seats_mutex);
        pthread_mutex_lock(&resigned_customer_counter_mutex);
        resigned_customers_counter++;
        pthread_mutex_unlock(&resigned_customer_counter_mutex);
        print_barbershop_status();
    }
    return NULL;
}

void *customer_generator_thread_routine(void *arg)
{
    pthread_t customer_threads[TOTAL_CUSTOMERS];

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
    {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        if (id == NULL)
        {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }

        if (pthread_create(&customer_threads[i], NULL, customer_thread_routine, id) != 0)
        {
            perror("Failed to create thread");
            free(id);
            exit(EXIT_FAILURE);
        }

        unsigned int my_seed = get_my_seed();
        simulate_short_work(&my_seed);
    }

    for (int i = 0; i < TOTAL_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    return NULL;
}

