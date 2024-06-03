#include "queue.h"

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

void enqueue(Queue *q, int data)
{
    Node *new_node = create_node(data);
    if (q->rear == NULL)
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
    if (q->front == NULL)
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

