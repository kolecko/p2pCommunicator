//
// Created by Matej Kolečáni on 05/10/2019.
//

#ifndef P2P_COM_USER_INTERFACE_H
#define P2P_COM_USER_INTERFACE_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <dirent.h>
#include <errno.h>

#include <pthread.h>

#include <termios.h>
#include <sys/ioctl.h>

#include "net-types.h"
#include "communicator/communicator.h"

#define ERR_INVALID_ARG     -1
#define ERR_INVALID_PARAM   -2
#define ERR_MIS_SYNTAX      -3
#define ERR_CMD_UNAVAILABLE -4

static char * pre_command = "msg - enter message; file - send file: ";
static pthread_mutex_t stdio_lock = PTHREAD_MUTEX_INITIALIZER;

extern b16 defaultOwnPort;
extern char * defaultDownloadsFolder;
extern b16 defaultMaxChunkSize;
extern int defaultSleepSend;
extern int defaultSleepQueue;
extern b8 debug;

int ui_parseargs(int argc, char * argv[]);

void ui_write(char * msg_buffer, b16 msg_buffer_length);
void ui_print(const char * text, ...);

void * ui_command_i();
void * ui_connected_i();

#endif //P2P_COM_USER_INTERFACE_H
