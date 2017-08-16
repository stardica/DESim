/*
 * DESim.c
 *
 *  Created on: Aug 3, 2017
 *      Author: stardica
 */


#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "desim.h"

context inittask;
context *curtask = &inittask;
context *hint = NULL;
eventcount etime;
eventcount *ectail = NULL;
eventcount *last_ec = NULL; /* to work with context library */
long long ecid = 0;
count_t last_value = 0;
context *current_context = NULL;
context *terminated_context = NULL;


void desim_init(void){

	//set up etime
	etime.name = strdup("etime");
	etime.id = 0;
	etime.count = 0;
	//etime.tasklist = NULL;
	etime.nextcontext = NULL;
	etime.nextec = NULL;

	//set up initial task
	inittask.nextcontext = NULL;
	inittask.name = strdup("initial task");
	inittask.count = 0;
	inittask.start = NULL;
	inittask.id = -1;
	inittask.magic = STK_OVFL_MAGIC;

	return;
}

eventcount *eventcount_create(char *name){

	eventcount *ec = NULL;

	ec = (eventcount *)malloc(sizeof(eventcount));
	assert(ec);

	eventcount_init(ec, 0, name);

	return ec;
}

void context_create(void (*func)(void), unsigned stacksize, char *name){

	/*stacksize should be multiple of unsigned size */
	assert ((stacksize % sizeof(unsigned)) == 0);

	context *new_context_ptr = NULL;

	new_context_ptr = (context *) malloc(sizeof(context));
	assert(new_context_ptr);

	new_context_ptr->count = etime.count;
	new_context_ptr->name = name;
	new_context_ptr->stack = (char *)malloc(stacksize);
	assert(new_context_ptr->stack);
	new_context_ptr->stacksize = stacksize;
	new_context_ptr->magic = STK_OVFL_MAGIC; // for stack overflow check
	new_context_ptr->start = func; /*assigns the head of a function*/

	/*list work. forms a singly linked list, with newest task at the head.
	etime.contextlist points to the head of the list*/
	new_context_ptr->nextcontext = etime.nextcontext;
	etime.nextcontext = new_context_ptr;

	context_init(new_context_ptr);

	return;
}

void eventcount_init(eventcount * ec, count_t count, char *ecname){

	ec->name = ecname;
	ec->id = ecid++;
	//ec->tasklist = NULL;
	ec->nextcontext = NULL;
	ec->count = count;
	ec->nextec = NULL;

	//create singly linked list
    if (ectail == NULL)
    {
    	//always points to first event count created.
    	etime.nextec = ec;
    }
    else
    {
    	//old tail!!
    	ectail->nextec = ec;
    }

    //point to new tail!
    ectail = ec;

    return;
}

/* advance(ec) -- increment an event count. */
/* Don't use on time event count! */
void advance(eventcount *ec){

	context *context_ptr = NULL;
	context *context_pptr = NULL;

	/* advance the ec's count */
	ec->count++;

	context_ptr = ec->nextcontext;

	/* check for no tasks being enabled */
	printf("ec name %s value %llu\n", ec->name, ec->count);

	if(!context_ptr)
		printf("no ctx yet\n");
	else
		printf("ec %s val %llu ctx %s val %llu\n", ec->name, ec->count, context_ptr->name, context_ptr->count);


	if ((context_ptr == NULL) || (context_ptr->count > ec->count))
	{
		//no context waiting on this eventcount
		printf("advance returning ctx null\n");
		printf("----\n");
		return;
	}

	//check
	int i = 0;

	/* advance down ec's task list */
	do
	{
		/* set tasks time equal to current time */
		context_ptr->count = etime.count;
		printf("traversing list ctx ptr name %s val %llu\n", context_ptr->name, context_ptr->count);
		context_pptr = context_ptr;
		context_ptr = context_ptr->nextcontext;

		if(!context_ptr)
			printf("ctx ptr is null!\n");

		assert(i == 0);

		i++;

	}while(context_ptr && (context_ptr->count == ec->count));


	context_pptr->nextcontext = etime.nextcontext;
	printf("context_pptr->nextcontext pts to %s\n", context_pptr->nextcontext->name);

	etime.nextcontext = ec->nextcontext;

	printf("etime next %s\n", etime.nextcontext->name);


	//sets list null
	ec->nextcontext = context_ptr;
	if(!ec->nextcontext)
		printf("yes ec->contextlist = null\n");


	printf("----\n");
	return;
}



void eventcount_destroy(eventcount *ec){

	eventcount	*front;
	eventcount	*back;
	unsigned	found = 0;

	assert(ec != NULL);

	// --- This function should unlink the eventcount from the etime eclist ---
	assert(etime.nextec != NULL);

	front = etime.nextec;
	if (ec == front)
	{
		etime.nextec = ec->nextec;
		free ((void*)ec);
		if (ectail == ec)
		{
			ectail = NULL;
		}
	}
	else
	{
	   back = etime.nextec;
	   front = front->nextec;
	   while ((front != NULL) && (!found))
	   {
		   if (ec == front)
		   {
			   back->nextec = ec->nextec;
			   free ((void*)ec);
			   found = 1;
		   }
		   else
		   {
			   back = back->nextec;
			   front = front->nextec;
		   }
	   }

	   assert(found == 1);

	   if (ectail == ec)
	   {
		   ectail = back;
	   }
	}
}


void pause(count_t count){

	printf("pause\n");

	/* if no tasks or still first task, keep running */
	if ((etime.nextcontext == NULL) || ((etime.count + count) <= etime.nextcontext->count))
	{
		printf("no tasks or count is low\n");
		etime.count += count;
	}
	else /* switch to next task */
	{
		printf("awaiting etime.count + count\n");
		printf("---\n");
		await(&etime, etime.count + count);
	}

	return;
}

/* save current task on ec, return next task to run */
void context_find_next(eventcount *ec, count_t value){

	printf("context_find_next\n");
	context *context_pptr = NULL;

	context_pptr = ec->nextcontext;

	/* save current task on ec's tasklist */
	if ((context_pptr == NULL) || (value < context_pptr->count))
	{

		assert(curtask);

		printf("find next ctx null or value low\n");

		//no task or still a task that needs to run
		//the next one is the current task
		ec->nextcontext = curtask;

		//and its next task is what we are pointing too
		curtask->nextcontext = context_pptr;

		/*if(!curtask->nextcontext)
			printf("curtask->contextlist null\n");

		printf("ec %s waiting task %s\n", ec->name, ec->nextcontext->name);*/

	}
	else
	{

		printf("insert into list ctx %s\n", context_pptr->name);

		context *context_ptr = NULL;

		bool time_list = (ec == &etime); //determines if this is etime or not.

		printf("ec is ETIME (time list %d)\n", time_list);

		//time list is 1 if this ec is etime
		if (time_list && hint && (value >= hint->count))
		{
			printf("in hint && value\n");

			context_ptr = hint->nextcontext;
			context_pptr = hint;
		}
		else
		{
			context_ptr = context_pptr->nextcontext;
			printf("ptr (%s) = pptr (%s) pptr next (%s)\n", context_ptr->name, context_pptr->name, context_pptr->nextcontext->name);
		}

		//look down the list for an older tasks (i.e. looking for value < ctx count)
		while (context_ptr && (value >= context_ptr->count))
		{
			context_pptr = context_ptr;
			context_ptr = context_ptr->nextcontext;

			if(!context_ptr)
				printf("context_ptr next = null\n");
		}


		if (curtask)
		{
			printf("yes curtask %s\n", curtask->name);

			context_pptr->nextcontext = curtask;
			curtask->nextcontext = context_ptr;

			if(!curtask->nextcontext)
				printf("curtask name %s no next element\n", curtask->name);
		}

		//working with etime, thus hint is the curtask
		if (time_list)
		{
			hint = curtask;
		}
	}


	//sets when this task should execute next (cycles).
	curtask->count = value;

	printf("curtask name %s count %llu\n", curtask->name, curtask->count);

	/*get next task to run*/

	//change curtask to the next task
	curtask = etime.nextcontext;

	if(context_pptr)
		printf("curtask name %s count %llu ctx pptr name %s count %llu nextname %s count %llu\n",
				curtask->name, curtask->count, context_pptr->name, context_pptr->count, context_pptr->nextcontext->name, context_pptr->nextcontext->count);
	else
		printf("curtask name %s count %llu ctx pptr null\n", curtask->name, curtask->count);

	if (hint == curtask)
	{
		printf("hint == curtask\n");
		hint = NULL;
	}

	//if there is no new task exit
	if (curtask == NULL)
	{
		context_cleanup();

		context_end();
	}

	//the new task should be something that runs in the future.
	assert(curtask->count >= etime.count);

	//moves etime to oldest ctx in list
  	etime.nextcontext = curtask->nextcontext;

  	if(etime.nextcontext)
  		printf("etime.nextcontext name %s count %llu\n", etime.nextcontext->name, etime.nextcontext->count);
  	else
  		printf("yes etime.next null\n");

  	etime.count = curtask->count; //set etime.count to next ctx's count (why?)

  	printf("---\n");
  	return;
}

context *context_select(void){

	printf("context_select\n");

	if (last_ec)
	{
		printf("here1\n");
		printf("---\n");
		context_find_next(last_ec, last_value);
		last_ec = NULL;
	}
	else
	{
		/*curtask points to the next ctx in the ctx list*/
		printf("here2\n");

		//set current task here
		curtask = etime.nextcontext;

		if(curtask)
			printf("curtask name %s\n", curtask->name);

		if (hint == curtask)
		{
			hint = NULL;
		}

		if (curtask == NULL)
		{
			context_cleanup();
			context_end();
		}

		//move down one context
		etime.nextcontext = curtask->nextcontext;
		printf("before et count %llu curtask count %llu\n", etime.count, curtask->count);

		etime.count = curtask->count;
		printf("after et count %llu curtask count %llu\n", etime.count, curtask->count);
	}

	printf("---\n");
	return curtask;
}

void context_next(eventcount *ec, count_t value){

	printf("context next\n");
	context *ctx = NULL;
	last_ec = ec; //these are global because context select doesn't take a variable in
	last_value = value;

	printf("---\n");
	ctx = context_select();
	assert(ctx);

	context_switch(ctx);

	return;
}


/* await(ec, value) -- suspend until ec.c >= value. */
void await (eventcount *ec, count_t value){

	/*todo*/
	/*check for stack overflows*/

	printf("await\n");

	if (ec->count >= value)
	{
		printf("await, count >= value\n");

		printf("---\n");
		return;
	}

	printf("await finding next ctx cur ec %s curtask %s value %llu\n", ec->name, curtask->name, value);

	printf("---\n");
	context_next(ec, value);

	printf("---\n");
	return;
}



void simulate(void){

	//simulate
	if(!context_simulate())
	{
		context *next_context = NULL;

		next_context = context_select();
		assert(next_context);

		context_switch(next_context);

		context_cleanup();
		context_end();
	}

	return;
}







// delete last inserted task
void context_remove_last(context *last_context){

	assert (etime.nextcontext == last_context);
	etime.nextcontext = last_context->nextcontext;
	last_context->nextcontext = NULL;

	return;
}



void context_destroy(context *ctx){

	/* free stack space */
	free(ctx->stack);
}




void context_cleanup(void){

  if (terminated_context)
  {
    context_destroy (terminated_context);
    terminated_context = NULL;
  }

  return;
}




void context_stub(void){

  printf("context stub\n");

  if(terminated_context)
  {
    context_destroy (terminated_context);
    terminated_context = NULL;
  }


  printf("----\n");
  (*current_context->start)();

  /*if current current ctx returns i.e. hits the bottom of its function
  it will return here. Then we need to terminate the process*/

  printf("context stub exiting\n");

  terminated_context = current_context;

  context_switch(context_select());

  return;
}



void context_exit (void){

  terminated_context = current_context;
  context_switch (context_select());
  return;
}


#if defined(__linux__) && defined(__i386__)

void context_switch (context *ctx)
{

	if (!current_context || !setjmp32_2(current_context->buf))
	{
	  current_context = ctx;
	  longjmp32_2(ctx->buf, 1);
	}
	if (terminated_context)
	{
	  context_destroy (terminated_context);
	  terminated_context = NULL;
	}

}

void context_init (context *new_context){

	new_context->buf[5] = ((int)context_stub);
	new_context->buf[4] = ((int)((char*)new_context->stack + new_context->stacksize - 4));
}

void context_end(void){

	longjmp32_2(main_context, 1);
	return;
}

int context_simulate(void){

	return setjmp32_2(main_context);
}

#elif defined(__linux__) && defined(__x86_64)

void context_switch (context *ctx){

	printf("context_switch");

	//setjmp returns 1 if jumping to this position via longjmp
	if (!current_context || !setjmp64_2(current_context->buf))
	{
		/*ok, this is deep wizardry....
		note that the jump is to the next context and the
		setjmp is for the current context*/

		current_context = ctx;
		longjmp64_2(ctx->buf, 1);
	}

	if (terminated_context)
	{
		context_destroy(terminated_context);
		terminated_context = NULL;
	}

	return;
}


void context_init(context *new_context){

	/*these are in this function because they are architecture dependent.
	don't move these out of this function!!!!*/

	new_context->buf[7] = (long long)context_stub; /*where the initial jump will go to*/
	new_context->buf[6] = (long long)((char *)new_context->stack + new_context->stacksize - 4); /*points to top of the virtual stack*/
	return;
}

void context_end(void){

	longjmp64_2(main_context, 1);
	return;
}


int context_simulate(void){

	return setjmp64_2(main_context);
}

#else
#error Unsupported machine/OS combination
#endif
