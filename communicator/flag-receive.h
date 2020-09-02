//
// Created by Matej Kolečáni on 17/10/2019.
//

#ifndef P2P_COM_FLAG_RECEIVE_H
#define P2P_COM_FLAG_RECEIVE_H

#include "communicator.h"

struct com_t_flagReceive_args {
    struct com_Transfer * transfer;
    b8 * transferMap;
    int status;
    int (* callback)(struct com_Transfer *, b8 *, struct net_Header *, struct net_HeaderMeta *, int * status);
};

b8 com_flagReceive (
        struct com_Transfer * transfer,
        b8 * chunksMap,
        int * status,
        int (* callback)(struct com_Transfer *, b8 *, struct net_Header *, struct net_HeaderMeta *, int * status)
);

void * com_t_flagReceive(struct com_t_flagReceive_args * args);


#endif //P2P_COM_FLAG_RECEIVE_H
