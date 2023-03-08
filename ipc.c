#include "ipc.h"
#include <stdio.h>
#include "pipes.h"
#include "unistd.h"

int send(void * self, local_id dst, const Message * msg) {
//    printf("hahahahha baby\n");
    baby_maybe_process * glb = (baby_maybe_process * ) self;
    printf("proc %zu send msg to %d use des %d\n", glb->id, dst, glb->global_elite->pipes_all[glb->id][dst].fd[1]);
    write(glb->global_elite->pipes_all[glb->id][dst].fd[1], msg, 8 + msg->s_header.s_payload_len);
    return 0;
}

int send_multicast(void * self, const Message * msg) {
    baby_maybe_process * glb = (baby_maybe_process * ) self;

    for (int8_t j = 0; j < (int8_t) glb->global_elite->number_of_child_procs; j++) {
        if (glb->id != j) {
            send(self, j, msg);
//            write(glb->global_elite->pipes_all[glb->id][j].fd[1], msg, 8 + msg->s_header.s_payload_len);
        }
    }
    return 0;
}

int receive(void * self, local_id from, Message * msg) {
    baby_maybe_process * glb = (baby_maybe_process * ) self;

//    printf("receive called\n");

    int read_header_result = read(glb->global_elite->pipes_all[from][glb->id].fd[0], &msg->s_header, sizeof(MessageHeader));
//    printf("read_header_result %d\n", read_header_result);
    if (read_header_result == -1) {
        return -1;
    }
    int read_body_result = read(glb->global_elite->pipes_all[from][glb->id].fd[0], &msg->s_payload, msg->s_header.s_payload_len);
//    printf("read_body_result %d\n", read_body_result);
    if (read_header_result == -1 || read_body_result == -1) {
        return -1;
    }

    return 0;
}

int receive_any(void * self, Message * msg) {
    baby_maybe_process * glb = (baby_maybe_process * ) self;

    while (1) {
//        printf("receive any started\n");
//        for (int8_t j = 0; j < (int8_t) glb->global_elite->number_of_child_procs; j++) {
        for (size_t j = 0; j < glb->global_elite->number_of_child_procs; j++) {
//            printf("r a cycle %zu\n", j);
            if (glb->id != j) {
//                printf("if condition\n");
                int receive_result = receive(self, j, msg);
//                printf("receive result: %d\n", receive_result);
                if (receive_result == 0) {
                    return 0;
                }
                if (receive_result == -1) {
//                    printf("zalupa\n");
                }
            }
        }
    }
}
