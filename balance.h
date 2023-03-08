#include "banking.h"
#include <stdlib.h>

BalanceState * new_balance(balance_t balance, timestamp_t time, balance_t balance0) {
    BalanceState * st = (BalanceState* ) malloc(sizeof(BalanceState));
    st->s_balance = balance;
    st->s_balance_pending_in = balance0;
    st->s_time = time;
    return st;
}
