/**
 * @file shm.h
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 08.1.19
 *  
 * @brief Provides names, constants and structs used for the shared memory.
 */
 
#ifndef SHM_H__ 
#define SHM_H__

#define SHM_NAME "/11701253_3cshm" 	/**<Name of the shared memory file*/
#define MAX_SIZE (60)			/**<Size of the circular buffer*/
#define SEM_US "/11701253_sem_used"	/**<Semaphore filename*/
#define SEM_FS "/11701253_sem_free"	/**<Semaphore filename*/
#define SEM_EX "/11701253_sem_mutex"	/**<Semaphore filename*/
#define BUF_LEN (8)			/**<Maximal number of edges in a particular solution*/

/**
 * @brief edge stores BUF_LEN sets of vertices
 */
struct edge{
	unsigned int m[BUF_LEN];
	unsigned int n[BUF_LEN];
};

/**
 * @brief shm contains the array data for the circular buffer
 */
struct shm{
	unsigned int write_pos;
	unsigned int term;
	struct edge data[MAX_SIZE];
};

#endif
