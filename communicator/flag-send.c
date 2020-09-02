//
// Created by Matej Kolečáni on 17/10/2019.
//

#include "flag-send.h"

int com_flagSend(struct com_Sender * sender, b32 transferOrder, b8 flags) {
    struct net_Header header;
    struct net_HeaderMeta meta;
    b8 buffer[PROTO_MAX_PACKET_SIZE];
    b16 bufferSize = sizeof(struct net_Header), crc;

    memset(&header, 0, PROTO_PURE_HEADER_SIZE);

    header.flags = flags;
    header.type = sender->transfer->type;
    header.transferOrder = htonl(transferOrder);
    header.size = htons(sender->transfer->chunkSize);

    if (flags & MSGHDR_FLAG_META) {
        memset(&meta, 0, PROTO_PURE_META_SIZE);
        bufferSize += sizeof(struct net_HeaderMeta);
        
        meta.metaDataSize = (sender->metaTransfer) ? htons(sender->metaTransfer->dataSize): 0;
        
        meta.dataSize = htonl(sender->transfer->dataSize);
        memcpy(buffer + sizeof(struct net_Header), &meta, sizeof(struct net_HeaderMeta));
    } else if (flags & MSGHDR_FLAG_REQUEST) {
        header.type = MSGHDR_TYPE_META;
    }

    memcpy(buffer, &header, sizeof(struct net_Header));
    
    crc = crc_gen(buffer + 2, bufferSize - 2);
    crc = htons(crc);
    
    memcpy(buffer, &crc, sizeof(b16));

    if (sendto(com_socket, buffer, bufferSize, 0, (struct sockaddr *) com_peerAddr, com_peerAddr_size) < 0) {
        perror("Communicator: Flags can't be sent.");
        return COM_UNDEFINED;
    }

    return COM_SENT;
}