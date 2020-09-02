//
// Created by Matej Kolečáni on 05/10/2019.
//

#ifndef P2P_COM_COMMUNICATOR_H
#define P2P_COM_COMMUNICATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/time.h>


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>

#include "../main.h"
#include "../net-types.h"
#include "../user-interface.h"
#include "../queue.h"
#include "../chunker.h"
#include "../crc.h"

#include "flag-send.h"
#include "flag-receive.h"
#include "chunk-send.h"
#include "chunk-receive.h"
#include "receiver.h"
#include "sender.h"

#define ERR_PORT_INVALID    -1
#define ERR_IP_INVALID      -2
#define ERR_F_UNAVAILABLE   -3

#define COM_UNDEFINED       -3
#define COM_NOT_ESTABLISHED -2
#define COM_TIMEOUT         -1
#define COM_ESTABLISHED      0

#define COM_RECV             3
#define COM_SEND             2
#define COM_NEXT             1
#define COM_STOP             0

#define COM_CHUNK_LAST       0
#define COM_CHUNK_OK         1
#define COM_CHUNK_WRONG      2
#define COM_CHUNK_NONE       3
#define COM_CHUNK_ALIVE      4

#define COM_SENT             1
#define COM_RETRY_COUNT      PROTO_RETRY

pthread_t threads[3];

struct com_Test {
    long long * damagingTransferMap;
    b32 damagingTransferMapSize;
};

struct com_Transfer {
    b8 type;
    b32 dataSize;
    b32 chunkCount;
    b16 chunkSize;
    b16 lastChunkSize;
    b32 from;
    b32 to;
    struct com_Test * test;
};

int u_assignSocket(b16 port);
int u_setPeer(char * ip, char * port);
void u_resetPeer();

void * com_t_awaiting();

int com_socket;
extern b32 com_transferOrder;

b16 * crcTable;

extern net_State com_state;
static pthread_mutex_t com_mutex_state = PTHREAD_MUTEX_INITIALIZER;

struct sockaddr_in com_ownAddr;

extern struct sockaddr_in * com_peerAddr;
static socklen_t com_peerAddr_size = sizeof(struct sockaddr_in);

b32 com_chunkToTransfer(struct com_Transfer * transfer, b32 chunkNumber);
b32 com_transferToChunk(struct com_Transfer * transfer, b32 transferOrder);
int com_isInTransferRange(struct com_Transfer * transfer, b32 transferOrder);
b32 com_getChunksCount(struct com_Transfer * transfer, b16 * rem);

int u_sendConnectionRequest();
int u_sendConnectionAcknowledge();


b8 debug;
#endif //P2P_COM_COMMUNICATOR_H
