#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#include "utilities.h"

#define SHMSIZE sizeof(MemorySegment)


////////////////////// Common functions for Chef and Saladmakers //////////////////////


/*
	Serves as the argument handler for both the Chef and the Saladmakers.
	It takes argc and argv of the invoking process and a string that defines
	whose arguments to handle.
	Returns a vector of the argument values, without the flags.
*/

int *ArgHandler(int argc, char **argv, char* behavior){

	/////// BEHAVIOR : CHEF ///////
	if(strcmp(behavior, "chef") == 0){

		int *args = malloc(2*sizeof(int));	// argument vector to return 
		if(args == NULL){
			perror("ArgHandler -> malloc error ");
			return NULL;
		} 		
		int nflag = 0, mflag = 0;       	// flags to check if both inline parameters were provided

		while(--argc){ 
			if(strcmp(*argv, "-n") == 0){		// numOfSlds
				if(nflag == 0){
					args[0] = atoi(*(argv + 1));
					nflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./chef -n numOfSlds -m mantime\n\n");
					free(args);
					return NULL;
				}
			}
			if(strcmp(*argv, "-m") == 0){		// mantime
				if(mflag == 0){
					args[1] = atoi(*(argv + 1));
					mflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./chef -n numOfSlds -m mantime\n\n");
					free(args);
					return NULL;
				}
			}

			argv++;
		}

		if((nflag != 1) || (mflag != 1)){		
			printf("Usage: ./chef -n numOfSlds -m mantime\nAll parameters are mandatory, flexible order.\n");
			free(args);
			return NULL;
		}

		return args;

	/////// BEHAVIOR : SALADMAKER /////// 
	} else if(strcmp(behavior, "saladmaker") == 0){
		int *args = malloc(5*sizeof(int));	// argument vector to return 
		int lflag = 0, uflag = 0, shmflag = 0, iflag = 0;
		

		while(--argc){ 
			if(strcmp(*argv, "-t1") == 0){		// lb
				if(lflag == 0){
					args[0] = atoi(*(argv + 1));
					lflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./saladmaker -t1 lb -t2 ub -s shmid -i index\n\n");
					free(args);
					return NULL;
				}
			}
			if(strcmp(*argv, "-t2") == 0){		// ub
				if(uflag == 0){
					args[1] = atoi(*(argv + 1));
					uflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./saladmaker -t1 lb -t2 ub -s shmid -i index\n\n");
					free(args);
					return NULL;
				}
			}
			if(strcmp(*argv, "-s") == 0){		// shmid
				if(shmflag == 0){
					args[2] = atoi(*(argv + 1));
					shmflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./saladmaker -t1 lb -t2 ub -s shmid -i index\n\n");
					free(args);
					return NULL;
				}
			}
			if(strcmp(*argv, "-i") == 0){		// index of saladmaker
				if(iflag == 0){
					args[3] = atoi(*(argv + 1));
					iflag = 1;
				} else {
					printf("Invalid argument.\nUsage: ./saladmaker -t1 lb -t2 ub -s shmid -i index\n\n");
					free(args);
					return NULL;
				}
			}

			argv++;
		}
		if((lflag != 1) || (uflag != 1) || (shmflag != 1) || (iflag != 1)){
			printf("Usage: ./saladmaker -t1 lb -t2 ub -s shmid -i index -i index\nAll parameters are mandatory, flexible order.\n");
			free(args);
			return NULL;
		}
		if(args[1] < args[0]){
			printf("Upper bound must be greater than lower bound!\n");
			printf("Usage: ./saladmaker -t1 lb -t2 ub -s shmid -i index\n");
			free(args);
			return NULL;
		}

		return args;

	} else{
		printf("ArgHandler: Error in behavior definition.\n");
		printf("Behavior options: 'chef' or 'saladmaker'\n");
		return NULL;
	}
}



////////////////////// Functions used by Chef //////////////////////

/*
char *NewLogDirectory(){
	char *logs_path = malloc(25*sizeof(char));			// arbitrary size to fit sth like "../logs/version<pid>"
	if(logs_path == NULL){
		perror("NewLogDirectory -> malloc error ");
		return NULL;
	} 
	sprintf(logs_path, "./logs/version%d", getpid());	// log directory's name
	// printf("about to mkdir %s\n", logs_path);
	int result = mkdir(logs_path, S_IRUSR | S_IWUSR | S_IXUSR);    	// create a new log directory for current execution
	if(result != 0)
		perror("Error in mkdir");

	return logs_path;
}
*/


/*	
	This is only utilized by the Chef. Saladmakers will only attach, manually
	Calls ftok to acquire a key, then shmget() and shmat()
	to create and attach a memory segment of size equal to 1 struct MemorySegment.
	Returns a pointer to the segment.
*/
MemorySegment *CreateSharedSegment(){
	int shm_id;
	key_t shm_key;
	MemorySegment *shm_ptr;			// once attached, this is the return value

	shm_key = ftok(".", 'a');		// generate key for shmem

	shm_id = shmget(shm_key, SHMSIZE, IPC_CREAT | 0666);	// create shmem segment with the size of MemorySegment struct
	if(shm_id < 0){
		perror("CreateSharedSegment -> shmget error ");
		return NULL;
	} 

	shm_ptr = (MemorySegment *)shmat(shm_id, NULL, 0);
	if(shm_ptr == (void *)-1){
		perror("CreateSharedSegment -> shmat error ");
		return NULL;
	}

	shmctl(shm_id, IPC_RMID, NULL);		// mark segment to be destroyed at the end
	shm_ptr->ID = shm_id;

	return shm_ptr;
}



/*
	Name says it. This function gets a pointer to the shmem segment
	and initializes the semaphores according to their intended use.
	Returns 0 for success, -1 if something fails.
*/

int InitializeSemaphores(MemorySegment *shared_memory){

	int retval = 0;
	
	retval = sem_init(&(shared_memory->mutex_salads), 1, 1);	// init to 1 => 1st to wait, should not be suspended
	if(retval < 0){
		perror("Error initializing sem mutex_salads ");
		return retval;
	}

	retval = sem_init(&(shared_memory->mutex_log), 1, 1);		// init to 1 => 1st to wait, should not be suspended
	if(retval < 0){
		perror("Error initializing sem mutex_log ");
		return retval;
	}

	retval = sem_init(&(shared_memory->saladmaker1), 1, 0);		// init to 0 => saladmaker should be suspended until Chef wakes him
	if(retval < 0){
		perror("Error initializing sem pepper_tomato ");
		return retval;
	}

	retval = sem_init(&(shared_memory->saladmaker2), 1, 0);		// init to 0 => saladmaker should be suspended until Chef wakes him
	if(retval < 0){
		perror("Error initializing sem pepper_onion ");
		return retval;
	}

	retval = sem_init(&(shared_memory->saladmaker3), 1, 0);		// init to 0 => saladmaker should be suspended until Chef wakes him
	if(retval < 0){
		perror("Error initializing sem onion_tomato ");
		return retval;
	}

	retval = sem_init(&(shared_memory->received_ingredients), 1, 0);	// init to 0 => chef gets suspended until Saladmaker sends feedback
	if(retval < 0){
		perror("Error initializing sem received_ingredients ");
		return retval;
	}

	return retval;
}



/*
	Decides in a pseudo-random way, which saladmaker will have each ingredient.
	There are 6 possible combinations: (nPk) where n is the number of saladmakers
	and k is the number of different vegetable pairs. Placement matters.
	Returns a vector of the chosen indices that correspond to the ingredients.
	1st is always the one who has tomatoes, then peppers, then onions.
*/

int *FindCombination(){

	srand(time(0));
	int has_tomato, has_pepper, has_onion;
	int *combination = malloc(3*sizeof(int));	// vector to return
	int result = rand() % 108;					// will be divided into 6 combinations

	if(result < 18){
		has_tomato = 1;
		has_pepper = 2;
		has_onion = 3;

	} else if(result < 36){
		has_tomato = 1;
		has_pepper = 3;
		has_onion = 2;

	} else if(result < 54){
		has_tomato = 2;
		has_pepper = 1;
		has_onion = 3;

	} else if(result < 72){
		has_tomato = 2;
		has_pepper = 3;
		has_onion = 1;

	} else if(result < 90){
		has_tomato = 3;
		has_pepper = 1;
		has_onion = 2;

	} else{
		has_tomato = 3;
		has_pepper = 2;
		has_onion = 1;

	}

	combination[0] = has_tomato;
	combination[1] = has_pepper;
	combination[2] = has_onion;

	return combination;

}



/*
	A bit of a dummy method. It's utilized to demonstrate 
	the Chef's resting time more intuitively.
*/
void Rest(int mantime){
	sleep(mantime);
}

////////////////////// Functions used by Saladmakers //////////////////////


/*
	A bit of a dummy method. It's utilized to demonstrate 
	the Saladmakers' vegetable-chopping-and-saladmaking time more intuitively.
	It chooses randomly a sleep time between lb and ub as given to Saladmakers.
*/
void ChopAndMakeSalad(int lb, int ub){

	int worktime = (rand() % (ub - lb + 1)) + lb;
	sleep(worktime);
}