//
// Created by Matej Kolečáni on 16/10/2019.
//
#include "communicator.h"
b32 com_chunkToTransfer(struct com_Transfer * transfer, b32 chunkNumber) {
    if (UINT32_MAX - transfer->from < chunkNumber) {
        return chunkNumber - (UINT32_MAX - transfer->from);
    }

    return transfer->from + chunkNumber;
}

b32 com_transferToChunk(struct com_Transfer * transfer, b32 transferOrder) {
    if (transfer->from > transferOrder) {
        return UINT32_MAX - transfer->from + transferOrder;
    }

    return transferOrder - transfer->from;
}

int com_isInTransferRange(struct com_Transfer * transfer, b32 transferOrder) {
    if (transfer->to < transfer->from) {
        return transferOrder >= transfer->from ^ transferOrder <= transfer->to;
    }

    return transferOrder >= transfer->from && transferOrder <= transfer->to;
}

b32 com_getChunksCount(struct com_Transfer * transfer, b16 * rem) {
    b32 chunksCount = transfer->dataSize / transfer->chunkSize;
    *rem = transfer->dataSize % transfer->chunkSize;
    if (*rem) chunksCount++;
    if (!*rem) *rem = transfer->chunkSize;

    return chunksCount;
}