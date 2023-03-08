#include "banking.h"
#include "pipes.h"
#include <stdio.h>
#include <string.h>

void transfer(void * parent_data, local_id src, local_id dst, balance_t amount) {
//    pipes_all_global * pipesAllGlobal = (pipes_all_global *) parent_data;
    baby_maybe_process * babyMaybeProcess = (baby_maybe_process *) parent_data;

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

    printf("destination in transfer: %hhd\n", transferOrder.s_dst);

    memcpy(&message_transfer_send.s_payload, &transferOrder, sizeof(TransferOrder));

//
//    for (size_t i = 0; i < 75; i++) {
//        message_transfer_send.s_payload[i] = send_message[i];
//    }

    printf("%zu %d %s\n", sizeof(transferOrder), 8+message_transfer_send.s_header.s_payload_len, message_transfer_send.s_payload);

    int send_result = send(babyMaybeProcess, src, &message_transfer_send);

    printf("%d fsdfsgsdfasfsdgsdfghsdhd\n", send_result);

    Message message_transfer_receive;

//    printf("transfer near end\n");

    while (1){
        while (receive(babyMaybeProcess, dst, &message_transfer_receive) == -1) {}
        if (message_transfer_receive.s_header.s_type == ACK) {
            break;
        }
        break;
    }
}
