#include <desim.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define LOOP 3
#define LATENCY 4

#define P_TIME (etime->count >> 1)
#define P_PAUSE(p_delay) pause((p_delay)<<1)

#define AWAIT_SUB_CLOCK_0 if (etime->count & 0x1) pause(1)
#define AWAIT_SUB_CLOCK_1 if (!(etime->count & 0x1)) pause(1)

//which input has priority
#define AORB 1

eventcount *ec_prod_a;
eventcount *ec_prod_b;
eventcount *ec_a;
eventcount *ec_c;
eventcount *end;

void producer_a(void);
void producer_b(void);
void arbiter(void);
void consumer(void);

int producer_a_request = 0;
int producer_b_request = 0;
int arbiter_busy = 0;
int consumer_busy = 0;


int main(void){

	char buff[100];


	//user must initialize DESim
	desim_init();

	//create the user defined eventcounts

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "ec_prod_a");
	ec_prod_a = eventcount_create(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "ec_prod_b");
	ec_prod_b = eventcount_create(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "ec_a");
	ec_a = eventcount_create(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "ec_c");
	ec_c = eventcount_create(strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "end");
	end = eventcount_create(strdup(buff));

	printf("Event counts created\n");

	//create the user defined contexts
	//l1 i caches
	memset(buff,'\0' , 100);
	snprintf(buff, 100, "producer_a");
	context_create(producer_a, 32768, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "producer_b");
	context_create(producer_b, 32768, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "arbiter");
	context_create(arbiter, 32768, strdup(buff));

	memset(buff,'\0' , 100);
	snprintf(buff, 100, "consumer");
	context_create(consumer, 32768, strdup(buff));
	printf("Contexts created\n");

	/*starts simulation and won't return until simulation
	is complete or all contexts complete*/
	printf("Simulate %d interactions\n", LOOP);
	simulate();
	printf("End simulation\n");

	return 1;
}

void producer_a(void){

	//use this to determine which producer you are

	printf("producer_a:\n\t init\n");
	printf("\t advancing ec_a cycle %llu\n", CYCLE);
	advance(ec_a);
	printf("\t advanced and doing work cycle %llu\n", CYCLE);

	/**********do work here***********/
	producer_a_request = 1;

	pause(LATENCY);

	printf("producer_a:\n");
	printf("\t resuming from latency cycle %llu\n", CYCLE);

	printf("\t pausing cycle %llu\n", CYCLE);
	P_PAUSE(1);
	printf("\t producer_a: \n");
	printf("\t awaiting end cycle %llu\n", CYCLE);

	/*this would not be needed in a real simulator
	this just controls when simulation ends*/
	await(end, 2);

	printf("\t producer_a exiting and simulation ending cycle %llu\n", CYCLE);

	return;
}

void producer_b(void){

	//use this to determine which producer you are

	printf("producer_b:\n\t init\n");
	printf("\t advancing ec_a cycle %llu\n", CYCLE);
	advance(ec_a);
	printf("\t advanced and doing work cycle %llu\n", CYCLE);

	/**********do work here***********/
	producer_b_request = 1;

	pause(LATENCY);

	printf("producer_b:\n");
	printf("\t resuming from latency cycle %llu\n", CYCLE);

	printf("\t pausing cycle %llu\n", CYCLE);
	P_PAUSE(1);
	printf("\t producer_b: \n");
	printf("\t awaiting end cycle %llu\n", CYCLE);

	/*this would not be needed in a real simulator
	this just controls when simulation ends*/
	await(end, 2);

	printf("\t producer_b exiting and simulation ending cycle %llu\n", CYCLE);

	return;
}

void arbiter(void){

	count i = 1;

	printf("arbiter:\n\t init\n");
	while(1)
	{

		if(consumer_busy)
		{
			//if the consumer is busy wait.
			printf("arbiter:\n\t Waiting cycle %llu\n", CYCLE);
			pause(1);
		}
		else
		{
			//consumer isn't busy now

			//await work
			printf("\t await ec_a cycle %llu\n", CYCLE);
			await(ec_a, i);
			i++;

			//enter sub clock domain and do work
			AWAIT_SUB_CLOCK_1;

			printf("arbiter:\n\t We have one or more advances in a given cycle. Arbiter\n"
					"\t will now choose one of the messages and provide to consumer\n"
					"\t advanced and doing work cycle %llu\n", CYCLE);

			assert(producer_a_request == 1 || producer_b_request == 1);
			printf("p_a %d p_b %d\n", producer_a_request, producer_b_request);
			if(AORB)
			{
				if(producer_a_request)
					producer_a_request--;
				else
					producer_b_request--;
			}
			else
			{
				if(producer_b_request)
					producer_b_request--;
				else
					producer_a_request--;
			}

			AWAIT_SUB_CLOCK_0;
			printf("arbiter:\n");

			/*advance producer ctx*/
			printf("\t advancing ec_c cycle %llu\n", CYCLE);
			advance(ec_c);
		}
	}

	fatal("arbiter should never exit cycle %llu\n", CYCLE);

	return;
}


void consumer(void){

	count i = 1;

	printf("consumer:\n\t init\n");
	while(1)
	{
		//await work
		printf("\t await ec_c cycle %llu\n", CYCLE);
		await(ec_c, i);
		i++;
		printf("consumer:\n\t advanced and doing work cycle %llu\n", CYCLE);

		consumer_busy = 1;

		/**********do work here***********/

		//charge latency
		printf("\t charging latency %d cycle %llu\n", LATENCY, CYCLE);
		pause(LATENCY);

		printf("consumer:\n");
		printf("\t resuming from latency cycle %llu\n", CYCLE);

		consumer_busy = 0;
		advance(end);
	}

	fatal("consumer should never exit cycle %llu\n", CYCLE);

	return;
}
