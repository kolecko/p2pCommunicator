//
// Created by Matej Kolečáni on 16/10/2019.
//
#include "communicator.h"

int u_assignSocket(b16 port) {
    struct timeval to;
    
    if ((com_socket = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Couldn't assign socket.");
        return com_socket;
    }
    
    to.tv_sec = PROTO_TIMEOUT;
    to.tv_usec = 0;
    setsockopt(com_socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *) &to, sizeof(struct timeval));

    com_ownAddr.sin_family = AF_INET;
    com_ownAddr.sin_addr.s_addr = INADDR_ANY;
    com_ownAddr.sin_port = htons(port);

    if (bind(com_socket, (const struct sockaddr *) &com_ownAddr, sizeof(com_ownAddr)) < 0) {
        perror("Couldn't bind address to socket.");
        return -2;
    }

    return com_socket;
}

int u_setPeer(char * ip, char * port) {
    b16 tempPort;
    char * endptr;

    com_peerAddr->sin_family = AF_INET;
    tempPort = (b16) strtol(port, &endptr, 10);

    if (inet_pton(AF_INET, ip, &(com_peerAddr->sin_addr)) == -1) {
        perror("Thor became so angry, you forked up IP");
        return ERR_IP_INVALID;
    }

    if (endptr[0] != '\0') {
        perror("Thor is angry, you wrote down wrong port.");
        return ERR_PORT_INVALID;
    }

    com_peerAddr->sin_port = htons(tempPort);

    if (debug) {
        ui_print("CONNECTING TO PORT: %hu\n", ntohs(com_peerAddr->sin_port));
    }

    return 0;
}

void u_resetPeer() {
    free(com_peerAddr);
    com_peerAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    com_transferOrder = 0;
}