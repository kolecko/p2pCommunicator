//
// Created by Matej Kolečáni on 18/10/2019.
//
#include "receiver.h"

int callback_receiveMetaChunk (
        struct com_Transfer * transfer,
        b8 * chunksMap,
        void * container,
        struct net_Header * header,
        struct net_HeaderMeta * meta,
        void * buffer,
        int * status
) {
    b32 chunkNumber = com_transferToChunk(transfer, header->transferOrder);

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_SND)) {
        chunksMap[chunkNumber] = CHUNK_RCVD;
        ch_writeChunk((struct ch_ChunksMeta *) transfer, container, chunkNumber, buffer);
        *status = header->transferOrder;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_LAST)) {
        *status = COM_CHUNK_LAST;
        return COM_STOP;
    }

    return COM_NEXT;
}

//TODO:
int callback_receiveRequest (
        struct com_Transfer * transfer,
        b8 * chunksMap,
        struct net_Header * header,
        struct net_HeaderMeta * meta,
        int * status
) {
    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_META | MSGHDR_FLAG_SND)) {
        transfer->type = header->type;
        transfer->from = header->transferOrder + 1;
        transfer->chunkSize = header->size;
        transfer->dataSize = meta->dataSize;
    
        transfer->chunkCount = com_getChunksCount(transfer, &transfer->lastChunkSize);
        transfer->to = transfer->from + transfer->chunkCount;
        *status = meta->metaDataSize;
        return COM_STOP;
    }
    
    if (header->flags == (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK)) {
        *status = COM_CHUNK_ALIVE;
        return COM_NEXT;
    }
    
    return COM_NEXT;
}



int callback_receiveChunk (
        struct com_Transfer * transfer,
        b8 * chunksMap,
        void * container,
        struct net_Header * header,
        struct net_HeaderMeta * meta,
        void * buffer,
        int * status
) {
    if (!com_isInTransferRange(transfer, header->transferOrder)) {
        ui_print("Transfer: Received chunk out of range with transfer order: %d\n", header->transferOrder);
        return COM_NEXT;
    }
    
    b32 chunkNumber = com_transferToChunk(transfer, header->transferOrder);
    
    if (header->flags == MSGHDR_FLAG_SND && !header->checksum) {
        chunksMap[chunkNumber] = CHUNK_RCVD;
    
        ch_writeChunk((struct ch_ChunksMeta *) transfer, container, chunkNumber, buffer);
        *status = COM_CHUNK_OK;
        return COM_NEXT;
    }

    if (header->flags == (MSGHDR_FLAG_SND | MSGHDR_FLAG_LAST) && !header->checksum) {
        *status = COM_CHUNK_LAST;
        return COM_STOP;
    }
    
    if (header->checksum) {
        ui_print("Transfer: Received damaged chunk with number: %d\n", chunkNumber);
        ui_print("Transfer: Chunk count is: %d\n", transfer->chunkCount);
    }
    
    if (header->flags != MSGHDR_FLAG_SND && header->flags != (MSGHDR_FLAG_SND | MSGHDR_FLAG_LAST)) {
        ui_print("Transfer: Received unmatched flags chunk with number: %d\n", chunkNumber);
        ui_print("Transfer: flags: %x\n", header->flags);
    }
    
    *status = COM_CHUNK_WRONG;
    return COM_NEXT;
}



int com_receiveMessage(struct com_Sender * sender, char * message) {
    b32 transferOrder = 0;
    b8 flags = 0;
    int status = 0;
    sender->transferMap = ch_mapParse((struct ch_ChunksMeta *) sender->transfer);

    while (ch_hasEmptyChunks((struct ch_ChunksMeta *) sender->transfer, sender->transferMap)) {
        while (com_chunkReceive(
                sender->transfer,
                sender->transferMap,
                message,
                &status,
                &transferOrder,
                callback_receiveChunk
        ) != COM_STOP) {
            flags = (status == COM_CHUNK_OK) ? (MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK) : (MSGHDR_FLAG_SND | MSGHDR_FLAG_NACK);
            com_flagSend(sender, transferOrder, flags);
        }
    }
    
    com_flagSend(sender, transferOrder, MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK | MSGHDR_FLAG_LAST);

    return COM_RECV;
}



int com_receiveFile(struct com_Sender * sender, FILE * file) {
    b32 transferOrder = 0;
    b8 flags = 0;
    int status = 0;
    sender->transferMap = ch_mapParse((struct ch_ChunksMeta *) sender->transfer);

    while (ch_hasEmptyChunks((struct ch_ChunksMeta *) sender->transfer, sender->transferMap)) {
        while (com_chunkReceive(
                sender->transfer,
                sender->transferMap,
                file,
                &status,
                &transferOrder,
                callback_receiveChunk
        ) != COM_STOP) {
            flags = (status == COM_CHUNK_OK) ? (MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK) : (MSGHDR_FLAG_SND | MSGHDR_FLAG_NACK);
            com_flagSend(sender, transferOrder, flags);
        }
    }

    com_flagSend(sender, transferOrder, MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK | MSGHDR_FLAG_LAST);

    return COM_RECV;
}



int com_receiveMeta(struct com_Sender * sender, char * meta) {
    b32 transferOrder = 0;
    b8 flags = 0;
    int status = 0;
    sender->metaTransferMap = ch_mapParse((struct ch_ChunksMeta *) sender->metaTransfer);

    while (ch_hasEmptyChunks((struct ch_ChunksMeta *) sender->metaTransfer, sender->metaTransferMap)) {
        while (com_chunkReceive(
                sender->metaTransfer,
                sender->metaTransferMap,
                meta,
                &status,
                &transferOrder,
                callback_receiveMetaChunk
        ) != COM_STOP) {
            flags = (status > 0) ? (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK): (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_NACK);
            com_flagSend(sender, transferOrder, flags);
        }
        
    }

    com_flagSend(sender, sender->metaTransfer->to, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_LAST | MSGHDR_FLAG_ACK);
    return COM_RECV;
}



int com_receive(struct com_Sender * sender, char ** message, char ** fileName, FILE ** file) {
    int retry = 0;
    int status;
    char fileNameBuffer[255];

    while (com_flagReceive(sender->transfer,NULL, &status, callback_receiveRequest) != COM_STOP) {
        retry++;
        
        if (status == COM_CHUNK_ALIVE) {
            status = 0;
            retry = 0;
        }
        
        if (retry == COM_RETRY_COUNT / 3 * 2) {
            com_flagSend(sender, com_transferOrder, (MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK));
        }
    
        if (retry == COM_RETRY_COUNT) {
            ui_print("KeepAlive: Disconnecting due to waiting.\n");
            return COM_STOP;
        }
    }

    pthread_mutex_lock(&com_mutex_state);
        if (com_state == PROTO_STATE_ESTABLISHED)
            com_state = PROTO_STATE_RECEIVING;
    pthread_mutex_unlock(&com_mutex_state);

    if (status > 0) {
        assignSenderMeta(sender);
        sender->metaTransfer->type = MSGHDR_TYPE_META;
        sender->metaTransfer->dataSize = status;
        sender->metaTransfer->chunkSize = sender->transfer->chunkSize;
        sender->metaTransfer->chunkCount = com_getChunksCount(sender->metaTransfer, &sender->metaTransfer->lastChunkSize);
        sender->metaTransfer->from = sender->transfer->from;
        sender->metaTransfer->to = sender->metaTransfer->from + sender->metaTransfer->chunkCount;

        sender->transfer->from += sender->metaTransfer->chunkCount + 1;
        sender->transfer->to += sender->metaTransfer->chunkCount + 1;
    }

    if (sender->transfer->type == MSGHDR_TYPE_MESSAGE) {
        *message = (char *) malloc(sender->transfer->dataSize * sizeof(b8));
        sender->transfer->type = MSGHDR_TYPE_MESSAGE;
        
        com_flagSend(sender, sender->transfer->from, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_SND | MSGHDR_FLAG_ACK);
        
        com_receiveMessage(sender, *message);
        
        ui_print("MESSAGE (peer): %s\n", *message);
    } else if (sender->transfer->type == MSGHDR_TYPE_FILE) {
        *fileName = (char *) malloc(sender->metaTransfer->dataSize);
        
        com_flagSend(sender, sender->metaTransfer->from, MSGHDR_FLAG_REQUEST | MSGHDR_FLAG_ACK);
        
        com_receiveMeta(sender, *fileName);

        strcpy(fileNameBuffer, *fileName);

        *fileName = realloc(*fileName, sender->metaTransfer->dataSize + strlen(defaultDownloadsFolder));

        strcpy(*fileName, defaultDownloadsFolder);
        strcat(*fileName, fileNameBuffer);
    
        ui_print("TRANSFER:\n");
        ui_print("Path to file: \t\t%s\n", *fileName);
        ui_print("No. of data chunks: \t%d\n", sender->transfer->chunkCount);
        ui_print("Common chunk size: \t%d\n", sender->transfer->chunkSize);
        ui_print("Last chunk size: \t%d\n", sender->transfer->lastChunkSize);
        ui_print("Data length.: \t\t%d\n", sender->transfer->dataSize);
    
        ui_print("META TRANSFER:\n");
        ui_print("No. of data chunks: \t%d\n", sender->metaTransfer->chunkCount);
        ui_print("Common chunk size: \t%d\n", sender->metaTransfer->chunkSize);
        ui_print("Last chunk size: \t%d\n", sender->metaTransfer->lastChunkSize);
        ui_print("Data length.: \t\t%d\n\n", sender->metaTransfer->dataSize);
        
        *file = fopen(*fileName, "w");

        if (!*file) {
            ui_print("Receiver (E): Couldn't open file.\n", NULL);
            return COM_UNDEFINED;
        }
        
        com_receiveFile(sender, *file);
    
        ui_print("FILE (peer) : received : %s\n", *fileName);
        
        fclose(*file);
    } else {
        return COM_UNDEFINED;
    }

    pthread_mutex_lock(&com_mutex_state);
    com_state = PROTO_STATE_ESTABLISHED;
    pthread_mutex_unlock(&com_mutex_state);
    
    return COM_NEXT;
}

struct cleanup_t_receive_args {
    struct com_Sender * sender;
    char * fileName;
    char * message;
    FILE * file;
};

void cleanup_t_receive(struct cleanup_t_receive_args * args) {
    cleanSender(&args->sender);
    
    if (args->message) {
        free(args->message);
        args->message = NULL;
    }
    
    if (args->file) {
        fclose(args->file);
        args->file = NULL;
    }
    
    if (args->fileName) {
        free(args->fileName);
        args->fileName = NULL;
    }
    
    free(args);
}

void * com_t_receive() {
    int status;
    struct cleanup_t_receive_args * args;
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    
    do {
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        args = (struct cleanup_t_receive_args *) malloc(sizeof(struct cleanup_t_receive_args));
        
        args->sender = com_demandSender();
        args->fileName = NULL;
        args->message = NULL;
        args->file = NULL;
        
        pthread_cleanup_push((void *) cleanup_t_receive, args)
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        
        status = com_receive(args->sender, &args->message, &args->fileName, &args->file);

        pthread_cleanup_pop(1)
    } while (status != COM_STOP);
    
    u_resetPeer();
    pthread_mutex_lock(&com_mutex_state);
    com_state = PROTO_STATE_WAITING;
    pthread_mutex_unlock(&com_mutex_state);
    
    pthread_create(&threads[THREAD_REC], NULL, com_t_awaiting, NULL);
    
    pthread_exit((void *) COM_STOP);
}