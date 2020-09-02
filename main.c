#include "main.h"

void * t_ui() {
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ui_command_i();
    pthread_cancel(threads[THREAD_REC]);
    return NULL;
}

int main(int argc, char * argv[]) {
    int i, state;
    com_peerAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    com_transferOrder = 0;
    
    ui_parseargs(argc, argv);

    defaultOwnPort = (!defaultOwnPort) ? 60000: defaultOwnPort;
    defaultDownloadsFolder = (!defaultDownloadsFolder) ? "downloads/" : defaultDownloadsFolder;
    defaultMaxChunkSize = (!defaultMaxChunkSize) ? PROTO_MAX_CHUNK_SIZE : defaultMaxChunkSize;
    defaultSleepSend = (!defaultSleepSend) ? 40: defaultSleepSend;
    defaultSleepQueue = (!defaultSleepQueue) ? 100: defaultSleepQueue;
    
    crc_init();

    if (u_assignSocket(defaultOwnPort) < 0) {
        return -1;
    }

    com_state = PROTO_STATE_WAITING;
    
    pthread_create(&threads[THREAD_REC], NULL, com_t_awaiting, NULL);
    pthread_detach(threads[THREAD_REC]);

    pthread_create(&threads[THREAD_UI], NULL, t_ui, NULL);

    for (i = 0; i < THREAD_COUNT; i++) {
        if ((state = pthread_join(threads[i], NULL)) < 0) {
            perror("p2p_com : Couldn't join thread. :");
            return state;
        }
    }

    return 0;
}