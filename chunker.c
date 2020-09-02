//
// Created by Matej Kolečáni on 26/09/2019.
//

#include "chunker.h"

void ch_describeFile(struct ch_ChunksMeta * chunksMeta, FILE * file) {
    b32 fileSize;

    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    chunksMeta->chunkSize = defaultMaxChunkSize;
    chunksMeta->dataSize = fileSize;
    chunksMeta->chunkCount = com_getChunksCount((struct com_Transfer *) chunksMeta, &chunksMeta->lastChunkSize);
}

void ch_describeMessage(struct ch_ChunksMeta * chunksMeta, char * message) {
    b32 messageLength  = strlen(message);

    chunksMeta->chunkSize = defaultMaxChunkSize;
    chunksMeta->dataSize = messageLength;
    chunksMeta->chunkCount = com_getChunksCount((struct com_Transfer *) chunksMeta, &chunksMeta->lastChunkSize);
}

void ch_describeMeta(struct ch_ChunksMeta * chunksMeta, void * meta) {
    b32 metaLength = strlen(meta);

    chunksMeta->chunkSize = defaultMaxChunkSize;
    chunksMeta->dataSize = metaLength;
    chunksMeta->chunkCount = com_getChunksCount((struct com_Transfer *) chunksMeta, &chunksMeta->lastChunkSize);
}

int ch_addUnsentChunks(b8 * chunksMap, b32 mapSize, struct Queue * queue) {
    b32 i;
    int state = 0;

    for (i = 0; i < mapSize && !q_full(queue); i++) {
        if (chunksMap[i] == CHUNK_ACKED) continue;
        chunksMap[i] = CHUNK_QUEUED;
        q_enqueue(queue, i);
        if (!state) state = 1;
    }


    return state;
}

int ch_hasEmptyChunks(struct ch_ChunksMeta * chunksMeta, void * map) {
    b8 * chunksMap = (b8 *) map;
    b32 i;

    for (i = 0; i < chunksMeta->chunkCount; i++) {
        if (chunksMap[i] == CHUNK_PEND) return 1;
    }

    return 0;
}

int ch_writeChunk(struct ch_ChunksMeta * chunksMeta, void * container, b32 chunkNumber, void * data) {
    b16 chunkSize = (chunkNumber == chunksMeta->chunkCount - 1) ? chunksMeta->lastChunkSize : chunksMeta->chunkSize;
    b32 offset = chunkNumber * chunksMeta->chunkSize;
    
    if (chunksMeta->type == MSGHDR_TYPE_MESSAGE) {
        memcpy((b8 *) (container + offset), data, chunkSize);
    } else if (chunksMeta->type == MSGHDR_TYPE_FILE) {
        fseek((FILE *) container, offset, SEEK_SET);
        fwrite(data, 1, chunkSize, (FILE *) container);
    } else if (chunksMeta->type == MSGHDR_TYPE_META) {
        memcpy((b8 *) (container + offset), data, chunkSize);
    } else {
        return COM_UNDEFINED;
    }

    return CH_COMPLETED;
}

b8 * ch_readChunk(struct ch_ChunksMeta * chunksMeta, void * content, b16 * chunkSize, b32 chunkNumber) {
    b8 * buffer;
    b16 bufferSize = (chunkNumber == chunksMeta->chunkCount - 1) ? chunksMeta->lastChunkSize : chunksMeta->chunkSize;
    b32 offset = chunkNumber * chunksMeta->chunkSize;
    
    buffer = malloc(chunksMeta->chunkSize * bufferSize);

    if (chunksMeta->type == MSGHDR_TYPE_MESSAGE) {
        memcpy(buffer, content + offset, bufferSize);
        * chunkSize = bufferSize;
    } else if (chunksMeta->type == MSGHDR_TYPE_FILE) {
        fseek((FILE *) content, offset, SEEK_SET);
        fread((void *) buffer, sizeof(b8), bufferSize, (FILE *) content);
    } else if (chunksMeta->type == MSGHDR_TYPE_META){
        memcpy(buffer, content + offset, bufferSize);
        * chunkSize = bufferSize;
    } else {
        free(buffer);
        return NULL;
    }

    * chunkSize = bufferSize;
    return buffer;
}

int ch_mapInit(struct ch_ChunksMeta * chunksMeta, void ** chunksMap, void * content) {
    if (chunksMeta->type == MSGHDR_TYPE_MESSAGE) {
        ch_describeMessage(chunksMeta, (char *) content);
    } else if (chunksMeta->type == MSGHDR_TYPE_FILE) {
        ch_describeFile(chunksMeta, (FILE *) content);
    } else if (chunksMeta->type == MSGHDR_TYPE_META) {
       ch_describeMeta(chunksMeta, (b8 *) content);
    } else {
        return CH_UNDEFINED;
    }

    *chunksMap = (b8 *) malloc(chunksMeta->chunkCount * sizeof(b8));
    memset(*chunksMap, CHUNK_UNSENT, chunksMeta->chunkCount);
    return CH_COMPLETED;;
}

b8 * ch_mapParse(struct ch_ChunksMeta * chunksMeta) {
    b8 * chunksMap = (b8 *) malloc(chunksMeta->chunkCount * sizeof(b8));
    memset(chunksMap, CHUNK_PEND, chunksMeta->chunkCount);

    return chunksMap;
}