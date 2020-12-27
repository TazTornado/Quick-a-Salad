#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

typedef struct MemorySegment{

	/* general info; useful to everyone */
	int ID;						// shared memory id
	int remaining_salads;
	
	/* stats for saladmakers */
	int salads_made_by_1;       // counter of salads made by saladmaker1
	int finished_1;             // flag to indicate that saladmaker1 has finished
	int salads_made_by_2;
	int finished_2;
	int salads_made_by_3;
	int finished_3;

	/* IPC and protection of resources */
	sem_t mutex_salads;     		
	sem_t mutex_log;				// concurrent writing in the collective logfile
	sem_t saladmaker1;				// saladmaker1 waits on this
	sem_t saladmaker2;				// saladmaker2 waits on this
	sem_t saladmaker3;				// saladmaker3 waits on this
	sem_t received_ingredients;		// chef waits for a saladmaker to receive ingredients

} MemorySegment;

////////// utilities' prototypes //////////

int *ArgHandler(int, char **, char *);
// char *NewLogDirectory();
MemorySegment *CreateSharedSegment();
int InitializeSemaphores(MemorySegment *);
int *FindCombination();
void Rest(int);
void ChopAndMakeSalad(int, int);