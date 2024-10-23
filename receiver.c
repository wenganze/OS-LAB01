#include "receiver.h"

void receive(message_t* message_ptr, mailbox_t* mailbox_ptr){
    /*  TODO: 
        1. Use flag to determine the communication method
        2. According to the communication method, receive the message
    */
    if(mailbox_ptr->flag == 1) mq_receive(mailbox_ptr->storage.msqid, message_ptr->text, MAX_MSG_SIZE, NULL);
    else if(mailbox_ptr->flag == 2) strcpy(message_ptr->text, mailbox_ptr->storage.shm_addr);
}

int main(int argc, char* argv[]){
    /*  TODO: 
        1) Call receive(&message, &mailbox) according to the flow in slide 4
        2) Measure the total receiving time
        3) Get the mechanism from command line arguments
            • e.g. ./receiver 1
        4) Print information on the console according to the output format
        5) If the exit message is received, print the total receiving time and terminate the receiver.c
    */
    mailbox_t mailbox;
    mailbox.flag = (int)(argv[1][0] - '0');

    sem_t *recv_sem = sem_open("/recv_sem", 0);
    sem_t *send_sem = sem_open("/send_sem", 0);

    if(mailbox.flag == 1) {
        mailbox.storage.msqid = mq_open("/msg_queue", O_RDONLY);
    }else if(mailbox.flag == 2){
        int shm_fd = shm_open("/share_memory", O_RDONLY, 0666);
        mailbox.storage.shm_addr = mmap(NULL, sizeof(char)*MAX_MSG_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    }

    struct timespec start, end;
    double time_taken = 0;

    printf("Message Passing\n");
    while (1)
    {  
        message_t recv;
        sem_wait(send_sem);
        clock_gettime(CLOCK_MONOTONIC, &start);
        receive(&recv, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        sem_post(recv_sem);
        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        if(strcmp(recv.text, "exit") == 0){
            printf("Sender exit!\n");
            sem_post(send_sem);
            break;
        }
        printf("Receiving message: %s\n", recv.text);
    }

    printf("Total taken time in receiving message: %f s\n", time_taken);


    sem_close(send_sem);
    sem_close(recv_sem);
    if(mailbox.flag == 1) mq_close(mailbox.storage.msqid);
    else if(mailbox.flag == 2) munmap(mailbox.storage.shm_addr, sizeof(char)*MAX_MSG_SIZE);

    return 0;
    
}