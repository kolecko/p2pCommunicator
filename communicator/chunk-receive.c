//
// Created by Matej Kolečáni on 18/10/2019.
//

#include "chunk-receive.h"

b8 com_chunkReceive (
    struct com_Transfer * transfer,
    b8 * transferMap,
    void * container,
    int * status,
    b32 * transferOrder,
    int (* callback)(struct com_Transfer *, b8 *, void *, struct net_Header *, struct net_HeaderMeta *, void *,  int *)
) {
    struct net_Header header;
    struct net_HeaderMeta meta;
    b8 buffer[PROTO_MAX_PACKET_SIZE];
    b16 crc, length, offset = 0;
    memset(buffer, 0, PROTO_MAX_PACKET_SIZE);

    length = recvfrom(
            com_socket,
            buffer,
            PROTO_MAX_PACKET_SIZE,
            MSG_WAITALL,
            (struct sockaddr *) com_peerAddr,
            &com_peerAddr_size
    );
    
    crc = crc_gen(buffer, length);
    memcpy(&header, buffer, sizeof(struct net_Header));
    
    header.checksum = crc;
    
    offset += sizeof(struct net_Header);
    header.size = ntohs(header.size);
    header.transferOrder = ntohl(header.transferOrder);
    header.checksum = ntohs(header.checksum);

    *transferOrder = header.transferOrder;

    if (header.flags & MSGHDR_FLAG_META) {
        offset += sizeof(struct net_HeaderMeta);
        memcpy(&meta, buffer + sizeof(struct net_Header), sizeof(struct net_HeaderMeta));
        meta.dataSize = ntohl(meta.dataSize);
        meta.metaDataSize = ntohs(meta.metaDataSize);

        return callback(transfer, transferMap, container, &header, &meta, buffer + offset, status);
    }

    return callback(transfer, transferMap, container, &header, NULL, buffer + offset, status);
}

void * com_t_chunkReceive(struct com_t_chunkReceive_args * args) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (com_chunkReceive(args->transfer, args->transferMap, args->container, &args->status, args->transferOrder, args->callback) != COM_STOP);
    return NULL;
}