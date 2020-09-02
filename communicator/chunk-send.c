//
// Created by Matej Kolečáni on 18/10/2019.
//

#include "chunk-send.h"

int com_chunkSend(struct com_Transfer * transfer, b8 flags, b32 chunkNumber, void * container) {
    struct net_Header header;
    b8 buffer[PROTO_MAX_PACKET_SIZE];
    b32 transferOrder = com_chunkToTransfer(transfer, chunkNumber), i;
    b16 chunkSize, bufferSize, crc;
    
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    void * chunk = ch_readChunk((struct ch_ChunksMeta *) transfer, container, &chunkSize, chunkNumber);

    bufferSize = PROTO_PURE_HEADER_SIZE + chunkSize;
    memset(&header, 0, sizeof(struct net_Header));

    header.flags = flags;
    header.type = transfer->type;
    header.transferOrder = htonl(transferOrder);
    header.size = htons(chunkSize);

    memcpy(buffer, &header, sizeof(struct net_Header));
    memcpy(buffer + sizeof(struct net_Header), chunk, chunkSize);
    
    free(chunk);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    
    crc = crc_gen(buffer + 2, bufferSize - 2);
    crc = htons(crc);
    
    if (transfer->test) {
        for (i = 0; i < transfer->test->damagingTransferMapSize; i++) {
            if (chunkNumber != transfer->test->damagingTransferMap[i]) continue;
            
            ui_print("Test: Intentionally damaging CRC for chunk with number: %d\n", chunkNumber);
            crc = (b16) (((b32) crc + 1u) % UINT16_MAX);
            transfer->test->damagingTransferMap[i] = -1;
        }
    }
    
    memcpy(buffer, &crc, sizeof(b16));

    if (sendto(com_socket, buffer, bufferSize, 0, (struct sockaddr *) com_peerAddr, com_peerAddr_size) < 0) {
        ui_print("Communicator: (E) Can't send chunk.\n", NULL);
        return COM_STOP;
    }

    return COM_SENT;
}
