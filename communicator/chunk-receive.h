//
// Created by Matej Kolečáni on 18/10/2019.
//

#ifndef P2P_COM_CHUNK_RECEIVE_H
#define P2P_COM_CHUNK_RECEIVE_H

#include "communicator.h"

struct com_t_chunkReceive_args {
    struct com_Transfer * transfer;
    b8 * transferMap;
    void * container;
    int status;
    b32 * transferOrder;
    int (* callback)(struct com_Transfer *, b8 *, void *, struct net_Header *, struct net_HeaderMeta *, void *, int *);
};

b8 com_chunkReceive (
    struct com_Transfer * transfer,
    b8 * transferMap,
    void * container,
    int * status,
    b32 * transferOrder,
    int (* callback)(struct com_Transfer *, b8 *, void *, struct net_Header *, struct net_HeaderMeta *, void *, int *)
);

void * com_t_chunkReceive(struct com_t_chunkReceive_args * args);

#endif //P2P_COM_CHUNK_RECEIVE_H
