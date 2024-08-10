#ifndef BOUNDEDBUFF_H
#define BOUNDEDBUFF_H

class BoundedBuff 
{
public:

	BoundedBuff(int size);
	~BoundedBuff();

	void produce(unsigned int);
	unsigned int consume();
	int get_count();

private:
    int count_;
    int in_;
    int out_;
    int size_;                   // Buffer Size
    unsigned int * buffer_;       // A pointer to the allocated buffer of size 'size_'
    pthread_mutex_t mutex_;      // Mutex for protecting buffer
    pthread_cond_t full_cond_;        // Condition variable for waiting and signaling full
    pthread_cond_t empty_cond_;        // Condition variable for waiting and signaling empty
};

#endif
