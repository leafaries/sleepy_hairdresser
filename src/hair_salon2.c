#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "queue.h"

#define NUM_CHAIRS 5
#define TOTAL_CUSTOMERS 10

void print_barbershop_status();
void simulate_work(int max_seconds);
void *barber_thread_routine(void *arg);
void *customer_thread_routine(void *arg);
void *customer_generator_thread_routine(void *arg);

pthread_mutex_t mutex;

pthread_cond_t barber_ready;
pthread_cond_t customer_ready;

int available_seats = NUM_CHAIRS;
int current_customer_on_barberchair = 0;
Queue *customer_queue;
int resigned_customers[TOTAL_CUSTOMERS];
int resigned_customers_counter = 0;
bool print_info = false;

__thread unsigned int thread_seed = 0;

int main(int argc, char *argv[])
{
    if (argc > 1 && strcmp(argv[1], "-info") == 0)
    {
        print_info = true;
    }

    pthread_t barber_thread;
    pthread_t customer_generator_thread;

    // Initialize conditional variables
    pthread_cond_init(&barber_ready, NULL);
    pthread_cond_init(&customer_ready, NULL);

    // Initialize mutex
    pthread_mutex_init(&mutex, NULL);

    // Create the customer queue
    customer_queue = create_queue();

    // Create threads
    pthread_create(&barber_thread, NULL, barber_thread_routine, NULL);
    pthread_create(&customer_generator_thread, NULL, customer_generator_thread_routine, NULL);

    // Wait for threads to finish
    pthread_join(barber_thread, NULL);
    pthread_join(customer_generator_thread, NULL);

    // Clean up resources
    pthread_cond_destroy(&barber_ready);
    pthread_cond_destroy(&customer_ready);
    pthread_mutex_destroy(&mutex);

    return 0;
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
    int time = rand_r(&seed) % (max_seconds * 1000000);
    usleep(time);
}

void *barber_thread_routine(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&mutex);
        while (queue_is_empty(customer_queue))
        {
            pthread_cond_wait(&customer_ready, &mutex);
        }

        current_customer_on_barberchair = dequeue(customer_queue);
        available_seats++;
        print_barbershop_status();
        if (print_info)
        {
            print_waiting_queue(customer_queue);
        }

        pthread_cond_signal(&barber_ready);
        pthread_mutex_unlock(&mutex);

        simulate_work(5);

        pthread_mutex_lock(&mutex);
        current_customer_on_barberchair = 0;
        print_barbershop_status();
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *customer_thread_routine(void *arg)
{
    int id = *(int *)arg;
    free(arg);
    pthread_mutex_lock(&mutex);
    if (available_seats == 0)
    {
        // Brak miejsc, klient rezygnuje
        resigned_customers[resigned_customers_counter++] = id;
        print_barbershop_status();
        if (print_info)
        {
            print_resigned_customers(resigned_customers_counter, resigned_customers);
        }
    }
    else
    {
        // Sprawdzanie czy poczekalnia jest pusta i czy fryzjer śpi (tzn. czy kolejka jest pusta)
        if (queue_is_empty(customer_queue))
        {
            if (current_customer_on_barberchair == 0)
            {
                // Fryzjer śpi, więc obudź go
                enqueue(customer_queue, id);
                pthread_cond_signal(&customer_ready);
            }
            else
            {
                // Fryzjer zajęty, więc klient siada w poczekalni
                enqueue(customer_queue, id);
                available_seats--;
                print_barbershop_status();
                pthread_cond_signal(&customer_ready);
                while (current_customer_on_barberchair != id)
                {
                    pthread_cond_wait(&barber_ready, &mutex);
                }
            }
        }
        else
        {
            // W poczekalni są inni klienci, więc klient siada i czeka na swoją kolej
            enqueue(customer_queue, id);
            available_seats--;
            print_barbershop_status();
            pthread_cond_signal(&customer_ready);
            while (current_customer_on_barberchair != id)
            {
                pthread_cond_wait(&barber_ready, &mutex);
            }
        }
    }
    pthread_mutex_unlock(&mutex);
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

        simulate_work(1);
    }

    for (int i = 0; i < TOTAL_CUSTOMERS; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    return NULL;
}

