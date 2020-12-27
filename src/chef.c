#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "utilities.h"

#define GO 1


/*
	Run executable: ./bin/chef -n numOfSlds -m mantime
	
	-n and -m in any order
*/


int main(int argc, char **argv){
	
////////////////////* Get program parameters and make checks *////////////////////

	int numOfSlds, mantime;
	int *my_args = ArgHandler(argc, argv, "chef");

	if(my_args != NULL){
		numOfSlds = my_args[0];
		mantime = my_args[1];
		free(my_args);

	} else {
		printf("Chef [%d] failed to acquire parameters. Exiting now..\n", getpid());
		exit(EXIT_FAILURE);
	}

	/* feedback message */
	// printf("\nHELLO, CHEF [%d] CLOCKING IN with mantime %d and salads# %d. \n", getpid(), mantime, numOfSlds);


///////////////////* Initialization of Logging *///////////////////
	
	FILE *collective_log = fopen("./logs/collective.log", "a+");
	if(collective_log == NULL){
		printf("Chef [%d] failure in fopen. Exiting..\n", getpid());
		exit(EXIT_FAILURE);
	}


//////////////////* Initialization of Shared Memory and Semaphores *///////////////////

	MemorySegment *shared_memory = CreateSharedSegment();
	if(shared_memory == NULL){
		printf("Chef [%d] failure. Exiting..\n", getpid());
		exit(EXIT_FAILURE);
	}

	shared_memory->remaining_salads = numOfSlds;	// init salads to be made
	// printf("TESTING: %d\n", shared_memory->remaining_salads);
	
	// 		  Init Semaphores		//
	//------------------------------//

	int retval = InitializeSemaphores(shared_memory);
	if (retval < 0){
		perror("Chef InitializeSemaphores error ");
		exit(EXIT_FAILURE);
	}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	/* Assign the Saladmakers their ingredient baskets */
	int has_tomato, has_pepper, has_onion;
	int *combo = FindCombination();

	/* keep Saladmaker indices as chosen */
	has_tomato = combo[0];
	has_pepper = combo[1];
	has_onion = combo[2];
	free(combo);


	/* Print useful info in stdout*/
	printf("\n\nShared Memory ID: %d\n", shared_memory->ID);
	printf("Saladmaker%d => tomatoes \nSaladmaker%d => peppers \nSaladmaker%d => onions.\n\n", has_tomato, has_pepper, has_onion);


	/* Initial prints to logfile with simulation's basic info */
	retval = fprintf(collective_log, "Today's order: %d salads.\n", numOfSlds);
	if(retval < 0){
		perror("Chef -> fprintf failed ");
		exit(EXIT_FAILURE);
	}
	retval = fprintf(collective_log, "Saladmaker%d has tomatoes, Saladmaker%d has peppers and Saladmaker%d has onions.\n", has_tomato, has_pepper, has_onion);
	if(retval < 0){
		perror("Chef -> fprintf failed ");
		exit(EXIT_FAILURE);
	}
	retval = fprintf(collective_log, "------------------------------------------------------------------------------------------\n");
	if(retval < 0){
		perror("Chef -> fprintf failed ");
		exit(EXIT_FAILURE);
	}

	/* Prepare for getting times to write in log */
	struct timeval  now;
	struct tm* local;

	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	/* Initial Chef's message */
	fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Chef] "
	"[Clocking in]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid());

////////////////////////////////////* WORK WORK WORK! *////////////////////////////////////
	int goflag = 1;
	int ingredient;
	
	while((goflag == GO) && !((shared_memory->finished_1 == 1) && (shared_memory->finished_2 == 1) && (shared_memory->finished_3 == 1))){
		ingredient = rand() % 100;
		if(ingredient < 34){ // onion => tomato + pepper selected
			
			/////////////* Write in log *////////////
			sem_wait(&(shared_memory->mutex_log));

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Selecting ingerdients tomato pepper]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid());

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Notifying Saladmaker #%d]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid(), has_onion);

			sem_post(&(shared_memory->mutex_log));
			/////////////////////////////////////////

			switch(has_onion){		// notify the right Saladmaker
				case 1:
					sem_post(&(shared_memory->saladmaker1));
					break;
				case 2:
					sem_post(&(shared_memory->saladmaker2));
					break;
				case 3:
					sem_post(&(shared_memory->saladmaker3));
					break;
			}


		} else if(ingredient < 68){			// tomato => pepper + onion selected
			
			/////////////* Write in log *////////////
			sem_wait(&(shared_memory->mutex_log));

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Selecting ingerdients onion pepper]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid());

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Notifying Saladmaker #%d]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid(), has_tomato);

			sem_post(&(shared_memory->mutex_log));
			/////////////////////////////////////////

			switch(has_tomato){		// notify the right Saladmaker
				case 1:
					sem_post(&(shared_memory->saladmaker1));
					break;
				case 2:
					sem_post(&(shared_memory->saladmaker2));
					break;
				case 3:
					sem_post(&(shared_memory->saladmaker3));
					break;
			}
			
		} else {		// pepper => tomato + onion selected
			
			/////////////* Write in log *////////////
			sem_wait(&(shared_memory->mutex_log));

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Selecting ingerdients tomato onion]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid());

			gettimeofday(&now, NULL);
			local = localtime(&now.tv_sec);

			fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
			"[%d] "
			"[Chef] "
			"[Notifying Saladmaker #%d]\n", 
			local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
			getpid(), has_pepper);

			sem_post(&(shared_memory->mutex_log));
			/////////////////////////////////////////

			switch(has_pepper){		// notify the right Saladmaker
				case 1:
					sem_post(&(shared_memory->saladmaker1));
					break;
				case 2:
					sem_post(&(shared_memory->saladmaker2));
					break;
				case 3:
					sem_post(&(shared_memory->saladmaker3));
					break;
			}
		}
		
		sem_wait(&(shared_memory->received_ingredients));

		/////////////* Write in log *////////////
		sem_wait(&(shared_memory->mutex_log));

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Chef] "
		"[Man time for resting]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid());

		sem_post(&(shared_memory->mutex_log));
		/////////////////////////////////////////

		Rest(mantime);

		sem_wait(&(shared_memory->mutex_salads));
		if(shared_memory->remaining_salads == 0)
			goflag = 0;
		sem_post(&(shared_memory->mutex_salads));
	}


	/////////////* Write in log *////////////
	sem_wait(&(shared_memory->mutex_log));

	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Chef] "
	"[Clocking out]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid());

	sem_post(&(shared_memory->mutex_log));
	/////////////////////////////////////////


	/* Print out stats of simulation */
	printf("Total salads made: %d\n", shared_memory->salads_made_by_1 + shared_memory->salads_made_by_2 + shared_memory->salads_made_by_3);
	printf("Saladmaker #1 made %d salads.\n", shared_memory->salads_made_by_1);
	printf("Saladmaker #2 made %d salads.\n", shared_memory->salads_made_by_2);
	printf("Saladmaker #3 made %d salads.\n", shared_memory->salads_made_by_3);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	retval = shmdt(shared_memory);
	if(retval < 0){
		perror("Chef shmdt error ");
		exit(EXIT_FAILURE);
	} 
	fclose(collective_log);
	exit(EXIT_SUCCESS);
}