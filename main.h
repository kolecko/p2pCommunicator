//
// Created by Matej Kolečáni on 07/10/2019.
//

#ifndef P2P_COM_MAIN_H
#define P2P_COM_MAIN_H

#include <stdio.h>
#include <pthread.h>
#include "crc.h"

#include "communicator/communicator.h"
#include "user-interface.h"

#define THREAD_COUNT   3

#define THREAD_UI      0
#define THREAD_REC     1

extern pthread_t threads[THREAD_COUNT];

b16 defaultOwnPort;
char * defaultDownloadsFolder;
b16 defaultMaxChunkSize;
int defaultSleepSend;
int defaultSleepQueue;

b32 com_transferOrder;
struct sockaddr_in * com_peerAddr;

net_State com_state;
b16 * crcTable;

#endif //P2P_COM_MAIN_H
