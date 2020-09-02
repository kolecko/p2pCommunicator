//
// Created by Matej Kolečáni on 17/10/2019.
//

#ifndef P2P_COM_FLAG_SEND_H
#define P2P_COM_FLAG_SEND_H

#include "communicator.h"
#include "sender.h"

int com_flagSend(struct com_Sender * sender, b32 transferOrder, b8 flags);

#endif //P2P_COM_FLAG_SEND_H
