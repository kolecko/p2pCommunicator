//
// Created by Matej Kolečáni on 29/09/2019.
//

#ifndef P2P_COM_NET_TYPES_H
#define P2P_COM_NET_TYPES_H

typedef unsigned char b8;
typedef unsigned short b16;
typedef unsigned int b32;

#define PROTO_MIN_PACKET_SIZE   21
#define PROTO_MAX_PACKET_SIZE   1472
#define IP_HEADER  20
#define UDP_HEADER 8

#define PROTO_MIN_CHUNK_SIZE    5
#define PROTO_MAX_CHUNK_SIZE    1462

#define PROTO_PURE_HEADER_SIZE  10
#define PROTO_PURE_META_SIZE    6

#define PROTO_TIMEOUT           3
#define PROTO_RETRY             5

// Merging flags with binary OR, e.g. MSGHDR_FLAG_REQ | MSGHDR_FLAG_ACK
#define MSGHDR_FLAG_REQUEST (unsigned) 0b100000

#define MSGHDR_FLAG_SND     (unsigned) 0b010000

// Used to do anything
#define MSGHDR_FLAG_ACK     (unsigned) 0b001000

// Used to negatively respond to some sequence number.
#define MSGHDR_FLAG_NACK    (unsigned) 0b000100

// Used to signalize ending of connection or with combination.
#define MSGHDR_FLAG_LAST    (unsigned) 0b000010

// If meta flag is sent header is extended by X bits.
#define MSGHDR_FLAG_META    (unsigned) 0b000001

// Might be null.
#define MSGHDR_TYPE_EMPTY   (unsigned) 0b00
#define MSGHDR_TYPE_MESSAGE (unsigned) 0b01
#define MSGHDR_TYPE_FILE    (unsigned) 0b10
#define MSGHDR_TYPE_META    (unsigned) 0b11

typedef b8 net_State;

// Port is assigned to communicating socket, nothing else.
#define PROTO_STATE_WAITING         0

// You addressed connection request to some IP peer and received acknowledge, but you only received ACK message.
#define PROTO_STATE_INITIATED       1

// You've been requested and you answered ack, that you received request. Waiting for user to accept.
#define PROTO_STATE_REQUESTED       2

// You've received very good answer YES.
#define PROTO_STATE_ESTABLISHED     3

// You're sending message or non-trivial data.
#define PROTO_STATE_SENDING         4

// You're receiving message or non-trivial data.
#define PROTO_STATE_RECEIVING       5

// You are ready to terminate connection active way.
#define PROTO_STATE_TERMINATING     8

#pragma pack(push, 1)
struct net_Header {
    b16 checksum: 16;
    b8 type: 2;
    b8 flags: 6;
    b8 reserved: 8;
    b32 transferOrder: 32;
    b16 size: 16;
};

struct net_HeaderMeta {
    b16 metaDataSize;
    b32 dataSize;
};
#pragma pack(pop)

#endif //P2P_COM_NET_TYPES_H
