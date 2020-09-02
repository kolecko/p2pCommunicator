//
// Created by Matej Kolečáni on 01/10/2019.
//
#include "queue.h"

struct Queue * q_init(unsigned capacity) {
    struct Queue * queue = (struct Queue*) malloc(sizeof(struct Queue));
    queue->front = 0;
    queue->size = 0;
    queue->rear = capacity - 1;
    queue->capacity = capacity;
    queue->array = (b32 *) malloc(queue->capacity * sizeof(int));
    return queue;
}

b32 q_full(struct Queue * queue) {
    return (queue->size == queue->capacity);
}

b32 q_empty(struct Queue* queue) {
    return (queue->size == 0);
}

b32 q_enqueue(struct Queue* queue, b32 value) {
    if (q_full(queue)) return 0;
    queue->size = queue->size + 1;
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = value;
    return 1;
}

b32 q_dequeue(struct Queue* queue) {
    if (q_empty(queue)) return 0;
    b32 value = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return value;
}
