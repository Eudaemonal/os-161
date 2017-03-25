/* This file will contain your solution. Modify it as you wish. */
#include <types.h>

#include <synch.h>  /* for P(), V(), sem_* */
#include "producerconsumer_driver.h"

/* Declare any variables you need here to keep track of and
   synchronise your bounded. A sample declaration of a buffer is shown
   below. You can change this if you choose another implementation. */

static struct pc_data buffer[BUFFER_SIZE];
int buf_start;
int buf_end;

static struct semaphore *empty;
static struct semaphore *full;
static struct semaphore *mutex;


/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

struct pc_data consumer_receive(void)
{
	struct pc_data thedata;

	P(full);
	P(mutex);

	thedata = buffer[buf_start];
	buf_start++;
	if(buf_start==BUFFER_SIZE)
		buf_start = 0;

	V(mutex);
	V(empty);

	return thedata;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
	P(empty);
	P(mutex);

	buffer[buf_end] = item;
	buf_end++;
	if(buf_end==BUFFER_SIZE)
		buf_end = 0;

	V(mutex);
	V(full);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
	int i = 0;

	buf_start = 0;
	buf_end = 0;

	for(i=0;i<BUFFER_SIZE;i++){
		buffer[i].item1 = 0;
		buffer[i].item2 = 0;
	}

	empty = sem_create("empty", BUFFER_SIZE);
	full = sem_create("full", 0);
	mutex = sem_create("mutex", 1);

}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
	sem_destroy(empty);
	sem_destroy(full);
	sem_destroy(mutex);
}

