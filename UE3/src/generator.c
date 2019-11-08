/**
 * @file generator.c
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 12.01.19
 *
 * @brief Generator program module.
 * 
 * This program generates random solutions to the 3-colour problem.
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
#include "shm.h"

#define MAX_VERTICES 3		/**<Maximal length of a vertex index (-> Max. 1000 vertices)*/

static char *pgm_name; 		/**< Program name */

sem_t *free_sem;		/**< Tracks free space in the circular buffer 	*/
sem_t *used_sem;		/**< Tracks used space in the circular buffer 	*/
sem_t *mutex;			/**< Ensures that only one generator at a time has write access */
struct shm *myshm;		/**< Shared memory 	*/

static void setup_shm(void);

static int parseEdge(int pos, char *edge, int **edgelist);

static void randColour(int msize, int *clist);

static int deleteEdges(int msize, int *clist, int **matrix);

static void circ_buf_write(struct edge sol);

static void cleanup(void);

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void){
	fprintf(stderr, "USAGE: %s EDGE1 EDGE2 ...\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief This function repeatedly generates random colourings for a given graph and writes the result to a
 * circular buffer.
 * @details
 * global variables: pgm_name, free_sem, used_sem, mutex, myshm
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS on successful termination, otherwise EXIT_FAILURE.
 */
int main(int argc, char *argv[]){
	pgm_name = argv[0];
	char c;
	
	if(argc<2){
		usage();
	}
	while ((c = getopt(argc, argv, "")) != -1){
		switch(c){
			case '?':
				usage();
				break;
		}
	}

	setup_shm();
	
	//Seed RNG
	srand(time(NULL));

	while(1){	
		
		//Write all edges to edgelist
		int **edgelist = (int **)malloc((argc-1)*sizeof(int*));
		for(int i = 0; i<(argc-1); i++){
			edgelist[i] = (int *)malloc(2*sizeof(int));
		}

		for(int i = 0; i<argc-1; i++){
			if(parseEdge(i, argv[i+1], edgelist)<0){
				cleanup();
				free(edgelist);
				exit(EXIT_FAILURE);
			}
		}

		//Find max in edgelist (size of matrix)
		int maxv = 0;
		for(int i = 0; i<argc-1; i++){
			if(edgelist[i][0]>maxv){
				maxv = edgelist[i][0];
			}
			if(edgelist[i][1]>maxv){
				maxv = edgelist[i][1];
			}
		}

		//matrix is msize x msize
		int msize = maxv+1;

		//Graph is realized using an adjacency matrix (adjmx[m][n])
		//Position in the matrix is read as follows:
		//m, n are vertices, the entry at this position determines if they
		//are connected (1 ... connected; 0 ... not connected)
		//m is the "current vertex", n the "other one"

		int **adjmx = (int **)malloc(msize*sizeof(int*));
		for(int i = 0; i<msize; i++){
			adjmx[i] = (int *)malloc(msize*sizeof(int));
		}

		//initialize matrix by setting every entry to 0
		for(int i = 0; i<msize; i++){
			for(int j = 0; j<msize; j++){
				adjmx[j][i] = 0;
			}
		}
		
		//fill out adjacency matrix
		for(int i = 0; i<argc-1; i++){
			int m = edgelist[i][0];
			int n = edgelist[i][1];

			if(m == n){
				//Vertex connected to itself
				continue;
			}

			if(adjmx[m][n] == 1){
				//Edge already exists
				continue;
			}

			adjmx[m][n] = 1;
			adjmx[n][m] = 1;
		}
		
		//Get random 3-coloring by giving each vertex a number from 0 to 2
		int *clist = malloc(msize*sizeof(int));

		//Generate random coloring
		randColour(msize, clist);
		
		//Delete edges
		int num_edges = deleteEdges(msize, clist, adjmx);
		if(num_edges > BUF_LEN){
			free(edgelist);
			free(clist);
			free(adjmx);
			continue;
		}

		//Initialize solution with -1s
		struct edge sol;
		for(int i = 0; i<BUF_LEN; i++){
			sol.n[i] = -1;
			sol.m[i] = -1;
		}

		//If the graph is 3-colourable, instantly write sol to the circular buffer
		if(num_edges == 0){
			sem_wait(mutex);
			if(myshm->term == 1){
				sem_post(mutex);
				cleanup();
				free(edgelist);
				free(clist);
				free(adjmx);
				exit(EXIT_SUCCESS);
			}
			circ_buf_write(sol);
			sem_post(mutex);
			free(edgelist);
			free(clist);
			free(adjmx);
			continue;
			
		}

		//Write solution to buffer
		sem_wait(mutex);
		int count = 0;
		for(int i = 0; i<msize; i++){
			for(int j = 0; j<msize; j++){
				if(adjmx[i][j] == -1){
					sol.n[count] = j;
					sol.m[count] = i;
					count++;
				}
			}
		}
		//Terminate if signal was caught by supervisor
		if(myshm->term == 1){
			sem_post(mutex);
			cleanup();
			free(edgelist);
			free(clist);
			free(adjmx);
			exit(EXIT_SUCCESS);
		}
		circ_buf_write(sol);
		sem_post(mutex);
		free(edgelist);
		free(clist);
		free(adjmx);
	}	

	fprintf(stderr, "%s: Should not be reached\n", pgm_name);
	cleanup();
	return EXIT_FAILURE;
}

/**
 * SHM-Setup funciton
 * @brief This function opens the shared memory and semaphores
 */
static void setup_shm(void){
	//Open shm
	int shmfd = shm_open(SHM_NAME, O_RDWR , 0600);
	if(shmfd < 0){
		fprintf(stderr, "%s: Error accessing shared memory\n", pgm_name);
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

	//Close file descriptor
	if(close(shmfd) < 0){
		fprintf(stderr, "%s: Error closing file descriptor\n", pgm_name);	
		munmap(myshm, sizeof(*myshm));
		shm_unlink(SHM_NAME);
		exit(EXIT_FAILURE);
	}

	//Open semaphores
	used_sem = sem_open(SEM_US, 0);
	free_sem = sem_open(SEM_FS, 0);
	mutex = sem_open(SEM_EX, 0);
	if(used_sem == SEM_FAILED || free_sem == SEM_FAILED || mutex == SEM_FAILED){
		fprintf(stderr, "%s: Failed to open semaphores\n", pgm_name);
		cleanup();
		exit(EXIT_FAILURE);
	}
}


/**
 * Parsing function
 * @brief This function parses an edge string into 2 ints and writes them to edgelist.
 * @details
 * @param pos Index in edgelist.
 * @param edge The edge string.
 * @param edgelist The edges are stored in edgelist.
 * @return Returns -1 if edge couldn't be parsed correctly, otherwise 1
 */
static int parseEdge(int pos, char *edge, int **edgelist){

	//check if edge string only contains numbers and a dash inbetween
	//and get location of the dash
	int dashloc = -1;
	for(int i = 0; i<strlen(edge); i++){
		if(edge[i] < '0' || edge[i] > '9'){
			if(edge[i] == '-'){
				
				if(i == 0 || i == strlen(edge)-1 || dashloc != -1){
					fprintf(stderr, "%s: Wrong edge format\n", pgm_name);
					return -1;
				}
				dashloc = i;
				if(dashloc>MAX_VERTICES || strlen(edge)-dashloc-1>MAX_VERTICES){
					fprintf(stderr, "%s: Graph is too large (max. 1000 vertices)\n", pgm_name);
					return -1;
				}
			}
			else{
				fprintf(stderr, "%s: Wrong edge format\n", pgm_name);
				return -1;
			}
		}
	}

	if(dashloc == -1){
		fprintf(stderr, "%s: Wrong edge format\n", pgm_name);
		return -1;
	}

	char *m_str = malloc(strlen(edge)*sizeof(char));
	char *n_str = malloc(strlen(edge)*sizeof(char));
	strncpy(m_str, edge, dashloc);
	strcpy(n_str, edge+dashloc+1);
	int m = strtol(m_str, NULL, 10);
	int n = strtol(n_str, NULL, 10);
	
	edgelist[pos][0] = m;
	edgelist[pos][1] = n;
	return 1;
}

/**
 * Colouring function
 * @brief This function "colours" each vertex randomly.
 * @details
 * @param msize Number of vertices.
 * @param clist Array which stores a colour for each vertex.
 */
static void randColour(int msize, int *clist){
	//3 possible colours: 0, 1, 2
	for(int i = 0; i<msize; i++){
		clist[i] = rand() % 3;
	}
}

/**
 * Function to delete edges
 * @brief This function "deletes" edges between vertices of the same colour.
 * @details
 * @param msize Number of vertices.
 * @param clist Array which stores a colour for each vertex.
 * @param matrix Adjacency matrix.
 */
static int deleteEdges(int msize, int *clist, int **matrix){
	int delEdges = 0;
	for(int i = 0; i<msize; i++){
		for(int j = 0; j<msize; j++){
			if((clist[i] == clist[j]) && (matrix[j][i] == 1)){
				if(delEdges >= BUF_LEN){
					return BUF_LEN+1;
				}
				//-1/-2 --> only one edge needs to be included in the solution
				matrix[j][i] = -1;
				matrix[i][j] = -2;
				delEdges++;
			}
		}
	}
	return delEdges;
}

/**
 * Writing function
 * @brief This function writes an edge to the circular buffer.
 * @details
 * @param sol struct that contains 2 vertices and represents an edge.
 */
static void circ_buf_write(struct edge sol){
	sem_wait(free_sem);
	myshm->data[myshm->write_pos] = sol;
	sem_post(used_sem);
	myshm->write_pos = (myshm->write_pos + 1) % MAX_SIZE;
}

/**
 * Cleanup function
 * @brief This function cleans up resources.
 */
static void cleanup(void){
	munmap(myshm, sizeof(*myshm));
	sem_close(used_sem);
	sem_close(free_sem);
	sem_close(mutex);
}








