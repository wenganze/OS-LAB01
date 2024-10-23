#include "sender.h"

void send(message_t message, mailbox_t* mailbox_ptr){
    /*  TODO: 
        1. Use flag to determine the communication method
        2. According to the communication method, send the message
    */
    if(mailbox_ptr->flag == 1) mq_send(mailbox_ptr->storage.msqid, message.text,  strlen(message.text) + 1, 0);
    else if(mailbox_ptr->flag == 2) strcpy(mailbox_ptr->storage.shm_addr, message.text);
}

int main(int argc, char* argv[]){
    /*  TODO: 
        1) Call send(message, &mailbox) according to the flow in slide 4
        2) Measure the total sending time
        3) Get the mechanism and the input file from command line arguments
            • e.g. ./sender 1 input.txt
                    (1 for Message Passing, 2 for Shared Memory)
        4) Get the messages to be sent from the input file
        5) Print information on the console according to the output format
        6) If the message form the input file is EOF, send an exit message to the receiver.c
        7) Print the total sending time and terminate the sender.c
    */

    sem_unlink("/recv_sem");
    sem_unlink("/send_sem");
    mq_unlink("/msg_queue");
    shm_unlink("/share_memory");


    FILE *file;
    file = fopen(argv[2], "r");
    mailbox_t mailbox;
    mailbox.flag = (int)(argv[1][0] - '0');
    int shm_fd;
    if(mailbox.flag == 1) {
        struct mq_attr attr = {0, 10, MAX_MSG_SIZE, 0};
        mailbox.storage.msqid = mq_open("/msg_queue", O_CREAT|O_RDWR, 0666, &attr);
    }
    else if(mailbox.flag == 2){
        shm_fd = shm_open("/share_memory", O_CREAT|O_RDWR, 0666);
        ftruncate(shm_fd, sizeof(char) * MAX_MSG_SIZE);
        mailbox.storage.shm_addr = mmap(NULL, sizeof(char)*MAX_MSG_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, shm_fd, 0);
    }
    sem_t *recv_sem = sem_open("/recv_sem", O_CREAT, 0666, 0);
    sem_t *send_sem = sem_open("/send_sem", O_CREAT, 0666, 0);

    if(!file){
        perror("Failed to open file");
        return EXIT_FAILURE;
    }

    struct timespec start, end;
    double time_taken = 0;

    char line[MAX_MSG_SIZE];
    printf("Message Passing\n");
    while (fgets(line, MAX_MSG_SIZE, file)){
        message_t msg;
        line[strcspn(line, "\n")] = '\0';
        printf("Sending message: %s\n", line);
        strcpy(msg.text, line);
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(msg, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        sem_post(send_sem);
        sem_wait(recv_sem);
    }

    if(feof(file)){
        printf("End of the file! Exit!\n");
        message_t msg;
        strcpy(msg.text, "exit");
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(msg, &mailbox);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_taken += (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) * 1e-9;
        sem_post(send_sem);
        sem_wait(recv_sem);
    }

    printf("Total taken time in sending message: %f s\n", time_taken);

    sem_close(send_sem);
    sem_close(recv_sem);    
    if(mailbox.flag == 1){
        mq_close(mailbox.storage.msqid);
        mq_unlink("/msg_queue");
    }else if(mailbox.flag == 2){
        munmap(mailbox.storage.shm_addr, sizeof(char)*MAX_MSG_SIZE);
        shm_unlink("/share_memory");
    }
    

    return 0;
}