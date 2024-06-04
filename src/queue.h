#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// Node structure for queue elements
typedef struct Node
{
    int data;
    struct Node *next;
} Node;

// Queue structure
typedef struct Queue
{
    Node *front;
    Node *rear;
} Queue;

// Function declarations
Node *create_node(int data);
Queue *create_queue();
bool queue_is_empty(Queue *q);
void enqueue(Queue *q, int data);
int dequeue(Queue *q);
void print_waiting_queue(Queue *customer_queue);
void print_resigned_customers(int resigned_customers_counter, int *resigned_customers);
#endif // QUEUE_H

