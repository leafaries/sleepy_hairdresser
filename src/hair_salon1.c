#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/time.h>

#define HAIRCUT_TIME 3 // in seconds

void print_status();
void serve_customer();
void *barber(void *arg);
void *customer(void *arg);
void *make_customers(void *arg);

// Mutex for accessing number of free waiting room seats
pthread_mutex_t modify_seats;
pthread_mutex_t srv_customer;

/* Semaphores */
// Semaphore indicating the barber is ready to cut hair
sem_t barber_ready;
// Semaphore indicating a customer is ready
sem_t customer_ready;

/* Inputs */
int chair_count;
int total_customers;

// Tracks the number of free waiting room seats
int available_seats;
int last_customer_to_leave = 0;    // 0 = nobody
int current_customer_on_chair = 0; // 0 = nobody


//// Queue for waiting customers                        NOTE: IMPLEMENTED WITH AN ARRAY
//int queue[MAX_CHAIRS]; // Queue of waiting customers
//int front = 0;         // Queue front index
//int rear = 0;          // Queue rear index
//
//void enqueue(int customerId) {
//    queue[rear] = customerId;
//    rear = (rear + 1) % MAX_CHAIRS;
//}
//
//int dequeue() {
//    int customerId = queue[front];
//    front = (front + 1) % MAX_CHAIRS;
//    return customerId;
//}

int main(int argc, char *argv[])
{
    // Initialization, should only be called once
    srand(time(NULL));

    // Barber thread
    pthread_t barber_thread;

    // Thread that create customer threads
    pthread_t customer_maker;

    printf("Podaj liczbę miejsc w poczekalni: \n");
    scanf("%d", &chair_count);
    available_seats = chair_count;

    // Initialize customers
    printf("Podaj liczbę klientów: \n");
    scanf("%d", &total_customers);
    //pthread_t customer_threads[total_customers];

    // Initialize mutex
    pthread_mutex_init(&modify_seats, NULL);

    // Initialize semaphores
    sem_init(&barber_ready, 0, 0);
    sem_init(&customer_ready, 0, 0);

    // Create barber thread
    if (pthread_create(&barber_thread, NULL, barber, NULL)) {
        fprintf(stderr, "Failed to create a barber thread\n");
        return EXIT_FAILURE;
    }

    // Create customer thread
    if (pthread_create(&customer_maker, NULL, make_customers, NULL)) {
        fprintf(stderr, "Failed to create a customer_maker thread\n");
        return EXIT_FAILURE;
    }

/*
    // Create customer threads with random arrival times
    for (int i = 0; i < total_customers; i++) {
        int *id = malloc(sizeof(int));
        if (id == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        *id = i + 1; // Customer numbering starts at 1
        if (pthread_create(&customer_threads[i], NULL, customer, id)) {
            fprintf(stderr, "Failed to create a customer %d thread\n", *id);
            return EXIT_FAILURE;
        }
        usleep(rand() % (HAIRCUT_TIME * 1000000)); // Random arrival within a haircute duration
    }

    // Join customer threads
    for (int i = 0; i < total_customers; i++) {
        pthread_join(customer_threads[i], NULL);
    }
*/
    // Join customer_maker thread
    pthread_join(customer_maker, NULL);

    // Cancel and join barber thread
    pthread_cancel(barber_thread);
    pthread_join(barber_thread, NULL);

    // Clean up resources
    pthread_mutex_destroy(&modify_seats);
    pthread_mutex_destroy(&srv_customer);
    sem_destroy(&barber_ready);
    sem_destroy(&customer_ready);

    return 0;
}

void print_status()
{
    printf("Rezygnacja:%d Poczekalnia: %d/%d [Fotel: %d]\n",
           last_customer_to_leave,
           chair_count - available_seats,
           chair_count,
           current_customer_on_chair);
}

void serveCustomer()
{
    // Random number between 0 and 400 (miliseconds)
    int s = rand() % 401;

    // Convert miliseconds to microseconds
    s *= 1000;
    usleep(s);
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

void *barber(void *arg)
{
    int counter = 0;

    while (1)
    {
        sem_wait(&customer_ready);             // Wait for a customer to be ready

//        pthread_mutex_lock(&accessQueue);     // Lock access to queue
//        int customerId = dequeue();           // Get the customer from the queue
//        pthread_mutex_unlock(&accessQueue);   // Release the queue


//       pthread_mutex_lock(&accessCurrentChair);
//        currentCustomerOnChair = customerId;
//      pthread_mutex_unlock(&accessCurrentChair);
//        printStatus();

        //pthread_mutex_lock(&accessWRSeats);   // Lock access to waiting room seats
        pthread_mutex_lock(&modify_seats);
        available_seats++;                // Increase the count of free waiting room seats
        pthread_mutex_unlock(&modify_seats);
        //pthread_mutex_unlock(&accessWRSeats); // Unlock access to waiting room seats

        //printStatus();
        sem_post(&barber_ready);               // Signal that the barber is ready to cut hair

        // pthread_mutex_lock(&accessWRSeats);   // Lock access to waiting room seats
        // printf("Strzyżenie:%d Poczekalnia %d/%d [Fotel: %d]\n", customerId, MAX_CHAIRS - numberOfFreeWRSeats, MAX_CHAIRS, customerId);
        // pthread_mutex_unlock(&accessWRSeats); // Unlock access to waiting room seats
        //sleep(HAIRCUT_TIME);                  // Simulate cutting hair

        //current_customer_on_chair = 0; // Free the barber chair
        //printStatus();

        pthread_mutex_lock(&srv_customer);
        serveCustomer();
        pthread_mutex_unlock(&srv_customer);
        printf("Customer was served.\n");

        counter++;
        if (counter == (total_customers - no_served_customers))
            break;
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

void *customer(void *arg)
{
    struct timeval start, stop;

    pthread_mutex_lock(&modify_seats);

    if (available_seats >= 1)
    {
        // Occupy a seat
        available_seats--;

        // printf("Customer[pid = %lu] is waiting.\n", pthread_self());
        // printf("Available seats: %d\n", available_seats);

        // Start waiting-time counter
        gettimeofday(&start, NULL);

        // Set the customer ready to be served
        sem_post(&customer_ready);

        pthread_mutex_unlock(&modify_seats);

        sem_wait(&barber_ready);

        gettimeofday(&stop, NULL);

        double sec = (double)(stop.tv_sec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_usec);

        // Assign the time spent to global variable (ms)
        waiting_time_sum += 1000 * sec;
        printf("Customer[pid = %lu] is beign served.\n", pthread_self());
    }
    else
    {
        pthread_mutex_unlock(&modify_seats);
        no_served_customers++;
        printf("Customer left.\n");
    }

    return NULL;
}

/*
    int customerId = *(int *)arg;
    while (1) {
        pthread_mutex_lock(&access_seats);

        if (available_seats > 0) {
            available_seats--;                // Take a seat
            printStatus(0);

//            pthread_mutex_lock(&accessQueue);     // Lock access to queue
//            enqueue(customerId);                  // Enqueue customer ID
//            pthread_mutex_unlock(&accessQueue);   // Unlock access to queue

            sem_post(&customer_ready);             // Notify the barber
            pthread_mutex_unlock(&access_seats); // Unlock access to waiting room seats
            sem_wait(&barber_ready);               // Wait for the barber to be ready
            break;                                // Exit after a haircut
        } else {
//            pthread_mutex_lock(&accessCurrentChair);
            lastResignedCustomer = customerId;
            printStatus(customerId);
//            pthread_mutex_unlock(&accessCurrentChair);
            pthread_mutex_unlock(&access_seats);
            break;                                // Exit without a haircut
        }
    }
    free(arg);
    return NULL;
    */
}

void *make_customers(void *arg)
{
    for (int i = 1; i <= total_customers; i++)
    {
        // Declare and create a customer thread
        pthread_t customer_thread;
        if (pthread_create(&customer_thread, NULL, customer, i)){
            fprintf(stderr, "Failed to create customer_thread %d", i);
            exit(EXIT_FAILURE);
        }

        // Sleep for 100ms before creating another customer
        usleep(100000);
    }
    return NULL;
}

