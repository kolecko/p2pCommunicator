//
// Created by Matej Kolečáni on 26/09/2019.
//

#ifndef P2P_COM_CHUNKER_H
#define P2P_COM_CHUNKER_H

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "communicator/communicator.h"
#include "queue.h"
#include "net-types.h"

#define CHUNK_ACKED     4
#define CHUNK_UNSENT    3
#define CHUNK_QUEUED    2
#define CHUNK_SENT      1

#define CHUNK_RCVD      0
#define CHUNK_PEND      1

#define CH_UNDEFINED    -1
#define CH_COMPLETED    0

struct ch_ChunksMeta {
    b8 type;
    b32 dataSize;
    b32 chunkCount;
    b16 chunkSize;
    b16 lastChunkSize;
};

int ch_mapInit(struct ch_ChunksMeta * chunksMeta, void ** chunksMap, void * content);

b8 * ch_readChunk(struct ch_ChunksMeta * chunksMeta, void * content, b16 * chunkSize, b32 chunkNumber);
int ch_writeChunk(struct ch_ChunksMeta * chunksMeta, void * container, b32 orderNumber, void * data);

int ch_addUnsentChunks(b8 * chunksMap, b32 mapSize, struct Queue * queue);
int ch_hasEmptyChunks(struct ch_ChunksMeta * chunksMeta, void * map);

b8 * ch_mapParse(struct ch_ChunksMeta * chunksMeta);

#endif //P2P_COM_CHUNKER_H
