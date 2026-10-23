#include "thread_pool.h"

#if SKA_THREAD

#include "seika/memory.h"

// --- Thread Pool Worker --- //
static SkaThreadPoolWork* tpool_work_create(SkaThreadFunc func, void *arg) {
    SkaThreadPoolWork* work;

    if (func == NULL) {
        return NULL;
    }

    work = SKA_ALLOC_BYTES(sizeof(*work));
    work->func = func;
    work->arg = arg;
    work->next = NULL;
    return work;
}

static void tpool_work_destroy(SkaThreadPoolWork* work) {
    if (work == NULL) {
        return;
    }
    SKA_FREE(work);
}


static SkaThreadPoolWork* tpool_work_get(SkaThreadPool* tp) {
    SkaThreadPoolWork* work;

    if (tp == NULL) {
        return NULL;
    }

    work = tp->work_first;
    if (work == NULL) {
        return NULL;
    }

    if (work->next == NULL) {
      tp->work_first = NULL;
      tp->work_last = NULL;
    } else {
      tp->work_first = work->next;
    }

    return work;
}

static void* tpool_worker(void *arg) {
    SkaThreadPool* tp = arg;
    SkaThreadPoolWork* work;

    while (true) {
      pthread_mutex_lock(&(tp->work_mutex));

      while (tp->work_first == NULL && !tp->should_stop) {
        pthread_cond_wait(&(tp->work_cond), &(tp->work_mutex));
      }

      if (tp->should_stop) {
        break;
      }

        work = tpool_work_get(tp);
        tp->working_count++;
        pthread_mutex_unlock(&(tp->work_mutex));

        if (work != NULL) {
            work->func(work->arg);
            tpool_work_destroy(work);
        }

        pthread_mutex_lock(&(tp->work_mutex));
        tp->working_count--;
        if (!tp->should_stop && tp->working_count == 0 &&
            tp->work_first == NULL) {
          pthread_cond_signal(&(tp->working_cond));
        }
        pthread_mutex_unlock(&(tp->work_mutex));
    }

    tp->thread_count--;
    pthread_cond_signal(&(tp->working_cond));
    pthread_mutex_unlock(&(tp->work_mutex));
    return NULL;
}


// --- Thread Pool --- //
SkaThreadPool* ska_tpool_create(usize num) {
    SkaThreadPool* tp;
    pthread_t thread;

    if (num == 0) {
        num = 2;
    }

    tp = SKA_ALLOC_BYTES_ZEROED(sizeof(*tp));
    tp->thread_count = num;

    pthread_mutex_init(&(tp->work_mutex), NULL);
    pthread_cond_init(&(tp->work_cond), NULL);
    pthread_cond_init(&(tp->working_cond), NULL);

    tp->work_first = NULL;
    tp->work_last = NULL;

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

    pthread_mutex_lock(&(tp->work_mutex));
    work = tp->work_first;
    while (work != NULL) {
        work2 = work->next;
        tpool_work_destroy(work);
        work = work2;
    }
    tp->should_stop = true;
    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    ska_tpool_wait(tp);

    pthread_mutex_destroy(&(tp->work_mutex));
    pthread_cond_destroy(&(tp->work_cond));
    pthread_cond_destroy(&(tp->working_cond));

    SKA_FREE(tp);
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

    pthread_mutex_lock(&(tp->work_mutex));
    if (tp->work_first == NULL) {
      tp->work_first = work;
      tp->work_last = tp->work_first;
    } else {
      tp->work_last->next = work;
      tp->work_last = work;
    }

    pthread_cond_broadcast(&(tp->work_cond));
    pthread_mutex_unlock(&(tp->work_mutex));

    return true;
}

void ska_tpool_wait(SkaThreadPool* tp) {
    if (tp == NULL) {
        return;
    }

    pthread_mutex_lock(&(tp->work_mutex));
    while (true) {
        if ((!tp->should_stop && tp->working_count != 0) || (tp->should_stop && tp->thread_count != 0)) {
            pthread_cond_wait(&(tp->working_cond), &(tp->work_mutex));
        } else {
            break;
        }
    }
    pthread_mutex_unlock(&(tp->work_mutex));
}

#endif // #if SKA_THREAD
