//
// Created by Matej Kolečáni on 05/10/2019.
//

#include "user-interface.h"

void ui_print(const char * text, ...) {
    va_list argList;
    char ctrl_down[] = "\033[K";
    char ctrl_group[] = "\033[1A\r\033[K";
    struct termios * tc;
    tcflag_t tc_t_flag;

    pthread_mutex_lock(&stdio_lock);
    tc = (struct termios *) malloc(sizeof(struct termios));
    write(STDOUT_FILENO, ctrl_group, sizeof(ctrl_group) - 1);
    va_start(argList, text);
    vfprintf(stdout, text, argList);
    va_end(argList);
    fflush(stdout);
    write(STDOUT_FILENO, ctrl_down, sizeof(ctrl_down) - 1);
    fprintf(stdout, "%s", pre_command);
    fflush(stdout);

    tcgetattr(STDOUT_FILENO, tc);

    tc_t_flag = tc->c_lflag;

    tc->c_lflag &= ~((unsigned) ECHOCTL);
    tcsetattr(STDOUT_FILENO, TCSANOW, tc);

    ioctl(STDOUT_FILENO, TIOCSTI, &tc->c_cc[VREPRINT]);

    tc->c_lflag = tc_t_flag;
    tcsetattr(STDOUT_FILENO, TCSANOW, tc);

    free(tc);
    pthread_mutex_unlock(&stdio_lock);
}

int ui_parseargs(int argc, char * argv[]) {
    char ** endptr;
    DIR * dir;
    b16 port;
    int temp;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--port") == 0) {
            i = i + 1;

            if (i == argc) {
                perror("p2p_com: Invalid parameter for port argument.");
                return ERR_INVALID_PARAM;
            }

            endptr = (char **) malloc(sizeof(char *));
            port = (b16) strtol(argv[i], endptr, 10);

            if (** endptr != '\0') {
                perror("p2p_com: Invalid parameter ");
                printf("usage: p2p_com [--port port] [--downloads download_folder] [--ss sleep send] [--sq sleep queue]\n"
                       "\tport - port for communication (default: 60000) <0; 65635>\n"
                       "\tdownloads - directory where to save received files"
                       "\tss - sleep after send in usec"
                       "\tsq - sleep after finished queue in usec");
                return ERR_INVALID_PARAM;
            }

            defaultOwnPort = port;
        } else if (strcmp(argv[i], "--ss") == 0) {
            i = i + 1;
    
            if (i == argc) {
                perror("p2p_com: Invalid parameter for ss argument.");
                return ERR_INVALID_PARAM;
            }
    
            endptr = (char **) malloc(sizeof(char *));
            temp = (b16) strtol(argv[i], endptr, 10);
    
            if (** endptr != '\0') {
                perror("p2p_com: Invalid parameter ");
                printf("usage: p2p_com [--port port] [--downloads download_folder] [--ss sleep send] [--sq sleep queue]\n"
                       "\tport - port for communication (default: 60000) <0; 65635>\n"
                       "\tdownloads - directory where to save received files");
                return ERR_INVALID_PARAM;
            }
    
            defaultSleepSend = temp;
        } else if (strcmp(argv[i], "--sq") == 0) {
            i = i + 1;
    
            if (i == argc) {
                perror("p2p_com: Invalid parameter for ss argument.");
                return ERR_INVALID_PARAM;
            }
    
            endptr = (char **) malloc(sizeof(char *));
            temp = (b16) strtol(argv[i], endptr, 10);
    
            if (** endptr != '\0') {
                perror("p2p_com: Invalid parameter ");
                printf("usage: p2p_com [--port port] [--downloads download_folder] [--ss sleep send] [--sq sleep queue]\n"
                       "\tport - port for communication (default: 60000) <0; 65635>\n"
                       "\tdownloads - directory where to save received files");
                return ERR_INVALID_PARAM;
            }
    
            defaultSleepQueue = temp;
        } else if (strcmp(argv[i], "--downloads") == 0) {
            i = i + 1;

            if (i == argc) {
                perror("p2p_com: File does not exist, parsed as parameter for download argument.");
                return ERR_INVALID_PARAM;
            }

            dir = opendir(argv[i]);

            if (errno == ENOENT) {
                perror("p2p_com: File does not exist, parsed as parameter for download argument.");
                return ERR_INVALID_PARAM;
            } else if (errno == EPERM) {
                perror("p2p_com: You are not permitted to open such a directory, "
                       "parsed as parameter for download argument.");
                return ERR_INVALID_PARAM;
            }

            closedir(dir);
            defaultDownloadsFolder = argv[i];

            printf("FOLDER: %s\n", defaultDownloadsFolder);
        } else {
            perror("p2p_com: Invalid argument.");
            return ERR_INVALID_ARG;
        }
    }
    
    return 0;
}

void * ui_command_i() {
    char * line = NULL;
    char * cmd = NULL;
    char ** args = NULL;
    long long * damagingMap = NULL;
    int dmSize = 0;
    int temp = 0;
    int i;
    struct sockaddr_in * peerAddr = (struct sockaddr_in *) malloc(sizeof(struct sockaddr_in));
    memset(peerAddr, 0, sizeof(struct sockaddr_in));

    size_t line_len = 128;

    for(;;) {
        pthread_mutex_lock(&stdio_lock);

        fprintf(stdout, "%s\n", pre_command);
        fflush(stdout);

        pthread_mutex_unlock(&stdio_lock);
        
        line = fgetln(stdin, &line_len);

        if (line == NULL) {
            break;
        }

        if (line_len == 0) {
            continue;
        }

        line[line_len - 1] = 0;
        line[strcspn(line, "\n\r")] = 0;

        if ((cmd = strsep(&line, " ")) < 0) {
            perror("Thor is angry. You don't know syntax");
            return (void *) ERR_MIS_SYNTAX;
        }

        if (strcmp("connect", cmd) == 0) {
            args = (char **) malloc(2 * sizeof(char *));

            if ((args[0] = strsep(&line, " ")) < 0) {
                free(args);
                ui_print("Connect: destination ip not valid");
                return (void *) ERR_MIS_SYNTAX;
            }

            if ((args[1] = strsep(&line, " ")) < 0) {
                free(args);
                ui_print("Connect: destination port not valid.\n");
                continue;
            }
            
            if (args[1] == NULL) {
                free(args);
                ui_print("Connect: Please write down port.\n");
                continue;
            }

            u_setPeer(args[0], args[1]);
    
            pthread_mutex_lock(&com_mutex_state);
            if (com_state != PROTO_STATE_WAITING) {
                pthread_mutex_unlock(&com_mutex_state);
                free(args);
                ui_print("Connect: connection is already being established or already is established");
                return (void *) ERR_CMD_UNAVAILABLE;
            }
            pthread_mutex_unlock(&com_mutex_state);
    
            u_sendConnectionRequest(peerAddr);
        } else if (strcmp("msg", cmd) == 0) {
            if (strlen(line) == 0) {
                ui_print("Send: Message has no content.\n");
                return (void *) ERR_MIS_SYNTAX;
            }
    
            pthread_mutex_lock(&com_mutex_state);
            if (com_state != PROTO_STATE_ESTABLISHED) {
                pthread_mutex_unlock(&com_mutex_state);
                ui_print("Send: Connection is not established.\n");
                return (void *) ERR_CMD_UNAVAILABLE;
            }
            pthread_mutex_unlock(&com_mutex_state);
    
            com_send(line, NULL, damagingMap, dmSize);
            memset(line, 0, strlen(line));
        } else if (strcmp("file", cmd) == 0) {
            if (strlen(line) == 0)  {
                ui_print("Send: No file.\n");
                return (void *) ERR_MIS_SYNTAX;
            }
    
            pthread_mutex_lock(&com_mutex_state);
            if (com_state != PROTO_STATE_ESTABLISHED) {
                pthread_mutex_unlock(&com_mutex_state);
                ui_print("Send: Connection is not established.\n");
                return (void *) ERR_CMD_UNAVAILABLE;
            }
            pthread_mutex_unlock(&com_mutex_state);
    
            com_send(NULL, line, damagingMap, dmSize);
            memset(line, 0, strlen(line));
        } else if (strcmp("fragment", cmd) == 0) {
            args = (char **) malloc(1 * sizeof(char *));
    
            if ((args[0] = strsep(&line, " ")) < 0) {
                free(args);
                ui_print("Chunker: Setting failed...\n");
                return (void *) ERR_MIS_SYNTAX;
            }
            
            temp = (int) strtol(args[0], NULL, 10);
            
            if (temp < PROTO_MIN_CHUNK_SIZE || temp > PROTO_MAX_CHUNK_SIZE) {
                free(args);
                ui_print("Chunker: %d is not in interval between %d and %d.\n", temp, PROTO_MIN_PACKET_SIZE, PROTO_MAX_PACKET_SIZE);
                return (void *) ERR_MIS_SYNTAX;
            }
            
            defaultMaxChunkSize= temp;
            ui_print("Chunker: %d is ok.\n", defaultMaxChunkSize);
            
        } else if (strcmp("damage", cmd) == 0) {
            args = (char **) malloc(sizeof(char *));
            
            if ((args[0] = strsep(&line, " ")) < 0) {
                free(args);
                ui_print("Damager: Syntax...\n");
                return (void *) ERR_MIS_SYNTAX;
            }
    
            temp = strtol(args[0], NULL, 10);
            
            free(args);
            if (temp) {
                if (damagingMap != NULL) free(damagingMap);
                
                args = (char **) malloc(temp * sizeof(char *));
                damagingMap = (long long *) malloc(temp * sizeof(long long));
                
                dmSize = temp;
                
                for (i = 0; i < dmSize; i++) {
                    if ((args[i] = strsep(&line, "<")) < 0) {
                        free(args);
                        free(damagingMap);
                        damagingMap = NULL;
                        dmSize = 0;
                        ui_print("Damager: Failed initialize map...\n");
                        return (void *) ERR_MIS_SYNTAX;
                    }
                    
                    damagingMap[i] = (long long) strtol(args[i], NULL, 10);
                }
                
                free(args);
            } else {
                if (damagingMap != NULL) free(damagingMap);
                dmSize = 0;
            }
    
            ui_print("Damager: Map is done, you destroyer.\n");
        } else if (strcmp("disconnect", cmd) == 0) {
            u_resetPeer();
            pthread_mutex_lock(&com_mutex_state);
            com_state = PROTO_STATE_WAITING;
            pthread_mutex_unlock(&com_mutex_state);
            pthread_cancel(threads[THREAD_REC]);
            pthread_join(threads[THREAD_REC], NULL);
            
            pthread_create(&threads[THREAD_REC], NULL, com_t_awaiting, NULL);
        } else if (strcmp("exit", cmd) == 0) {
            break;
        }
    }
    
    pthread_exit(0);
}