#include "queue.h"
#include <cstddef>
#include <cstdio>
//#include <time.h>

Node *create_node(int data)
{
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (!new_node)
    {
        perror("Unable to allocate memory for new node");
        exit(EXIT_FAILURE);
    }
    new_node->data = data;
    new_node->next = NULL;
    return new_node;
}

Queue *create_queue()
{
    Queue *new_queue = (Queue *)malloc(sizeof(Queue));
    if (!new_queue)
    {
        perror("Unable to allocate memory for new queue");
        exit(EXIT_FAILURE);
    }
    new_queue->front = new_queue->rear = NULL;
    return new_queue;
}

bool queue_is_empty(Queue *q)
{
    return (q->front == NULL);
}

void enqueue(Queue *q, int data)
{
    Node *new_node = create_node(data);
    if (queue_is_empty(q))
    {
        // If the queue is empty, the new node is both the front and the rear
        q->front = q->rear = new_node;
        return;
    }
    // Add the new node at the end of the queue and change rear
    q->rear->next = new_node;
    q->rear = new_node;
}

int dequeue(Queue *q)
{
    if (queue_is_empty(q))
    {
        printf("Queue is empty, cannot dequeue.\n");
        return -1;
    }
    Node *temp = q->front;
    int data = temp->data;
    q->front = q->front->next;

    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    free(temp);
    return data;
}

void destroy_queue(Queue *q)
{
    Node *current = q->front;
    Node *next = NULL;

    while (current != NULL)
    {
        next = current->next;   // Save reference to the next node
        free(current);          // Free the current node
        current = next;         // Move to the next node
    }

    free(q); // Finally, free the queue structure itself
}

void print_waiting_queue(Queue *customer_queue)
{
    printf("Aktualna kolejka oczekujących klientów: ");
    Node *current = customer_queue->front;
    while (current != NULL)
    {
        printf("%d ", current->data);
        current = current->next;
    }
    printf("\n");
}

void print_resigned_customers(int resigned_customers_counter, int *resigned_customers)
{
    printf("Klienci którzy nie dostali się do gabinetu: ");
    for (int i = 0; i < resigned_customers_counter; i++)
    {
        printf("%d ", resigned_customers[i]);
    }
    printf("\n");
}

