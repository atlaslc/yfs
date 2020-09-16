#ifndef pWrapper_H__
#define pWrapper_H__

#include <pthread.h>
#include <assert.h>
#include <sys/time.h>

class pMutex {
  friend class pCond;

  public:
  pMutex() {
    assert(pthread_mutex_init(&lock_,NULL)==0);
  }

  ~pMutex() {
    assert(pthread_mutex_destroy(&lock_) == 0);
  }

  void lock() {
    assert(pthread_mutex_lock(&lock_) == 0);
  }

  void unlock() {
    assert(pthread_mutex_unlock(&lock_) == 0);
  }

  private:
    pthread_mutex_t lock_;

};


class pScopedMutex {
  public:
    pScopedMutex(pMutex *l): l_(l) {
      l->lock();
    }
    ~pScopedMutex() {
      l_->unlock();
    }
  private:
    pMutex *l_;

};

class pCond {
  public:
    pCond() {
      assert(pthread_cond_init(&cond_,NULL)==0);
    }
    ~pCond() {
      assert(pthread_cond_destroy(&cond_)==0);
    }
    void wait(pMutex *mutex) {
      assert(pthread_cond_wait(&cond_, &(mutex->lock_)) == 0);
    }
    int timedwait(pMutex *mutex, int sec) {
        struct timeval now;
        gettimeofday(&now, NULL);
        struct timespec next_timeout;
        next_timeout.tv_sec = now.tv_sec + sec;
        next_timeout.tv_nsec = 0;
        int r = pthread_cond_timedwait(&cond_, &(mutex->lock_),&next_timeout);
        return r;
    }
    void signal() {
      assert(pthread_cond_signal(&cond_)==0);
    }
    void broadcast() {
      assert(pthread_cond_broadcast(&cond_) == 0);
    }
  private:
    pthread_cond_t cond_;

};

#endif 
