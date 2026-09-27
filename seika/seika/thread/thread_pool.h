#pragma once

#if SKA_THREAD

#include "seika/thread/pthread.h"

typedef void (*SkaThreadFunc)(void* arg);

typedef struct SkaThreadPoolWork {
    SkaThreadFunc func;
    void* arg;
    struct SkaThreadPoolWork* next;
} SkaThreadPoolWork;

typedef struct SkaThreadPool {
    SkaThreadPoolWork* work_first;
    SkaThreadPoolWork* work_last;
    pthread_mutex_t work_mutex;
    pthread_cond_t work_cond;
    pthread_cond_t working_cond;
    usize working_count;
    usize thread_count;
    bool should_stop;
} SkaThreadPool;

SkaThreadPool* ska_tpool_create(usize num);
void ska_tpool_destroy(SkaThreadPool* tp);
// Adds worker (or job) to the thread pool.
bool ska_tpool_add_work(SkaThreadPool* tp, SkaThreadFunc func, void* arg);
// Blocking function that finishes once all work is completed.
void ska_tpool_wait(SkaThreadPool* tp);

#endif // #if SKA_THREAD
