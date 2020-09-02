cmake_minimum_required(VERSION 3.14)
project(p2p_com C)

set(CMAKE_C_STANDARD 99)

add_executable(p2p_com main.c chunker.c chunker.h crc.c crc.h net-types.h queue.c queue.h user-interface.c user-interface.h communicator/communicator.h main.h communicator/connector.c communicator/tools.c communicator/essentials.c communicator/sender.c communicator/flag-receive.c communicator/flag-receive.h communicator/flag-send.c communicator/flag-send.h communicator/chunk-receive.c communicator/chunk-receive.h communicator/chunk-send.c communicator/chunk-send.h communicator/receiver.c communicator/sender.h communicator/receiver.h)