#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED

#define QUEUE_MAX_SIZE 4096

typedef struct SQueue {
    int head;
    int tail;
    char *buffer[QUEUE_MAX_SIZE];
} Queue;

void clear(Queue *queue);
void enqueue(Queue *queue, char *element);
char* dequeue(Queue *queue);
int empty(Queue *queue);
void resetIfNeeded(Queue *queue);
int size(Queue *queue);

#endif // QUEUE_H_INCLUDED