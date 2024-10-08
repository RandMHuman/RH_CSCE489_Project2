/*************************************************************************************
 * babyyoda - used to test your semaphore implementation and can be a starting point for
 *			     your store front implementation
 *
 *************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "Semaphore.h"
#include "BoundedBuff.h"

// Semaphores that each thread will have access to as they are global in shared memory
Semaphore *empty = NULL; // Discuss: https://learning.oreilly.com/library/view/operating-system-concepts/9781118063330/12_chapter05.html#:-:text=5.6.2%20Semaphore%20Implementation,process%20to%20execute.
Semaphore *full = NULL; // https://learning.oreilly.com/library/view/operating-system-concepts/9781118063330/12_chapter05.html#:-:text=5.6%20Semaphores

pthread_mutex_t buf_mutex; // Discuss: https://learning.oreilly.com/library/view/operating-system-concepts/9781118063330/12_chapter05.html#:-:text=5.5%20Mutex%20Locks

//unsigned int buffer = 0;
//#define BUFFER_SIZE 10


//unsigned int *buffer;
//int in = 0;
//int out = 0;
BoundedBuff *buffer = NULL;

unsigned int consumed = 0;

bool quitthreads;

unsigned int serialnum;
unsigned int num_produce;

typedef struct {
	bool * quitthreads;
	unsigned int this_thread_id;
	unsigned int * produce;
	unsigned int * serialnum;
} ConsumerThreadArgs;

// Define the consumer thread data struct
typedef struct { 
    int num_threads; // Number of consumers
    unsigned int *ptr_thread_id_table; // Pointer to dynamically allocated array for thread name id
	ConsumerThreadArgs **ptr_thread_arg_addr_table; // pointer to dynamically allocated array of ptrs to dynamically allocated array of ConsumerThreadArgs
	pthread_t * ptr_tid_table;
} ThreadData;




ThreadData producer_thread_data;
ThreadData consumer_thread_data; // Contains ConsumerThreadArgs *ptr_thread_arg_table with ptr to condition variable 'quitthreads' for reeling in the consumers

/*************************************************************************************
 * producer_routine - this function is called when the producer thread is created.
 *
 *			Params: data - a void pointer that should point to an integer that indicates
 *							   the total number to be produced
 *
 *			Returns: always NULL
 *
 *************************************************************************************/

void *producer_routine(void *data) {

	time_t rand_seed;
	srand((unsigned int) time(&rand_seed));

	// The current serial number (incremented)
	//unsigned int serialnum = 1;
	
	// We know the data pointer is an integer that indicates the number to produce
	//int left_to_produce = *((int *) data);
	//int left_to_produce = *((int *) data);

	ConsumerThreadArgs * myArgs =  (ConsumerThreadArgs *) data;

	unsigned int *serialnum_ = myArgs->serialnum;
	unsigned int *produce_ = myArgs->produce;
	unsigned int item = 0;


	// Loop through the amount we're going to produce and place into the buffer
	//while (left_to_produce > 0) {
	while (*produce_ > 0) {
		// Semaphore check to make sure there is an available slot
		full->wait();
		if (*myArgs->quitthreads){
			continue; // We will check to see if quitthreads is true after being resumed to see if we should quit instead of shopping...
			// The buffer may be empty as we shop if we do not exit the loop here...
		}
		if (*produce_ <= 0){
			continue; // We will check to see if quitthreads is true after being resumed to see if we should quit instead of shopping...
			// The buffer may be empty as we shop if we do not exit the loop here...
		}
		printf("Producer %d working...\n", myArgs->this_thread_id);
		// Place item on the next shelf slot by first setting the mutex to protect our buffer vars
		pthread_mutex_lock(&buf_mutex);
		item = *serialnum_;
		printf("Producer %d wants to put Yoda #%d into buffer...\n", myArgs->this_thread_id, *serialnum_);
		//buffer = serialnum;
		buffer->produce(*serialnum_);
		*serialnum_ = item + 1;
		*produce_ = *produce_ - 1;
		
		printf("   Yoda #%d put on shelf by %d.\n", item, myArgs->this_thread_id);
		// Semaphore signal that there are items available
		empty->signal();
		pthread_mutex_unlock(&buf_mutex);


		// random sleep but he makes them fast so 1/20 of a second
		usleep((useconds_t) (rand() % 200000));
	
	}
	printf("Producer %d packing up and going home.\n", myArgs->this_thread_id);

	return NULL;
}


/*************************************************************************************
 * consumer_routine - this function is called when the consumer thread is created.
 *
 *       Params: data - a void pointer that should point to a boolean that indicates
 *                      the thread should exit. Doesn't work so don't worry about it
 *
 *       Returns: always NULL
 *
 *************************************************************************************/

void *consumer_routine(void * data) {

    // We know the data pointer is an unsigned integer that indicates the unique identifier for this consumer
	//unsigned int consumer_id = *((unsigned int *) data);
	//bool quitthreads = *((bool *) data + sizeof(unsigned int));
	// We know the data pointer is a ptr to ConsumerThreadArgs
	ConsumerThreadArgs * my_args = (ConsumerThreadArgs *) data;
	unsigned int consumer_id = my_args->this_thread_id;
	bool* ptr_quitthreads = my_args->quitthreads;
	printf("Consumer %d started\n",consumer_id);
	


	//bool quitthreads = false;

	while (!*my_args->quitthreads) {
		int purchase = -1;
		printf("Consumer %d wants to buy a Yoda...\n",consumer_id);
		int current_count = empty->get_count();
		printf("Consumer %d peaks in and sees: %d.\n",consumer_id, current_count);
		// Semaphore to see if there are any items to take
		empty->wait();
		if (*my_args->quitthreads){
			continue; // We will check to see if quitthreads is true after being resumed to see if we should quit instead of shopping...
			// The buffer may be empty as we shop if we do not exit the loop here...
		}


		// Take an item off the shelf
		pthread_mutex_lock(&buf_mutex);
					
		//buffer = 0;
		purchase = buffer->consume();
		consumed++; //DISCUSS -  This must also be protected. (Issues will be seen here ?? line 170: "while (consumed < num_produce)")
		printf("   Consumer %d bought Yoda #%d.\n", consumer_id, purchase);
			
		pthread_mutex_unlock(&buf_mutex);

		full->signal();

		// Consumers wait up to one second
		usleep((useconds_t) (rand() % 10000000));

		
	}
	printf("Consumer %d goes home.\n", consumer_id);

	return NULL;	
}


/*************************************************************************************
 * main - Standard C main function for our storefront. 
 *
 *		Expected params: pctest <num_consumers> <max_items>
 *				max_items - how many items will be produced before the shopkeeper closes
 *
 *************************************************************************************/

int main(int argv, const char *argc[]) {
	char character = __SCHAR_MAX__;
	int integer = __INT_MAX__;
	long longval = __LONG_MAX__;
	double doubleval = __LONG_LONG_MAX__;
	printf("sizeof(char) %d \n", sizeof(character));
	printf("sizeof(int) %d \n", sizeof(integer));
	printf("sizeof(long) %d \n", sizeof(longval));
	printf("sizeof(doubleval) %d \n", sizeof(doubleval));

	printf("__SCHAR_MAX__ %d \n", character);
	printf("__INT_MAX__ %d \n", integer);
	printf("__LONG_MAX__ %d \n", longval);
	printf("__LONG_LONG_MAX__ %d \n", doubleval);

	unsigned char u_character = 256;
	printf("sizeof(char) %d \n", sizeof(u_character));
	printf("u_character = 256 : %d \n", u_character);

	u_character = u_character - 1;
	printf("u_character = u_character - 1 : %d \n", u_character);


	// Get our argument parameters
	if (argv < 4) {
		printf("Invalid parameters. Format: %s <buffer_size> <num_consumers> <max_items>\n", argc[0]);
		exit(0);
	}

	// User input on the size of the buffer
	unsigned int buffer_size = (unsigned int) strtol(argc[1], NULL, 10);
	unsigned int num_consumers = (unsigned int) strtol(argc[2], NULL, 10);
	num_produce = (unsigned int) strtol(argc[3], NULL, 10);
	//unsigned int num_produce = 1000; //parse from args
	//unsigned int num_consumers = 400; //parse from args
	//unsigned int buffer_size = 100; //parse from args

	unsigned int num_producers = 5;
	unsigned int max_production = num_produce;
	
	printf("Producing %d today.\n", num_produce);
	
	// Initialize our semaphores
	empty = new Semaphore(0); // signifies no yodas when initialized...
	//full = new Semaphore(1); // Signifies one empty space when initialized... ** Needs to be updated to "max number of items that can be stocked at once... aka buffer_size argument"
	full = new Semaphore(buffer_size); // Signifies buffer_size empty space when initialized...

	pthread_mutex_init(&buf_mutex, NULL); // Initialize our buffer mutex

	buffer = new BoundedBuff(buffer_size);

	//pthread_t producer;
	pthread_t *producers = (pthread_t *) malloc(num_producers * sizeof(pthread_t));
	producer_thread_data.ptr_tid_table = producers;
	producer_thread_data.num_threads = num_producers;
	producer_thread_data.ptr_thread_id_table = (unsigned int *)malloc(num_producers * sizeof(unsigned int));
	ConsumerThreadArgs ** producer_thread_arg_table = NULL;
	producer_thread_arg_table = (ConsumerThreadArgs **)malloc(num_producers * sizeof(ConsumerThreadArgs *));
	ConsumerThreadArgs * producer_arg_struct_array = (ConsumerThreadArgs *)malloc(num_producers * sizeof(ConsumerThreadArgs));
	producer_thread_data.ptr_thread_arg_addr_table = producer_thread_arg_table;

	ConsumerThreadArgs * current_producer_arg = NULL;
    for (int i = 0; i < producer_thread_data.num_threads; i++) {
        producer_thread_data.ptr_thread_id_table[i] = i + 1; // Ids will be 1 to num_producers
		current_producer_arg = & producer_arg_struct_array[i];
		current_producer_arg->quitthreads = &quitthreads;
		producer_thread_data.ptr_thread_arg_addr_table[i] = current_producer_arg;// Changing what is at the destination of the indexed pointer ptr_thread_arg_table[i] (effectively we are saving the address to the current arg struct into the address table...)
    }


	//pthread_t consumer;
	//pthread_t consumers[num_consumers];
	pthread_t *consumers = (pthread_t *) malloc(num_consumers * sizeof(pthread_t));

    // Initialize the thread data struct and allocate memory for consumer ids (struct declared in global space)
	consumer_thread_data.ptr_tid_table = consumers;
	consumer_thread_data.num_threads = num_consumers;
	consumer_thread_data.ptr_thread_id_table = (unsigned int *)malloc(num_consumers * sizeof(unsigned int));
    ConsumerThreadArgs ** thread_arg_table = NULL;
	thread_arg_table = (ConsumerThreadArgs **)malloc(num_consumers * sizeof(ConsumerThreadArgs *));
	ConsumerThreadArgs * arg_struct_array = (ConsumerThreadArgs *)malloc(num_consumers * sizeof(ConsumerThreadArgs));
	consumer_thread_data.ptr_thread_arg_addr_table = thread_arg_table;
	



    // Check if memory allocation was successful
    if (consumer_thread_data.ptr_thread_id_table == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }

	ConsumerThreadArgs * current_arg = NULL;
    for (int i = 0; i < consumer_thread_data.num_threads; i++) {
        consumer_thread_data.ptr_thread_id_table[i] = i + 1; // Ids will be 1 to num_consumers
		current_arg = & arg_struct_array[i];
		current_arg->quitthreads = &quitthreads;
		consumer_thread_data.ptr_thread_arg_addr_table[i] = current_arg;// Changing what is at the destination of the indexed pointer ptr_thread_arg_table[i] (effectively we are saving the address to the current arg struct into the address table...)
    }


	// Launch our producer thread
	//pthread_create(&producer, NULL, producer_routine, (void *) &num_produce);
	serialnum = 1;
	for (unsigned int i=0; i<num_producers; i++) {
		current_producer_arg = producer_thread_data.ptr_thread_arg_addr_table[i];
		current_producer_arg->this_thread_id = i + 1;
		current_producer_arg->produce = &num_produce;
		current_producer_arg->serialnum = &serialnum;

		pthread_create(&producers[i], NULL, producer_routine, (void *) current_producer_arg);
		//pthread_join(consumers[i], NULL);
	}


	// Launch our consumer thread
	//pthread_create(&consumer, NULL, consumer_routine, NULL);

	for (unsigned int i=0; i<num_consumers; i++) {
		current_arg = consumer_thread_data.ptr_thread_arg_addr_table[i];
		current_arg->this_thread_id = i + 1;
	
		pthread_create(&consumers[i], NULL, consumer_routine, (void *) current_arg);
		//pthread_join(consumers[i], NULL);
	}




	// Wait for our producer thread to finish up
	//pthread_join(producer, NULL);
	for (unsigned int i=0; i<num_producers; i++) {
		printf("pthread join for producer tid: %d.\n", i);
		pthread_join(producers[i], NULL);
	}


	printf("The manufacturers have completed their work for the day.\n");

	printf("Produced %d today.\n", max_production);
	printf("Waiting for consumer to buy up the rest.\n");

	// Give the consumers a second to finish snatching up items
	while (consumed < max_production)
		sleep(1);
	
	
	printf("Consumed %d today.\n", consumed);
	
	// Now we know all of the Yoda are consumed...
	// We may have some consumers stuck on the empty semaphore...
	
	quitthreads = true;
	
	int sleeping_consumers = empty->get_count();
	printf("Sleeping Consumers: %d.\n", sleeping_consumers);

	
	// Now make sure they all exited...
	for (unsigned int i=0; i<num_consumers; i++) {
		sleeping_consumers = empty->get_count();
		printf("Sleeping Consumers: %d.\n", sleeping_consumers);

		//Can we get a list of the waiting processes?
		if (empty->get_count() != 0) empty->signal(); // DISCUSS - Could wind up grabbing an 'empty' yoda... Need to check the shelf if re-using the semaphor to wake the consumer...

				// Consumer 2 wants to buy a Yoda...
				// Yoda put on shelf.
				// Consumer 1 bought Yoda #10.
				// The manufacturer has completed his work for the day.
				// Waiting for consumer to buy up the rest.
				// Consumer 2 bought Yoda #0.
				// Consumer 1 goes home.
				// Consumer 2 goes home.
				// Producer/Consumer simulation complete!
	}

	for (unsigned int i=0; i<num_consumers; i++) {
		sleeping_consumers = empty->get_count();
		printf("pthread join for tid: %d.\n", i);
		pthread_join(consumers[i], NULL);
	}

	//free(shared_consumer_thread_data.ptr_thread_id_table);


	// We are exiting, clean up
	delete empty;
	delete full;

	printf("Producer/Consumer simulation complete!\n");

}
