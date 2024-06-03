#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

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
void enqueue(Queue *q, int data);
int dequeue(Queue *q);

#endif // QUEUE_H

