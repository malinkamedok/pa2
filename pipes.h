#ifndef PA1_PIPES_H
#define PA1_PIPES_H

#include "mm_malloc.h"

struct pipes {
    int fd[2];
};

typedef struct {
    size_t number_of_child_procs;
    struct pipes ** pipes_all;
} pipes_all_global;

pipes_all_global * new(size_t number);

typedef struct {
    size_t id;
    pipes_all_global * global_elite;
} baby_maybe_process;

baby_maybe_process * new_baby(size_t id, pipes_all_global * x);

#endif //PA1_PIPES_H
