#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "paintshop.h"
#include "paintshop_driver.h"


/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

/* Declare any globals you need here (e.g. locks, etc...) */
#define BUFFER_SIZE   10


static struct paintorder ord_buf[BUFFER_SIZE];
static unsigned int tint_buf[BUFFER_SIZE];	// Index +1 = tint num

static paint

static struct cv *ord_full;
static struct cv *ord_empty;
static struct lock *ord_lock;

static struct cv *tint_full;
static struct cv *tint_empty;
static struct lock *tint_lock;
/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_paint()
 *
 * Takes one argument referring to the order to be filled. The
 * function makes the order available to staff threads and then blocks
 * until the staff have filled the can with the appropriately tinted
 * paint.
 */

void order_paint(struct paintorder *order)
{
    //(void) order; /* Avoid compiler warning, remove when used */

	lock_acquire(ord_lock);
	while(ord_buf_ptr == BUFFER_SIZE)
		cv_wait(ord_full, ord_lock);
    ord_buf[ord_buf_ptr] = *order;
	ord_buf_ptr++;
	if(ord_buf_ptr == 1)
		cv_signal(ord_empty, ord_lock);
	lock_release(ord_lock);
}



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY PAINT SHOP STAFF THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it returns a pointer to the order.
 *
 */

struct paintorder *take_order(void)
{
	struct paintorder *ret = NULL;
	int i;

	lock_acquire(ord_lock);

	while(ord_buf_ptr == 0)
		cv_wait(ord_empty, ord_lock);
    ret = &ord_buf[0];
	ord_buf_ptr--;

	for(i=1;i<BUFFER_SIZE;i++)
		ord_buf[i-1] = ord_buf[i];

	if(ord_buf_ptr == BUFFER_SIZE-1)
		cv_signal(ord_full, ord_lock);

	lock_release(ord_lock);

	return ret;
}


/*
 * fill_order()
 *
 * This function takes an order provided by take_order and fills the
 * order using the mix() function to tint the paint.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE
 * REQUIRED TINTS (AND, IDEALLY, ONLY THE TINTS) IT NEEDS TO USE TO
 * FILL THE ORDER.
 */

void fill_order(struct paintorder *order)
{

	/* add any sync primitives you need to ensure mutual exclusion
	   holds as described */
	for (i = 0; i < PAINT_COMPLEXITY;i++){
		col = order->requested_tints[i];
		//order->can.contents[i] = col;
		tint_buf[col-1]++;
		if(tint_buf[col]>1)				//test
			panic("fill_order conflict");
	}


	lock_acquire(tint_lock);
	/* the call to mix must remain */
	mix(order);

	lock_release(tint_lock);
}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to (unblocks) the
 * waiting customer.
 */

void serve_order(struct paintorder *order)
{
	(void) order;
	//lock_release(tint_lock);
}



/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * paintshop_open()
 *
 * Perform any initialisation you need prior to opening the paint shop
 * to staff and customers. Typically, allocation and initialisation of
 * synch primitive and variable.
 */

void paintshop_open(void)
{
	int i;
	/*Initialize order buffer */
	ord_buf_ptr = 0;
	/*Initialize order lock and cv */
	ord_full = cv_create("ord_full");
	ord_empty = cv_create("ord_empty");
	ord_lock = lock_create("ord_lock");

	/*Initialize tint buffer */
	for(i = 0;i<BUFFER_SIZE;i++)
		tint_buf[i] = 0;
	/*Initialize tint lock and cv */
	tint_full = cv_create("tint_full");
	tint_empty = cv_create("tint_empty");
	tint_lock = lock_create("tint_lock");
}

/*
 * paintshop_close()
 *
 * Perform any cleanup after the paint shop has closed and everybody
 * has gone home.
 */

void paintshop_close(void)
{
	cv_destroy(ord_full);
	cv_destroy(ord_empty);
	lock_release(ord_lock);

	cv_destroy(tint_full);
	cv_destroy(tint_empty);
	lock_release(tint_lock);
}

