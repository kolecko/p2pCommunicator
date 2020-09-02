//
// Created by Matej Kolečáni on 20/10/2019.
//

#ifndef P2P_COM_SENDER_H
#define P2P_COM_SENDER_H

#include "communicator.h"

struct com_Sender {
    struct com_Transfer * transfer;
    struct com_Transfer * metaTransfer;
    b8 * transferMap;
    b8 * metaTransferMap;
    struct Queue * transferQueue;
    struct Queue * metaTransferQueue;
};

struct com_Sender * com_demandSender();
void assignSenderMeta(struct com_Sender * sender);
void cleanSender(struct com_Sender ** sender);
void assignTransferTest(struct com_Transfer * transfer);

int com_send(char * message, char * filePath, long long * damagingMap, b32 damagingMapSize);

#endif //P2P_COM_SENDER_H
