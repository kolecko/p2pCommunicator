//
// Created by Matej Kolečáni on 17/10/2019.
//
#include "sender.h"

struct com_Sender * com_demandSender() {
    struct com_Sender *sender = NULL;
    sender = (struct com_Sender *) malloc(sizeof(struct com_Sender));
    
    sender->transfer = (struct com_Transfer *) malloc(sizeof(struct com_Transfer));
    sender->transfer->test = NULL;

    sender->metaTransfer = NULL;

    sender->transferMap = NULL;
    sender->metaTransferMap = NULL;

    sender->transferQueue = NULL;
    sender->metaTransferQueue = NULL;

    return sender;
}

void assignSenderMeta(struct com_Sender * sender) {
    sender->metaTransfer = (struct com_Transfer *) malloc(sizeof(struct com_Transfer));
    sender->metaTransfer->test = NULL;
    sender->metaTransfer->type = MSGHDR_TYPE_META;
}

void assignTransferTest(struct com_Transfer * transfer) {
    transfer->test = (struct com_Test *) malloc(sizeof(struct com_Test));
}

void cleanSender(struct com_Sender ** sender) {
    if (*sender != NULL) {
        if ((*sender)->transfer != NULL) {
            if ((*sender)->transfer->test != NULL) {
                free((*sender)->transfer->test);
                (*sender)->transfer->test = NULL;
            }
            free((*sender)->transfer);
            (*sender)->transfer = NULL;
        }
        
        if ((*sender)->metaTransfer != NULL) {
            if ((*sender)->metaTransfer->test != NULL) {
                free((*sender)->metaTransfer->test);
                (*sender)->metaTransfer->test = NULL;
            }
    
            free((*sender)->metaTransfer);
            (*sender)->metaTransfer = NULL;
        }
        
        if ((*sender)->transferMap != NULL) {
            free((*sender)->transferMap);
            (*sender)->transferMap = NULL;
        }
        
        if ((*sender)->transferQueue != NULL) {
            if ((*sender)->transferQueue->array != NULL) {
                free((*sender)->transferQueue->array);
                (*sender)->transferQueue->array = NULL;
            }
            free((*sender)->transferQueue);
            (*sender)->transferQueue = NULL;
        }
        
        if ((*sender)->metaTransferMap != NULL) {
            free((*sender)->metaTransferMap);
            (*sender)->metaTransferMap = NULL;
        }
        
        if ((*sender)->metaTransferQueue != NULL) {
            if ((*sender)->metaTransferQueue->array != NULL) {
                free((*sender)->metaTransferQueue->array);
                (*sender)->metaTransferQueue->array = NULL;
            }
            
            free((*sender)->metaTransferQueue);
            (*sender)->metaTransferQueue = NULL;
        }
        
        free(*sender);
        *sender = NULL;
    }
}

int callback_acceptTransfer(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    b32 chunkNumber;
    
    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        if (*status == 0) {
            *status = COM_UNDEFINED;
            return COM_STOP;
        }

        (*status)--;
        return COM_NEXT;
    }

    chunkNumber = com_transferToChunk(transfer, header->transferOrder);

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK) && header->transferOrder == transfer->from) {
        *status = COM_SEND;
        return COM_STOP;
    }

    if (*status == 0) {
        *status = COM_UNDEFINED;
        return COM_STOP;
    }

    (*status)--;
    return COM_NEXT;
}

int callback_acceptMetaTransfer(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    b32 chunkNumber;

    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        if (*status == 0) {
            *status = COM_UNDEFINED;
            return COM_STOP;
        }

        (*status)--;
        return COM_NEXT;
    }

    chunkNumber = com_transferToChunk(transfer, header->transferOrder);

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK) && header->transferOrder == transfer->from) {
        *status = COM_SEND;
        return COM_STOP;
    }

    if (*status == 0) {
        *status = COM_UNDEFINED;
        return COM_STOP;
    }

    (*status)--;
    return COM_NEXT;
}

int callback_acceptChunk(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    b32 chunkNumber;

    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        return COM_NEXT;
    }

    chunkNumber = com_transferToChunk(transfer, header->transferOrder);

    if (header->flags == (MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK)) {
        chunksMap[chunkNumber] = CHUNK_ACKED;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_SND | MSGHDR_FLAG_NACK)) {
        chunksMap[chunkNumber] = CHUNK_UNSENT;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_LAST | MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK)) {
        return COM_STOP;
    }

    return COM_NEXT;
}



int callback_acceptMetaChunk(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    b32 chunkNumber;

    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        return COM_NEXT;
    }

    chunkNumber = com_transferToChunk(transfer, header->transferOrder);

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK)) {
        chunksMap[chunkNumber] = CHUNK_ACKED;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_NACK)) {
        chunksMap[chunkNumber] = CHUNK_UNSENT;
        return COM_NEXT;
    }

    return COM_NEXT;
}



int callback_completeTransfer(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        if (*status == 0) {
            *status = COM_UNDEFINED;
            return COM_STOP;
        }
        (*status)--;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_LAST | MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK) && transfer->to == header->transferOrder) {
        *status = COM_SENT;
        return COM_STOP;
    }

    if (*status == 0) {
        *status = COM_UNDEFINED;
        return COM_STOP;
    }
    (*status)--;
    return COM_NEXT;
}

int callback_completeMetaTransfer(struct com_Transfer * transfer, b8 * chunksMap, struct net_Header * header, struct net_HeaderMeta * meta, int * status) {
    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        if (*status == 0) {
            *status = COM_UNDEFINED;
            return COM_STOP;
        }
        (*status)--;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK | MSGHDR_FLAG_LAST) && header->transferOrder == transfer->to) {
        *status = COM_SEND;
        return COM_STOP;
    }

    if (*status == 0) {
        *status = COM_UNDEFINED;
        return COM_STOP;
    }
    (*status)--;
    return COM_NEXT;
}

int com_commitTransfer(struct com_Sender * sender, void * content) {
    struct com_t_flagReceive_args flagReceiveArgs;
    b32 chunkNumber;
    sender->transferQueue = q_init(sender->transfer->chunkCount);
    
    flagReceiveArgs.transfer = sender->transfer;
    flagReceiveArgs.transferMap = sender->transferMap;
    flagReceiveArgs.callback = callback_acceptChunk;
    
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);
    
    while (ch_addUnsentChunks(sender->transferMap, sender->transfer->chunkCount, sender->transferQueue)) {
        while (!q_empty(sender->transferQueue)) {
            chunkNumber = q_dequeue(sender->transferQueue);
            if (sender->transfer->type == MSGHDR_TYPE_MESSAGE && !(chunkNumber & 1)) {
                sender->transferMap[chunkNumber] = CHUNK_SENT;
                continue;
            }
            if (com_chunkSend(sender->transfer, MSGHDR_FLAG_SND, chunkNumber, content) == COM_SENT) {
                sender->transferMap[chunkNumber] = CHUNK_SENT;
            }
            usleep(defaultSleepSend);
        }
        usleep(defaultSleepQueue);
    }

    pthread_cancel(threads[THREAD_REC]);
    pthread_join(threads[THREAD_REC], NULL);
    return COM_SENT;
}


int com_commitMetaTransfer(struct com_Sender * sender, void * metaContent) {
    struct com_t_flagReceive_args flagReceiveArgs;
    b32 chunkNumber;
    
    sender->metaTransferQueue = q_init(sender->transfer->chunkCount);
    flagReceiveArgs.transfer = sender->metaTransfer;
    flagReceiveArgs.transferMap = sender->metaTransferMap;
    flagReceiveArgs.callback = callback_acceptMetaChunk;
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);
    
    while (ch_addUnsentChunks(sender->metaTransferMap, sender->metaTransfer->chunkCount, sender->metaTransferQueue)) {
        while (!q_empty(sender->metaTransferQueue)) {
            chunkNumber = q_dequeue(sender->metaTransferQueue);

            if (com_chunkSend(sender->metaTransfer, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_SND, chunkNumber, metaContent) == COM_SENT) {
                sender->metaTransferMap[chunkNumber] = CHUNK_SENT;
            }
            
            usleep(defaultSleepSend);
        }
        usleep(defaultSleepQueue);
    }

    pthread_cancel(threads[THREAD_REC]);
    pthread_join(threads[THREAD_REC], NULL);

    return COM_SENT;
}

int com_sendMessage(struct com_Sender * sender, char * message)  {
    struct com_t_flagReceive_args flagReceiveArgs;
    
    sender->transfer->type = MSGHDR_TYPE_MESSAGE;
    sender->transfer->chunkSize = defaultMaxChunkSize;
    
    ch_mapInit((struct ch_ChunksMeta *) sender->transfer, (void **) &sender->transferMap, message);
    
    sender->transfer->from = com_transferOrder + 1;
    sender->transfer->to = sender->transfer->from + sender->transfer->chunkCount;
    
    flagReceiveArgs.transfer = sender->transfer;
    flagReceiveArgs.transferMap = sender->transferMap;
    flagReceiveArgs.status = COM_RETRY_COUNT;
    flagReceiveArgs.callback = callback_acceptTransfer;
    
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);

    com_flagSend(sender, com_transferOrder, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_META | MSGHDR_FLAG_SND);

    pthread_join(threads[THREAD_REC], NULL);

    if (flagReceiveArgs.status != COM_SEND) {
        return COM_UNDEFINED;
    }

    if (com_commitTransfer(sender, message) != COM_SENT) {
        return COM_UNDEFINED;
    }

    flagReceiveArgs.transfer = sender->transfer;
    flagReceiveArgs.transferMap = sender->transferMap;
    flagReceiveArgs.status = COM_RETRY_COUNT;
    flagReceiveArgs.callback = callback_completeTransfer;
    
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);

    com_flagSend(sender, sender->transfer->to, MSGHDR_FLAG_LAST | MSGHDR_FLAG_SND);

    pthread_join(threads[THREAD_REC], NULL);
    
    ui_print("Message: %s\n", message);
    
    if (flagReceiveArgs.status != COM_SENT) {
        return COM_UNDEFINED;
    }

    return COM_SENT;
}



int com_sendFile(struct com_Sender * sender, char * filePath)  {
    struct com_t_flagReceive_args flagReceiveArgs;
    char * fileName;
    FILE * file = fopen(filePath, "r");
    if (!file) {
        return COM_UNDEFINED;
    }

    sender->transfer->type = MSGHDR_TYPE_FILE;
    sender->transfer->chunkSize = defaultMaxChunkSize;

    ch_mapInit((struct ch_ChunksMeta *) sender->transfer, (void **) &sender->transferMap, file);

    fileName = basename(filePath);
    assignSenderMeta(sender);

    sender->metaTransfer->type = MSGHDR_TYPE_META;
    ch_mapInit((struct ch_ChunksMeta *) sender->metaTransfer, (void **) &sender->metaTransferMap, fileName);
    
    sender->metaTransfer->from = com_transferOrder + 1;
    sender->metaTransfer->to = sender->metaTransfer->from + sender->metaTransfer->chunkCount;
    sender->transfer->from = sender->metaTransfer->to + 1;
    sender->transfer->to = sender->transfer->from + sender->transfer->chunkCount;
    
    ui_print("TRANSFER:\n");
    ui_print("Path to file: \t\t%s\n", filePath);
    ui_print("No. of data chunks: \t%d\n", sender->transfer->chunkCount);
    ui_print("Common chunk size: \t%d\n", sender->transfer->chunkSize);
    ui_print("Last chunk size: \t%d\n", sender->transfer->lastChunkSize);
    ui_print("Data length.: \t\t%d\n", sender->transfer->dataSize);
    
    ui_print("META TRANSFER:\n");
    ui_print("No. of data chunks: \t%d\n", sender->metaTransfer->chunkCount);
    ui_print("Common chunk size: \t%d\n", sender->metaTransfer->chunkSize);
    ui_print("Last chunk size: \t%d\n", sender->metaTransfer->lastChunkSize);
    ui_print("Data length.: \t\t%d\n\n", sender->metaTransfer->dataSize);
    

    flagReceiveArgs.transfer = sender->metaTransfer;
    flagReceiveArgs.transferMap = sender->metaTransferMap;
    flagReceiveArgs.status = COM_RETRY_COUNT;
    flagReceiveArgs.callback = callback_acceptMetaTransfer;
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);

    com_flagSend(sender, com_transferOrder, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_META | MSGHDR_FLAG_SND);

    pthread_join(threads[THREAD_REC], NULL);

    if (flagReceiveArgs.status != COM_SEND) {
        return COM_UNDEFINED;
    }

    if (com_commitMetaTransfer(sender, fileName) != COM_SENT) {
        return COM_UNDEFINED;
    }

    flagReceiveArgs.transfer = sender->metaTransfer;
    flagReceiveArgs.transferMap = sender->metaTransferMap;
    flagReceiveArgs.status = COM_RETRY_COUNT;
    flagReceiveArgs.callback = callback_completeMetaTransfer;;
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);

    com_flagSend(sender, sender->metaTransfer->to, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_LAST);

    pthread_join(threads[THREAD_REC], NULL);
    
    if (flagReceiveArgs.status != COM_SEND) {
        return COM_UNDEFINED;
    }
    
    if (com_commitTransfer(sender, file) != COM_SENT) {
        return COM_UNDEFINED;
    }

    flagReceiveArgs.transfer = sender->transfer;
    flagReceiveArgs.transferMap = sender->transferMap;
    flagReceiveArgs.status = COM_RETRY_COUNT;
    flagReceiveArgs.callback = callback_completeTransfer;
    pthread_create(&threads[THREAD_REC], NULL, (void *) com_t_flagReceive, &flagReceiveArgs);

    com_flagSend(sender, sender->transfer->to, MSGHDR_FLAG_SND | MSGHDR_FLAG_LAST);

    pthread_join(threads[THREAD_REC], NULL);

    if (flagReceiveArgs.status != COM_SENT) {
        return COM_UNDEFINED;
    }
    
    ui_print("FILE (me) : sent : %s\n", filePath);

    return COM_SENT;
}

int com_send(char * message, char * filePath, long long * damagingMap, b32 damagingMapSize) {
    int state;
    struct com_Sender * sender = com_demandSender();
    
    pthread_mutex_lock(&com_mutex_state);
    if (com_state == PROTO_STATE_ESTABLISHED) {
        com_state = PROTO_STATE_SENDING;
    }
    pthread_mutex_unlock(&com_mutex_state);
    
    pthread_cancel(threads[THREAD_REC]);
    pthread_join(threads[THREAD_REC], NULL);
    
    if (damagingMap) {
        assignTransferTest(sender->transfer);
        sender->transfer->test->damagingTransferMap = damagingMap;
        sender->transfer->test->damagingTransferMapSize = damagingMapSize;
    }

    if (filePath) {
        state = com_sendFile(sender, filePath);
    } else if (message) {
        state = com_sendMessage(sender, message);
    } else {
        state = COM_UNDEFINED;
    }

    pthread_mutex_lock(&com_mutex_state);
    com_state = PROTO_STATE_ESTABLISHED;
    pthread_mutex_unlock(&com_mutex_state);
    
    cleanSender(&sender);
    
    pthread_create(&threads[THREAD_REC], 0, com_t_receive, NULL);

    return state;
}