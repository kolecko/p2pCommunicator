//
// Created by Matej Kolečáni on 18/10/2019.
//

#ifndef P2P_COM_CHUNK_SEND_H
#define P2P_COM_CHUNK_SEND_H

#include "communicator.h"

int com_chunkSend(struct com_Transfer * transfer, b8 flags, b32 chunkNumber, void * container);

#endif //P2P_COM_CHUNK_SEND_H
