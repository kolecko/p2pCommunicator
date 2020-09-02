//
// Created by Matej Kolečáni on 29/09/2019.
//

#ifndef P2P_COM_CRC_H
#define P2P_COM_CRC_H

#include <stdlib.h>

#include "net-types.h"

extern b16 * crcTable;

void crc_init();
b16 crc_gen(b8 * buffer, b16 bufferSize);
b16 crc_reverseBits(b16 number);

#endif //P2P_COM_CRC_H
