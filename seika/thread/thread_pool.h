#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "seika/thread/pthread.h"

typedef void (*SkaThreadFunc)(void* arg);

typedef struct SkaThreadPoolWork {
    SkaThreadFunc func;
    void* arg;
    struct SkaThreadPoolWork* next;
} SkaThreadPoolWork;

typedef struct SkaThreadPool {
    SkaThreadPoolWork* workFirst;
    SkaThreadPoolWork* workLast;
    pthread_mutex_t workMutex;
    pthread_cond_t workCond;
    pthread_cond_t workingCond;
    usize workingCount;
    usize threadCount;
    bool shouldStop;
} SkaThreadPool;

SkaThreadPool* ska_tpool_create(usize num);
void ska_tpool_destroy(SkaThreadPool* tp);
// Adds worker (or job) to the thread pool.
bool ska_tpool_add_work(SkaThreadPool* tp, SkaThreadFunc func, void* arg);
// Blocking function that finishes once all work is completed.
void ska_tpool_wait(SkaThreadPool* tp);
