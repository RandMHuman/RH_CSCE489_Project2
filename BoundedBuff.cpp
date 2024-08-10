#include <pthread.h>
#include <stdexcept>
#include "BoundedBuff.h"

/*************************************************************************************
 * BoundedBuff (constructor) - initialize the BoundedBuff with the given count.
 *
 *    Params:  count - initial count for the BoundedBuff
 *
 *************************************************************************************/
BoundedBuff::BoundedBuff(int size){
    if (pthread_mutex_init(&mutex_, nullptr) != 0) {
        throw std::runtime_error("Mutex initialization failed");
    }
    if (pthread_cond_init(&full_cond_, nullptr) != 0) {
        pthread_mutex_destroy(&mutex_);
        throw std::runtime_error("Condition variable initialization failed");
    }
    if (pthread_cond_init(&empty_cond_, nullptr) != 0) {
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&full_cond_);
        throw std::runtime_error("Condition variable initialization failed");
    }
    size_ = size + 1;
    count_ = 0;
    in_ = 0;
    out_ = 0;
    buffer_ = (unsigned int *)malloc(size_ * sizeof(unsigned int));

}

/*************************************************************************************
 * ~BoundedBuff (destructor) - clean up the allocated buffer, mutex, and condition variable.
 *
 *************************************************************************************/
BoundedBuff::~BoundedBuff() {
    free(buffer_);
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&full_cond_);
    pthread_cond_destroy(&empty_cond_);

}

/*************************************************************************************
 * consume - consume an item (unsigned int represnting serial number) from the bounded buff. Decrement the BoundedBuff count and wait if it's zero or less.
 *
 *************************************************************************************/
unsigned int BoundedBuff::consume() {
    int consumed = 0;
    pthread_mutex_lock(&mutex_);
    if (in_ == out_){
        //count_ //count_ should == 0 here...
        pthread_cond_wait(&empty_cond_, &mutex_);
    }
    if(count_ == 0){
        printf("ERROR - Mutex / synchronization fail in BoundedBuff consume");
    }
    
    consumed = buffer_[out_];
    buffer_[out_] = 0;
    out_ = (out_ + 1) % size_;
    count_ = count_ - 1;

    pthread_cond_signal(&full_cond_);
    pthread_mutex_unlock(&mutex_);
    return consumed;
    
}

/*************************************************************************************
 * produce - increment the BoundedBuff count and signal one waiting thread if any.
 *
 *************************************************************************************/
void BoundedBuff::produce(unsigned int produced_item) {
    pthread_mutex_lock(&mutex_);
    if ( ((in_ + 1)%size_) == out_){
        //count_ //count_ should == size_ here, or size + 1, which is BUFFERSIZE, or max_buffer, or buffer_size...
        pthread_cond_wait(&full_cond_, &mutex_);
    }
    if (count_ >= size_){
        printf("ERROR - Mutex / synchronization fail in BoundedBuff produce");
    }

    buffer_[in_] = produced_item;
    in_ = (in_ + 1) % size_;
    count_ = count_ + 1;

    pthread_cond_signal(&empty_cond_);
    pthread_mutex_unlock(&mutex_);
}

int BoundedBuff::get_count() {
    return count_;
}