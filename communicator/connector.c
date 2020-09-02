//
// Created by Matej Kolečáni on 16/10/2019.
//
#include "communicator.h"
#include <errno.h>

int u_sendConnectionRequest() {
    struct net_Header header;
    b16 crc;
    memset(&header, 0, sizeof(struct net_Header));

    header.flags = MSGHDR_FLAG_REQUEST;
    header.type = MSGHDR_TYPE_EMPTY;
    header.transferOrder = htonl(0);
    
    crc = crc_gen((b8 *) &header + 2, PROTO_PURE_HEADER_SIZE - 2);
    crc = htons(crc);
    memcpy(&header, &crc, 2);

    pthread_mutex_lock(&com_mutex_state);
    com_state = PROTO_STATE_INITIATED;
    pthread_mutex_unlock(&com_mutex_state);

    if (sendto(com_socket, &header, sizeof(struct net_Header), 0, (struct sockaddr *) com_peerAddr, com_peerAddr_size) < 0) {
        ui_print("Communicator: (E) Can't send connection request. : %d\n", errno);
        return COM_STOP;
    }

    ui_print("Connector: Connection request sent.\n");
    return COM_SENT;
}


int u_sendConnectionAcknowledge() {
    struct net_Header header;
    b16 crc;
    memset(&header, 0, sizeof(struct net_Header));

    header.flags = MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK;
    header.type = 0;
    header.transferOrder = 0;
    header.size = 0;
    
    crc = crc_gen((b8 *) &header + 2, PROTO_PURE_HEADER_SIZE - 2);
    crc = htons(crc);
    memcpy(&header, &crc, 2);

    if (sendto(com_socket, &header, sizeof(struct net_Header), 0, (struct sockaddr *) com_peerAddr, com_peerAddr_size) < 0) {
        ui_print("Communicator: (E) Can't send transfer request.\n", NULL);
        return COM_STOP;
    }

    ui_print("Connector: Connection acknowledged.\n", NULL);
    return COM_SENT;
}

int com_awaiting() {
    int retry = 0;
    struct net_Header header;
    int length;
    b16 crc;
    
    memset(&header, 0, sizeof(struct net_Header));
    
    memset(com_peerAddr, 0, sizeof(struct sockaddr_in));
    
    while (recvfrom(
            com_socket,
            &header,
            sizeof(struct net_Header),
            MSG_WAITALL,
            (struct sockaddr *) com_peerAddr,
            &com_peerAddr_size) == -1
    ) {
        pthread_mutex_lock(&com_mutex_state);
        if (com_state == PROTO_STATE_INITIATED) {
            ui_print("Connect: Connecting...\n");
            retry++;
            
            if (retry == COM_RETRY_COUNT) {
                ui_print("Connect: Unreachable peer..\n");
                com_state = PROTO_STATE_WAITING;
                u_resetPeer();
                retry = 0;
            }
            
            pthread_mutex_unlock(&com_mutex_state);
            continue;
        }
        
        pthread_mutex_unlock(&com_mutex_state);
        retry = 0;
    }
    
    crc = crc_gen((b8 *) &header, PROTO_PURE_HEADER_SIZE);
    header.checksum = crc;
    
    if (crc) {
        return COM_NOT_ESTABLISHED;
    }
    
    pthread_mutex_lock(&com_mutex_state);
    
    if (
        header.flags == (MSGHDR_FLAG_ACK | MSGHDR_FLAG_REQUEST) &&
        com_state == PROTO_STATE_INITIATED
    ) {
        com_state = PROTO_STATE_ESTABLISHED;
        pthread_mutex_unlock(&com_mutex_state);
        ui_print("Connect: Connection has been established.\n");
        return COM_ESTABLISHED;
    }

    if (
        header.flags == MSGHDR_FLAG_REQUEST &&
        (com_state == PROTO_STATE_WAITING || com_state == PROTO_STATE_INITIATED)
    ) {
        com_state = PROTO_STATE_REQUESTED;

        if (u_sendConnectionAcknowledge() < 0) {
            pthread_mutex_unlock(&com_mutex_state);
            return COM_NOT_ESTABLISHED;
        }

        com_state = PROTO_STATE_ESTABLISHED;
        pthread_mutex_unlock(&com_mutex_state);
        ui_print("Connect: Connection has been established.\n");
        return COM_ESTABLISHED;
    }

    pthread_mutex_unlock(&com_mutex_state);
    return COM_NOT_ESTABLISHED;
}

void *com_t_awaiting() {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    while (com_awaiting() != COM_ESTABLISHED);

    pthread_create(&threads[THREAD_REC], 0, com_t_receive, NULL);
    return NULL;
}
