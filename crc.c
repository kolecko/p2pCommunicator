//
// Created by Matej Kolečáni on 29/09/2019.
//
#include "crc.h"

void crc_init() {
    b16 i;
    b16 generator = 0x8408; // 0x1021 reversed
    b16 current;
    b8 bit;
    
    crcTable = malloc(256 * sizeof(b16));
    
    for (i = 0; i < 256; i++) {
        current = (b16) i;
        for (bit = 0; bit < 8; bit++) {
            if (current & 0x1u) {
                current = current >> 1u;
                current = current ^ generator;
                continue;
            }
            current >>= 1u;
        }
        crcTable[i] = current;
    }
}

b16 crc_gen(b8 * buffer, b16 bufferSize) {
    int i;
    b16 crc = 0;
    b8 current, pos;
    
    for (i = bufferSize - 1; i >= 0; i--) {
        current = buffer[i];
        crc = crc ^ current;
        pos = (b8) crc;
        crc = crc >> 8u;
        crc = (b16) (crc ^ (b16) crcTable[pos]);
    }
    
    return crc;
}

b16 crc_reverseBits(b16 number) {
    b16 count = 15u;
    b16 reverse = number;
    
    number >>= 1u;
    
    while(number) {
        reverse <<= 1u;
        reverse |= number & 1u;
        number >>= 1u;
        count--;
    }
    
    reverse <<= count;
    return reverse;
}