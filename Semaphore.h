#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore 
{
public:

	Semaphore(int count);
	~Semaphore();

	void wait();
	void signal();

private:
    int count_;                  // Semaphore count
    pthread_mutex_t mutex_;      // Mutex for protecting semaphore count
    pthread_cond_t cond_;        // Condition variable for waiting and signaling
};

#endif
