#include "thread_pool.h"

#include "seika/memory.h"

// --- Thread Pool Worker --- //
static SkaThreadPoolWork* tpool_work_create(SkaThreadFunc func, void *arg) {
    SkaThreadPoolWork* work;

    if (func == NULL) {
        return NULL;
    }

    work = SKA_MEM_ALLOCATE_SIZE(sizeof(*work));
    work->func = func;
    work->arg = arg;
    work->next = NULL;
    return work;
}

static void tpool_work_destroy(SkaThreadPoolWork* work) {
    if (work == NULL) {
        return;
    }
    SKA_MEM_FREE(work);
}


static SkaThreadPoolWork* tpool_work_get(SkaThreadPool* tp) {
    SkaThreadPoolWork* work;

    if (tp == NULL) {
        return NULL;
    }

    work = tp->workFirst;
    if (work == NULL) {
        return NULL;
    }

    if (work->next == NULL) {
        tp->workFirst = NULL;
        tp->workLast  = NULL;
    } else {
        tp->workFirst = work->next;
    }

    return work;
}

static void* tpool_worker(void *arg) {
    SkaThreadPool* tp = arg;
    SkaThreadPoolWork* work;

    while (true) {
        pthread_mutex_lock(&(tp->workMutex));

        while (tp->workFirst == NULL && !tp->shouldStop) {
            pthread_cond_wait(&(tp->workCond), &(tp->workMutex));
        }

        if (tp->shouldStop) {
            break;
        }

        work = tpool_work_get(tp);
        tp->workingCount++;
        pthread_mutex_unlock(&(tp->workMutex));

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tp->workMutex));
        tp->workingCount--;
        if (!tp->shouldStop && tp->workingCount == 0 && tp->workFirst == NULL) {
            pthread_cond_signal(&(tp->workingCond));
        }
        pthread_mutex_unlock(&(tp->workMutex));
    }

    tp->threadCount--;
    pthread_cond_signal(&(tp->workingCond));
    pthread_mutex_unlock(&(tp->workMutex));
    return NULL;
}


// --- Thread Pool --- //
SkaThreadPool* ska_tpool_create(usize num) {
    SkaThreadPool* tp;
    pthread_t thread;

    if (num == 0) {
        num = 2;
    }

    tp = SKA_MEM_ALLOCATE_SIZE_ZERO(1, sizeof(*tp));
    tp->threadCount = num;

    pthread_mutex_init(&(tp->workMutex), NULL);
    pthread_cond_init(&(tp->workCond), NULL);
    pthread_cond_init(&(tp->workingCond), NULL);

    tp->workFirst = NULL;
    tp->workLast  = NULL;

    for (usize i = 0; i < num; i++) {
        pthread_create(&thread, NULL, tpool_worker, tp);
        pthread_detach(thread);
    }

    return tp;
}

void ska_tpool_destroy(SkaThreadPool* tp) {
    SkaThreadPoolWork* work;
    SkaThreadPoolWork* work2;

    if (tp == NULL)
        return;

    pthread_mutex_lock(&(tp->workMutex));
    work = tp->workFirst;
    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }
    tp->shouldStop = true;
    pthread_cond_broadcast(&(tp->workCond));
    pthread_mutex_unlock(&(tp->workMutex));

    ska_tpool_wait(tp);

    pthread_mutex_destroy(&(tp->workMutex));
    pthread_cond_destroy(&(tp->workCond));
    pthread_cond_destroy(&(tp->workingCond));

    SKA_MEM_FREE(tp);
}

bool ska_tpool_add_work(SkaThreadPool* tp, SkaThreadFunc func, void* arg) {
    SkaThreadPoolWork* work;

    if (tp == NULL) {
        return false;
    }

    work = tpool_work_create(func, arg);
    if (work == NULL) {
        return false;
    }

    pthread_mutex_lock(&(tp->workMutex));
    if (tp->workFirst == NULL) {
        tp->workFirst = work;
        tp->workLast  = tp->workFirst;
    } else {
        tp->workLast->next = work;
        tp->workLast       = work;
    }

    pthread_cond_broadcast(&(tp->workCond));
    pthread_mutex_unlock(&(tp->workMutex));

    return true;
}

void ska_tpool_wait(SkaThreadPool* tp) {
    if (tp == NULL) {
        return;
    }

    pthread_mutex_lock(&(tp->workMutex));
    while (true) {
        if ((!tp->shouldStop && tp->workingCount != 0) || (tp->shouldStop && tp->threadCount != 0)) {
            pthread_cond_wait(&(tp->workingCond), &(tp->workMutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->workMutex));
}
