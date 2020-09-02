//
// Created by Matej Kolečáni on 01/10/2019.
//

#ifndef P2P_COM_QUEUE_H
#define P2P_COM_QUEUE_H

#include <stdio.h>
#include <stdlib.h>

#include "net-types.h"

struct Queue {
    b32 front;
    b32 rear;
    b32 size;
    b32 capacity;
    b32 * array;
};

struct Queue * q_init(unsigned capacity);
b32 q_full(struct Queue * queue);
b32 q_empty(struct Queue * queue);
b32 q_enqueue(struct Queue * queue, b32 value);
b32 q_dequeue(struct Queue * queue);

#endif //P2P_COM_QUEUE_H
