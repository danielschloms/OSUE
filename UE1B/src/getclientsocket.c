/**
 * Function getclientsocket()
 * @brief Creates a socket and connects to the host.
 * @details This function creates a socket, connects to a
 * host and checks return values after each action.
 * @param hostname Host.
 * @param portnr Port number.
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

int getclientsocket(char* hostname, char* portnr, char* pgm_name){

	/*
	 * Create addrinfo *ai and fill out addrinfo hints
	 */
	struct addrinfo *ai;						/* Return value from getaddrinfo() 	*/
	struct addrinfo hints;						/* Hints for getaddrinfo() 		*/
	struct addrinfo *nxt;
	int checkai;							/* Used to check getaddrinfo() 	*/

	hints.ai_flags = 0;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	/*
	 * Check return value of getaddrinfo()
	 */
	checkai = getaddrinfo(hostname, portnr , &hints, &ai);
	if(checkai != 0){
		fprintf(stderr, "%s: getaddrinfo: %s\n", pgm_name, gai_strerror(checkai));
		exit(EXIT_FAILURE);
	}
	int sockfd;
	for(nxt = ai; nxt != NULL; nxt=nxt->ai_next){
		sockfd = socket(nxt->ai_family, nxt->ai_socktype, nxt->ai_protocol); 	/* sockfd ... used socket*/	
			/*
			 * Check if socket was created
			 */
		if(sockfd < 0){
			fprintf(stderr, "%s: Could not create socket\n", pgm_name);
			continue;
		}
		break;
	}
	freeaddrinfo(ai);
	/*
	 * Check if getaddrinfor() returned a valid struct addrinfo
	 */
	if(nxt == NULL){
		fprintf(stderr, "%s: Could not resolve host %s\n", pgm_name, hostname);
		exit(EXIT_FAILURE);
	}

	/*
	 * Create the connection and check it's return value
	 */
	if(connect(sockfd, nxt->ai_addr, nxt->ai_addrlen) < 0){
		close(sockfd);
		fprintf(stderr, "%s: Connection failed\n", pgm_name);
		exit(EXIT_FAILURE);
	}
	return sockfd;
}
