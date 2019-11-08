/**
 * Function checkrequest()
 * @brief This function checks if the request is valid.
 * @details This function checks if the request starts
 * with the method "GET" and ends with "HTTP/1.1".
 * @param method String which should be "GET".
 * @param http String which should be "HTTP/1.1".
 * @param clfd File descriptor of connected socket.
 * @return Returns 0 if successful, otherwise -1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

int checkrequest(char *method, char *http, int clfd){
	
	char *errmsg = malloc(sizeof(char)*64);		/* errmsg is sent when method ist not GET or when the request is invalid*/

	/*
	 * Check if method is GET
	 */
	if(strcmp(method, "GET")!=0){
		fprintf(stderr, "Invalid request (method)\n");
		errmsg = "HTTP/1.1 501 Not implemented\r\nConnection: close\r\n\r\n";
		send(clfd, errmsg, strlen(errmsg), 0);
		close(clfd);
		free(errmsg);
		return -1;
	}
		
	/*
	 * Check if request ends with "HTTP/1.1"
	 */
	if(strncmp(http, "HTTP/1.1", 8)!=0){
		printf("http: %s\n", http);
		fprintf(stderr, "Invalid request (no HTTP/1.1)\n");
		errmsg = "HTTP/1.1 400 Bad request\r\nConnection: close\r\n\r\n";
		free(errmsg);
		send(clfd, errmsg, strlen(errmsg), 0);
		close(clfd);
		return -1;
	}
	free(errmsg);
	return 0;
}
