/**
 * Function getserversocket()
 * @brief Creates a socket and sets it as passive
 * @details This function creates a socket, binds to a port
 * and waits for a connection to a client.
 * @param hostname Host.
 * @param portnr Port number.
 * @return Returns a passive socket on success
 * otherwise -1.
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#define LISTEN_BACKLOG 5

/**
 * Signal handler function.
 * @brief If SIGINT or SIGTERM is received, the program terminatse
 * @details global variables: term
 */
static void sig_handler(int signo){
	if(signo == SIGINT || signo == SIGTERM){
		printf("Terminating\n");
		exit(EXIT_SUCCESS);
	}
}

int getserversocket(char *portnr){
	if(signal(SIGINT, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGTERM\n");
	}
	//struct sockaddr_in saddr;			
	//saddr.sin_family = AF_INET;
	//saddr.sin_addr.s_addr = INADDR_ANY;
	
	struct addrinfo hints, *ai, *nxt;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	//hints.ai_addr = (struct sockaddr*)&saddr;

	int checkai = getaddrinfo(NULL, portnr, &hints, &ai);		/* used to check return value of getaddrinfo()	*/


	/*
	 * Check return value of getaddrinfo()
	 */
	if(checkai!=0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(checkai));
		return -1;
	}

	int sockfd;	/* Socket used by the server	*/

	for(nxt = ai; nxt != NULL; nxt=nxt->ai_next){

		/*
		 * Create socket and check return value
	 	 */
		if((sockfd = socket(nxt->ai_family, nxt->ai_socktype, nxt->ai_protocol))<0){
			fprintf(stderr, "Socket could not be created\n");
			continue;
		}

		int yes=1;

		/*
		 * Port can be used again if blocked
	 	 */
		if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
   		 	fprintf(stderr, "Could not set socket options\n");
			close(sockfd);
    			continue;
		}  

		/*
		 * Bind socket and check return value
	 	 */		
		if(bind(sockfd, nxt->ai_addr, nxt->ai_addrlen)<0){
			continue;
		}
		break;
	}
	
	freeaddrinfo(ai);
	
	/*
	 * Check if addrinfo was found
	 */
	if(nxt == NULL){
		fprintf(stderr, "Failed to bind\n");
		return -1;
	}
	
	/*
	 * Set socket as passive and check return value
	 */
	if(listen(sockfd, LISTEN_BACKLOG) < 0){
		fprintf(stderr, "Socket could not be marked as passive");
		close(sockfd);
		return -1;
	}
	printf("Listening...\n");
	return sockfd;
}
