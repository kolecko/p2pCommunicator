//
// Created by Matej Kolečáni on 17/10/2019.
//
#include "flag-receive.h"

b8 com_flagReceive (
    struct com_Transfer * transfer,
    b8 * transferMap,
    int * status,
    int (* callback)(struct com_Transfer *, b8 *, struct net_Header *, struct net_HeaderMeta *, int *)
) {
    int length = 0;
    struct net_Header header;
    struct net_HeaderMeta meta;
    b8 buffer[PROTO_MAX_PACKET_SIZE];
    b16 crc;
    memset(buffer, 0, PROTO_MAX_PACKET_SIZE);
    
    length = recvfrom(com_socket,
      buffer,
      PROTO_MAX_PACKET_SIZE,
      MSG_WAITALL,
      (struct sockaddr *) com_peerAddr,
      &com_peerAddr_size
    );
    
    if (length < 0) {
        return length;
    }
    
    crc = crc_gen(buffer, length);
    memcpy(&header, buffer, sizeof(struct net_Header));
    
    header.checksum = crc;
    header.size = ntohs(header.size);
    header.transferOrder = ntohl(header.transferOrder);
    header.checksum = ntohs(header.checksum);
    
    if (header.flags & MSGHDR_TYPE_META) {
        memcpy(&meta, buffer + sizeof(struct net_Header), sizeof(struct net_HeaderMeta));
        meta.dataSize = ntohl(meta.dataSize);
        meta.metaDataSize = ntohs(meta.metaDataSize);
        return callback(transfer, transferMap, &header, &meta, status);
    }

    return callback(transfer, transferMap, &header, NULL, status);
}

void * com_t_flagReceive(struct com_t_flagReceive_args * args) {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (com_flagReceive(args->transfer, args->transferMap, &args->status, args->callback) != COM_STOP);
    return NULL;
}