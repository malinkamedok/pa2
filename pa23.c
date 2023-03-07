#include "banking.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "pipes.h"
#include "pa2345.h"
#include "ipc.h"

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount)
{
    pipes_all_global * pipesAllGlobal = (pipes_all_global *) parent_data;

    char send_message[75];
    TransferOrder transferOrder;
    transferOrder.s_amount = amount;
    transferOrder.s_dst = dst;
    transferOrder.s_src = src;
    Message message_transfer_send;
    MessageHeader messageHeader;
    messageHeader.s_magic = MESSAGE_MAGIC;
    messageHeader.s_payload_len = sizeof(transferOrder);
    messageHeader.s_local_time = get_physical_time();
    messageHeader.s_type = TRANSFER;
    message_transfer_send.s_header = messageHeader;
    for (size_t i = 0; i < 75; i++) {
        message_transfer_send.s_payload[i] = send_message[i];
    }

    send(pipesAllGlobal, src, &message_transfer_send);

    Message message_transfer_receive;

    while (1){
        while (receive(pipesAllGlobal, dst, &message_transfer_receive) == -1) {}
        if (message_transfer_receive.s_header.s_type == ACK) {
            break;
        }
    }
}

int main(int argc, char * argv[])
{

    if (strcmp(argv[1], "-p") != 0) {
        return -1;
    }

    int number_of_child_procs = atoi(argv[2]);
    if (number_of_child_procs == 0) {
        return -1;
    }

    if (number_of_child_procs < 1 || number_of_child_procs > 10) {
        return -1;
    }

    if (argc < 4 || argc > 13) {
        return -1;
    }

    if (argc-3 != number_of_child_procs) {
        return -1;
    }

    int* money_at_start  = (int*)calloc(number_of_child_procs, sizeof(int));

    printf( "Size = %d\n", number_of_child_procs ) ;

    for( int i = 3; i < argc; i++)
    {
        sscanf( argv[i], "%d", &money_at_start[i-3] ) ;
    }

    for (int i = 0; i < number_of_child_procs; i++) {
        printf("child number %d has money %d\n", i+1, money_at_start[i]);
    }

    FILE *pi;
    pi = fopen(pipes_log, "a+");

    FILE *ev;
    ev = fopen(events_log, "a+");

    pipes_all_global * global = new(number_of_child_procs+1);

    //pipes creation
    for (size_t i = 0; i < number_of_child_procs + 1; i++) {
        for (size_t j = 0; j < number_of_child_procs + 1; j++) {
            if (i != j) {
                int result_pipe = pipe(global->pipes_all[i][j].fd);
                fprintf(pi, "pipe opened i: %zu j: %zu\n", i, j);
                printf("pipe opened i: %zu j: %zu\n", i, j);
//                fcntl(global->pipes_all[i][j].fd[0], F_SETFL, O_NONBLOCK);
//                fcntl(global->pipes_all[i][j].fd[1], F_SETFL, O_NONBLOCK);
                if (result_pipe == -1) {
                    return -1;
                }
            }
        }
    }

    size_t process_number = 0;

    //procs creation
    for (size_t count = 1; count < number_of_child_procs + 1; count++) {
        if (fork() == 0) {
            process_number = count;
            printf("process %zu started\n", process_number);

            printf(log_started_fmt, get_physical_time(), (int ) process_number, getpid(), getppid(), money_at_start[count-1]);

            fprintf(ev, log_started_fmt, get_physical_time(), (int ) process_number, getpid(), getppid(), money_at_start[count-1]);

            fprintf(pi, "zalupa haha\n");

//            printf("proc %zu printed in log\n", process_number);

            //CLOSING PIPES FOR CHILD PROC
            // close pipes not working with current process
//            for (size_t i = 0; i < number_of_child_procs+1; i++) {
//                for (size_t j = 0; j < number_of_child_procs+1; j++) {
//                    if (i != j && i != process_number && j != process_number) {
//                        close(global->pipes_all[i][j].fd[0]);
//                        close(global->pipes_all[i][j].fd[1]);
//                        close(global->pipes_all[j][i].fd[0]);
//                        close(global->pipes_all[j][i].fd[1]);
//                    }
//                }
//            }
//            for (size_t i = 0; i < number_of_child_procs+1; i++) {
//                if (i != process_number) {
//                    close(global->pipes_all[process_number][i].fd[0]);
//                    close(global->pipes_all[i][process_number].fd[1]);
//                }
//            }
            baby_maybe_process * babyMaybeProcess = new_baby(process_number, global);

//            printf("pipes closed\n");

            //SENDING STARTED MESSAGE
            char started_message[75];
            sprintf(started_message, log_started_fmt, get_physical_time(), (int ) process_number, getpid(), getppid(), money_at_start[count-1]);

            Message message_start_send;
            MessageHeader messageHeader;
            messageHeader.s_magic = MESSAGE_MAGIC;
            messageHeader.s_payload_len = 75;
            messageHeader.s_local_time = get_physical_time();
            messageHeader.s_type = STARTED;
            message_start_send.s_header = messageHeader;
            for (size_t i = 0; i < 75; i++) {
                message_start_send.s_payload[i] = started_message[i];
            }

//            for (size_t i = 0; i < number_of_child_procs+1; i++) {
//                if (process_number != i) {
//                    write(global->pipes_all[process_number][i].fd[1], &message_start_send, 75);
//                    printf("proc %zu message to proc %zu printed\n", process_number, i);
//                }
//            }

//            printf("trying to send msg\n");
            send_multicast((void*)babyMaybeProcess, &message_start_send);
            printf("msg sent\n");

            //READING STARTED MESSAGE
            for (size_t i = 1; i < number_of_child_procs+1; i++) {
                if (i != process_number) {
                    printf("child proc %zu try read msg from %zu use des %d\n", process_number, i, global->pipes_all[i][process_number].fd[0]);
                    Message message_start_receive;
//                    read(global->pipes_all[i][process_number].fd[0], &message_start_receive.s_header, sizeof(MessageHeader));
//                    read(global->pipes_all[i][process_number].fd[0], message_start_receive.s_payload, message_start_receive.s_header.s_payload_len);

                    receive((void*)babyMaybeProcess, i, &message_start_receive);
                    printf("child %zu received message: %s\n", count, message_start_receive.s_payload);
                }
            }

            //log_received_all_started_fmt
            printf(log_received_all_started_fmt, get_physical_time(), (int ) process_number);
            fprintf(ev, log_received_all_started_fmt, get_physical_time(), (int ) process_number);
            printf("trying to read stop msg\n");
            Message stop_message_receive;
            receive((void *) babyMaybeProcess, 0, &stop_message_receive);
//            printf("stop msg: %d\n", stop_message_receive.s_header.s_type);
            printf("stop received\n");
            if (stop_message_receive.s_header.s_type == STOP) {
                printf("stop msg received by %zu proc\n", process_number);
            }


            Message any_message_receive;
            TransferOrder transferOrder;
            int done_procs, stop_signal = 0;
            while (1) {
                while (receive_any(global, &any_message_receive) == -1) {}
                switch (any_message_receive.s_header.s_type) {
                    case TRANSFER:
                        memcpy(&transferOrder, any_message_receive.s_payload, any_message_receive.s_header.s_payload_len);
                        if (process_number == transferOrder.s_src) {
                            timestamp_t time = get_physical_time();
                            money_at_start[process_number-1] = money_at_start[process_number-1] - transferOrder.s_amount;

                            char transfer_to_dst_char[75];
                            sprintf(transfer_to_dst_char, log_transfer_out_fmt, get_physical_time(), (int ) process_number, transferOrder.s_amount, transferOrder.s_dst);

                            Message transfer_to_dst;
                            MessageHeader messageHeader;
                            messageHeader.s_magic = MESSAGE_MAGIC;
                            messageHeader.s_payload_len = 75;
                            messageHeader.s_local_time = get_physical_time();
                            messageHeader.s_type = TRANSFER;
                            transfer_to_dst.s_header = messageHeader;
                            for (size_t i = 0; i < 75; i++) {
                                transfer_to_dst.s_payload[i] = transfer_to_dst_char[i];
                            }
                            send(global, transferOrder.s_dst, &transfer_to_dst);
                            any_message_receive.s_header.s_type = -1;
                        } else if (process_number == transferOrder.s_dst) {
                            timestamp_t time = get_physical_time();

                            Message send_ack_msg;
                            MessageHeader messageHeader;
                            messageHeader.s_type = ACK;
                            messageHeader.s_payload_len = 0;
                            messageHeader.s_local_time = get_physical_time();
                            messageHeader.s_magic = MESSAGE_MAGIC;
                            send_ack_msg.s_header = messageHeader;

                            send(global, 0, &send_ack_msg);
                        } else {
                            printf("msg receive fault");
                        }
                    case STOP:

                        char transfer_to_dst_char[75];
                        sprintf(transfer_to_dst_char, log_transfer_out_fmt, get_physical_time(), (int ) process_number, transferOrder.s_amount, transferOrder.s_dst);

                        Message transfer_to_dst;
                        MessageHeader messageHeader;
                        messageHeader.s_magic = MESSAGE_MAGIC;
                        messageHeader.s_payload_len = 75;
                        messageHeader.s_local_time = get_physical_time();
                        messageHeader.s_type = TRANSFER;
                        transfer_to_dst.s_header = messageHeader;
                        for (size_t i = 0; i < 75; i++) {
                            transfer_to_dst.s_payload[i] = transfer_to_dst_char[i];
                        }
                        send(global, transferOrder.s_dst, &transfer_to_dst);

//                        Message message_done_send;
//                        MessageHeader messageHeader;
//                        messageHeader.s_type = DONE;
//                        messageHeader.s_payload_len = 75;
//                        messageHeader.s_magic = MESSAGE_MAGIC;
//                        messageHeader.s_local_time = get_physical_time();
//                        message_done_send.s_header = messageHeader;
//                        char message_done_char[75];
//                        sprintf(message_done_char, log_done_fmt, get_physical_time(), (int )process_number, money_at_start[process_number-1]);
//
//                        for (size_t i = 0; i < 75; i++) {
//                            message_done_send.s_payload[i] = message_done_char[i];
//                        }
//
//                        send_multicast(global, &message_done_send);

                        stop_signal = 1;
                        if (done_procs == global->number_of_child_procs - 2) {
                            //log all done
                            break;
                        }
                    case DONE:
                        done_procs = done_procs + 1;
                        if (stop_signal == 1 && done_procs == global->number_of_child_procs - 2) {
                            //log all done
                            break;
                        }
                }
            }
            exit(0);
        }
    }

    //READING STARTED MESSAGE BY PARENT
    for (size_t i = 1; i < number_of_child_procs + 1; i++) {
        Message message;
        read(global->pipes_all[i][0].fd[0], &message.s_header, sizeof(MessageHeader));
        read(global->pipes_all[i][0].fd[0], message.s_payload, message.s_header.s_payload_len);
        printf("parent received: %s from proc #%zu\n", message.s_payload, i);
    }

    bank_robbery(global, number_of_child_procs);

    Message stop_message;
    MessageHeader messageHeader;
    messageHeader.s_type = STOP;
    messageHeader.s_payload_len = 0;
    messageHeader.s_local_time = get_physical_time();
    messageHeader.s_magic = MESSAGE_MAGIC;
    stop_message.s_header = messageHeader;

    send_multicast(global, &stop_message);

    for (size_t i = 1; i < number_of_child_procs + 1; i++) {
        Message message;
        read(global->pipes_all[i][0].fd[0], &message.s_header, sizeof(MessageHeader));
        read(global->pipes_all[i][0].fd[0], message.s_payload, message.s_header.s_payload_len);
        printf("parent received: %s from proc #%zu\n", message.s_payload, i);
    }

    //log all done


//    bank_robbery(global, number_of_child_procs);

//    for (size_t i = 1; i < number_of_child_procs+1; i++) {
//
//        Message done_message_send;
//        MessageHeader messageHeader;
//        messageHeader.s_type = STOP;
//        messageHeader.s_magic = MESSAGE_MAGIC;
//        messageHeader.s_local_time = get_physical_time();
//        messageHeader.s_payload_len = sizeof(done_message_send);
//        done_message_send.s_header = messageHeader;
//
//
//        write(global->pipes_all[0][i].fd[1], &done_message_send, 8);
//
//    }

    fclose(ev);
    fclose(pi);

    //bank_robbery(parent_data);
    //print_history(all);

    return 0;
}
