#include <pthread.h>
#include <stdexcept>
#include "Semaphore.h"

/*************************************************************************************
 * Semaphore (constructor) - initialize the semaphore with the given count.
 *
 *    Params:  count - initial count for the semaphore
 *
 *************************************************************************************/
Semaphore::Semaphore(int count){
    count_ = count;
    if (pthread_mutex_init(&mutex_, nullptr) != 0) {
        throw std::runtime_error("Mutex initialization failed");
    }
    if (pthread_cond_init(&cond_, nullptr) != 0) {
        pthread_mutex_destroy(&mutex_);
        throw std::runtime_error("Condition variable initialization failed");
    }
}

/*************************************************************************************
 * ~Semaphore (destructor) - clean up the mutex and condition variable.
 *
 *************************************************************************************/
Semaphore::~Semaphore() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
}

/*************************************************************************************
 * wait - decrement the semaphore count and wait if it's zero or less.
 *
 *************************************************************************************/
void Semaphore::wait() {
    pthread_mutex_lock(&mutex_);
    count_ = count_ - 1;
    if (count_ < 0) {
        pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
    
}

/*************************************************************************************
 * signal - increment the semaphore count and signal one waiting thread if any.
 *
 *************************************************************************************/
void Semaphore::signal() {
    pthread_mutex_lock(&mutex_);
    count_ = count_ + 1;
    if (count_ <= 0) {
        pthread_cond_signal(&cond_);
    }
    pthread_mutex_unlock(&mutex_);

}
int Semaphore::get_count() {
    return count_;
}

