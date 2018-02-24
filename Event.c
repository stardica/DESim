#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string.h>

#include <cstdlib>
#include <ctime>

#include <knightsim.h>
#include <rdtsc.h>

#define LATENCY 16


#define EVENTS 813
#define TOTAL_CYCLES 1000000


#define STACKSIZE 16384

void event(void);
void event_init(void);

long long p_pid = 0;

unsigned long long sim_start = 0;
unsigned long long sim_time = 0;

int iters = 0;


int main(void){

	//user must initialize DESim
	KnightSim_init();

	event_init();

	/*starts simulation and won't return until simulation
	is complete or all contexts complete*/

	printf("KnightSim Event: Simulating %d events @ %d total\n", EVENTS, TOTAL_CYCLES);

	sim_start = rdtsc();

	simulate();

	sim_time += (rdtsc() - sim_start);

	//clean up
	KnightSim_clean_up();

	printf("End simulation time %llu cycles %llu pairs %d iters %d\n", sim_time, CYCLE, EVENTS, iters);

	return 1;
}

void event_init(void){

	int i = 0;
	char buff[100];

	//create the user defined contexts
	for(i = 0; i < EVENTS; i++)
	{
		memset(buff,'\0' , 100);
		snprintf(buff, 100, "events_%d", i);
		context_create(event, STACKSIZE, strdup(buff), i);
	}

	return;
}


void event(void){

	//int my_pid = p_pid++;
	/*count_t i = 1;*/
	context_init_halt();

	while(CYCLE < TOTAL_CYCLES)
	{
		//do work
		//iters++;

		//printf("event %d cycle %llu\n", my_pid, CYCLE);
		pause(1);



	}

	//context will terminate
	return;
}
