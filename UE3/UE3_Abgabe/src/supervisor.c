/**
 * @file supervisor.c
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 12.01.19
 *
 * @brief Supervisor program module.
 * 
 * This program sets up shared memory and reads solutions to the 3-colour problem provided by a generator.
 **/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <time.h>
#include <signal.h>
#include "shm.h"

static int best_sol = BUF_LEN+1; /**< Number of edges in the best solution so far */

sem_t *free_sem;	/**< Tracks free space in the circular buffer 	*/
sem_t *used_sem;	/**< Tracks used space in the circular buffer 	*/
sem_t *mutex;		/**< Ensures that only one generator at a time has write access */
struct shm *myshm;	/**< Shared memory 	*/

static char *pgm_name; /**< Program name */

static void circ_buf_read(struct edge *sol);

static void cleanup(void);

static void setup_shm(void);

/**
 * Signal handler.
 * @brief This function handles SIGINT and SIGTERM and signals the generators to terminate.
 * @details global variables: pgm_name
 */
static void sig_handler(int signo){
	if(signo == SIGINT || signo == SIGTERM){
		printf("\nTerminating...\n");
		myshm->term = 1;
		//sem_post ensures that if every generator is waiting to write at least one can
		//continue and thus every generator can terminate one by one
		sem_post(free_sem);
		cleanup();
		exit(EXIT_SUCCESS);
	}
}

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void){
	fprintf(stderr, "USAGE: %s\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief This function repeatedly calls the reading function to read solutions to provided by a generator.
 * @details
 * global variables: pgm_name, free_sem, used_sem, mutex, myshm
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_FAILURE if there is an error, otherwise sig_handler() or circ_buf_read() will terminate the program.
 */
int main(int argc, char *argv[]){	
	pgm_name = argv[0];
	
	if(argc > 1){
		usage();
	}

	if(signal(SIGINT, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGTERM\n");
	}

	setup_shm();

	//Instructions said that supervisor should remember the best solution so far
	struct edge sol;

	while(1){
		circ_buf_read(&sol);
	}

	fprintf(stderr, "%s: Should not be reached\n", pgm_name);
	cleanup();
	return EXIT_FAILURE;
}

/**
 * SHM-Setup funciton
 * @brief This function sets up the shared memory and semaphores
 */
static void setup_shm(void){
	//Set up shared memory	
	int shmfd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0600);
	if(shmfd < 0){
		fprintf(stderr, "%s: Error setting up shared memory\n", pgm_name);
		close(shmfd);
		shm_unlink(SHM_NAME);
		exit(EXIT_FAILURE);
	}

	//Set size
	if(ftruncate(shmfd, sizeof(struct shm)) < 0){
		fprintf(stderr, "%s: Error setting size of memory\n", pgm_name);
		close(shmfd);
		shm_unlink(SHM_NAME);
		exit(EXIT_FAILURE);
	}

	//Memory mapping
	myshm = mmap(NULL, sizeof(*myshm), PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
	if(myshm == MAP_FAILED){
		fprintf(stderr, "%s: Error mapping memory\n", pgm_name);
		close(shmfd);
		shm_unlink(SHM_NAME);
		exit(EXIT_FAILURE);
	}

	//Initialize struct members
	myshm->term = 0;
	myshm->write_pos = 0;	

	//After mapping, shmfd can be closed
	if(close(shmfd) < 0){
		fprintf(stderr, "%s: Error closing file descriptor\n", pgm_name);
		munmap(myshm, sizeof(*myshm));
		shm_unlink(SHM_NAME);
		exit(EXIT_FAILURE);
	}

	//Create semaphores
	used_sem = sem_open(SEM_US, O_CREAT | O_EXCL, 0600, 0);
	free_sem = sem_open(SEM_FS, O_CREAT | O_EXCL, 0600, MAX_SIZE);
	mutex = sem_open(SEM_EX, O_CREAT | O_EXCL, 0600, 1);
	if(mutex == SEM_FAILED || used_sem == SEM_FAILED || free_sem == SEM_FAILED){
		fprintf(stderr, "%s: Failed to create semaphores\n", pgm_name);
		cleanup();
		exit(EXIT_FAILURE);
	}

}

static int read_pos = 0; /**< Reading position in the circular buffer */

/**
 * Reading function
 * @brief This function reads a solution from the circular buffer.
 * @details
 * @param sol Struct that contains 2 vertices and represents an edge.
 */
static void circ_buf_read(struct edge *sol) {
	sem_wait(used_sem);
	struct edge *tmp = &myshm->data[read_pos];
	if(tmp->n[0] == -1){
		printf("Graph is 3-colourable\n");
		sem_post(free_sem);
		myshm->term = 1;
		cleanup();
		exit(EXIT_SUCCESS);
	}

	for(int i = 0; i<BUF_LEN; i++){
		if(tmp->n[i] == -1){
			if(i<best_sol){
				best_sol = i;
				printf("Solution with %i edges: ", i);
				for(int j = 0; j<i; j++){
					printf("%i-%i ", tmp->n[j], tmp->m[j]);
				}
				printf("\n");
				sol = tmp;
				sem_post(free_sem);
				read_pos = (read_pos + 1) % MAX_SIZE;
				return;
			}
		}
	}	
	
	if(best_sol>BUF_LEN){
		best_sol = BUF_LEN;
		printf("Solution with %i edges: ", BUF_LEN);
		for(int j = 0; j<BUF_LEN; j++){
					printf("%i-%i ", tmp->n[j], tmp->m[j]);
				}
				printf("\n");
				sol = tmp;
				sem_post(free_sem);
				read_pos = (read_pos + 1) % MAX_SIZE;
				return;
	}
	sem_post(free_sem);
	read_pos = (read_pos + 1) % MAX_SIZE;
}

/**
 * Cleanup function
 * @brief This function reads a solution from the circular buffer.
 * @details
 * @param sol Struct that contains 2 vertices and represents an edge.
 */
static void cleanup(void){
	munmap(myshm, sizeof(*myshm));
	shm_unlink(SHM_NAME);
	sem_close(used_sem);
	sem_close(free_sem);
	sem_close(mutex);
	sem_unlink(SEM_US);
	sem_unlink(SEM_FS);
	sem_unlink(SEM_EX);
}

