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


/*
    Run executable: ./bin/saladmaker -t1 lb -t2 ub -s shmid -i index
    
    -t1, -t2, -s, -i in any order
*/

int main(int argc, char **argv){

////////////////////* Get program parameters and make checks *////////////////////

	int ub, lb, shmid = 0, index;
	int *my_args = ArgHandler(argc, argv, "saladmaker");

	if(my_args != NULL){
		ub = my_args[0];
		lb = my_args[1];
		shmid = my_args[2];
		index = my_args[3];
		
		free(my_args);

	} else {
		printf("Saladmaker [%d] failed to acquire parameters. Exiting now..\n", getpid());
		exit(EXIT_FAILURE);
	}

////////////////////////* Attach Shared Memory Segment *//////////////////////////
	
	MemorySegment *sh_mem = (MemorySegment *)shmat(shmid, NULL, 0);
	if(sh_mem == (void *)-1){
		perror("Saladmaker -> shmat error ");
		exit(EXIT_FAILURE);
	}
	

	int *salads_made_by_me = NULL;
	int *finished = NULL;
	sem_t *my_semaphore;

	switch(index){
		case 1:		// This is saladmaker #1
			salads_made_by_me = &(sh_mem->salads_made_by_1);
			finished = &(sh_mem->finished_1);
			my_semaphore = &(sh_mem->saladmaker1);
			break;

		case 2:		// This is saladmaker #2
			salads_made_by_me = &(sh_mem->salads_made_by_2);
			finished = &(sh_mem->finished_2);
			my_semaphore = &(sh_mem->saladmaker2);
			break;

		case 3:		// This is saladmaker #3
			salads_made_by_me = &(sh_mem->salads_made_by_3);
			finished = &(sh_mem->finished_3);
			my_semaphore = &(sh_mem->saladmaker3);
			break;

	}
	*salads_made_by_me = 0;
	*finished = 0;

    // printf("\nHELLO, SALADMAKER%d [%d] CLOCKING IN\n", index, getpid());
	// printf("My variables in shared memory: %d, %d, %d\n", sh_mem->salads_made_by_1, *finished, sh_mem->remaining_salads);


////////////////////////* Initialize my log *//////////////////////////

	char *my_log_name = malloc(strlen("./logs/saladmakerXXXXXXXXXX.log")*sizeof(char) + 1);		// 10 XXXXXX..XX digits for this pid
	if(my_log_name == NULL){
		printf("Saladmaker [%d] failure in malloc. Exiting..\n", getpid());
		exit(EXIT_FAILURE);
	}
	sprintf(my_log_name, "./logs/saladmaker%d.log", getpid());
	

	/* Open my log */
	FILE *mylog = fopen(my_log_name, "a+");
	if(mylog == NULL){
		printf("Saladmaker [%d] failure in fopen. Exiting..\n", getpid());
		exit(EXIT_FAILURE);
	}

	/* Open shared log */
	FILE *collective_log = fopen("./logs/collective.log", "a");
	if(collective_log == NULL){
		printf("Saladmaker [%d] failure in fopen. Exiting..\n", getpid());
		exit(EXIT_FAILURE);
	}

//////////////////////////////////////////////////////////////////////////////////

	struct timeval  now;
	struct tm* local;

	//////////* Write in my log *////////////
	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Saladmaker%d] "
	"[Clocking in]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid(), index);


	///////////* Write in shared log *////////////
	sem_wait(&(sh_mem->mutex_log));

	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Saladmaker%d] "
	"[Clocking in]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid(), index);

	sem_post(&(sh_mem->mutex_log));
	/////////////////////////////////////////

////////////////////////////////////* WORK WORK WORK! *////////////////////////////////////

	int goflag = 1;

	while(goflag != 0){

		///////////* Write in my log *////////////
		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Waiting for ingredients]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		///////////* Write in shared log *////////////
		sem_wait(&(sh_mem->mutex_log));

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Waiting for ingredients]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		sem_post(&(sh_mem->mutex_log));
		/////////////////////////////////////////

		sem_wait(my_semaphore);
		sem_post(&(sh_mem->received_ingredients));


		///////////* Write in my log *////////////
		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Received ingredients]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Start making salad]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);


		///////////* Write in shared log *////////////
		sem_wait(&(sh_mem->mutex_log));

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Received ingredients]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Start making salad]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		sem_post(&(sh_mem->mutex_log));
		/////////////////////////////////////////

		ChopAndMakeSalad(lb, ub);

		////////////* Write in my log *////////////
		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Finished making salad]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);
		
		///////////* Write in shared log *////////////
		sem_wait(&(sh_mem->mutex_log));

		gettimeofday(&now, NULL);
		local = localtime(&now.tv_sec);

		fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
		"[%d] "
		"[Saladmaker%d] "
		"[Finished making salad]\n", 
		local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
		getpid(), index);

		sem_post(&(sh_mem->mutex_log));
		/////////////////////////////////////////


		// check if the order is done
		sem_wait(&(sh_mem->mutex_salads));
		if(sh_mem->remaining_salads == 0)
			goflag = 0;
		sem_post(&(sh_mem->mutex_salads));

		if(goflag == 0)
			break;

		///////////* Modify remaining salads *////////////
		sem_wait(&(sh_mem->mutex_salads));
		sh_mem->remaining_salads -= 1;
		sem_post(&(sh_mem->mutex_salads));
		//////////////////////////////////////////////////

		*salads_made_by_me += 1;	// made another salad!
	}

	*finished = 1;		// done. let the chef know

	///////////* Write in my log *////////////
	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	fprintf(mylog, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Saladmaker%d] "
	"[Clocking out]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid(), index);
		
	///////////* Write in shared log *////////////
	sem_wait(&(sh_mem->mutex_log));

	gettimeofday(&now, NULL);
	local = localtime(&now.tv_sec);

	fprintf(collective_log, "[%02d/%02d/%02d, %02d:%02d:%02d.%03ld] "
	"[%d] "
	"[Saladmaker%d] "
	"[Clocking out]\n", 
	local->tm_mday, local->tm_mon+1, local->tm_year+1900, local->tm_hour, local->tm_min, local->tm_sec, now.tv_usec / 1000, 
	getpid(), index);

	sem_post(&(sh_mem->mutex_log));
	/////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////
	int retval = shmdt(sh_mem);
	if(retval < 0){
		perror("Saladmaker shmdt error ");
		exit(EXIT_FAILURE);
	} 
	free(my_log_name);
	fclose(collective_log);
	fclose(mylog);
    exit(EXIT_SUCCESS);
}