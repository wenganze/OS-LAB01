#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <time.h>

#define MAX_MSG_SIZE 1024

typedef struct {
    int flag;      // 1 for message passing, 2 for shared memory
    union{
        mqd_t msqid; //for system V api. You can replace it with struecture for POSIX api
        char* shm_addr;
    }storage;
} mailbox_t;


typedef struct {
    /*  TODO: 
        Message structure for wrapper
    */
    char text[MAX_MSG_SIZE];
} message_t;


void send(message_t message, mailbox_t* mailbox_ptr);