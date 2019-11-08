/**
 * @file server.c
 * @author Daniel Elias Schloms <e11701253@student.tuwien.ac.at>
 * @date 06.11.18
 *
 * @brief HTTP Server
 * 
 * This program waits for connections from clients and transmits requested files
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "serverheader.h"

static char *pgm_name; 			/* Program name		*/
static int term = 0;			/* Program terminates if 1	*/


/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name, term
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-p PORT] [-i INDEX] DOC_ROOT\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Signal handler function.
 * @brief This function sets term = 1 if a signal is received.
 * @details global variables: term
 */
static void sig_handler(int signo){
	if(signo == SIGINT || signo == SIGTERM){
		printf("SIG\n");
		term = 1;
	}
}

/**
 * Program entry point.
 * @brief After checking options and arguments this function creates a server socket, listens
 * for connections and transmits a requested file to a client.
 * @details The function first checks for options and arguments.If -p is used, a port can be
 * specified, the default is 8080. If -i is used, an index file can
 * be specified, the default is "index.html". Then it creates a socket and listens for connections.
 * After a connection is established, the request from the client is read and checked and the reqested
 * file is transmitted. Then the connection is closed and the server listens for connections again.
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
	if(signal(SIGINT, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGINT\n");
	}
	if(signal(SIGTERM, sig_handler)==SIG_ERR){
		printf("\nCan't catch SIGTERM\n");
	}
	pgm_name = argv[0];
	if(argc<2){
		usage();
	}
	
	char c;			/* Used for getopt()				*/
	int opt_p = 0;		/* opt_p = 1 if -p is used			*/
	int opt_i = 0;		/* opt_i = 1 if -i is used			*/
	char *portnr;		/* Port Number, default is 8080			*/
	char *index;		/* Index filename, default is "index.html	*/
	char *errmsg;		/* Error message				*/
	char *root_dir;		/* Name of the root directory			*/
	
	/*
	 * Check for options
	 */
	while((c=getopt(argc, argv, "p:i:"))!= -1){
		switch(c){
			case 'p': opt_p++;
				portnr = optarg;
				break;
			case 'i': opt_i++;
				index = optarg;
				break;
			case '?':
				usage(); 
				break;
		}
		
	}

	/*
	 * Set the root directory
	 */
	root_dir = argv[argc-1];
	DIR* dir = opendir(root_dir);

	/*
	 * Check if root directory exists
	 */	
	if(dir){
		closedir(dir);
	}
	else{
		fprintf(stderr, "%s: Failed to open directory\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	/*
	 * Set default port number (-p not used)
	 */
	if(opt_p == 0){
		portnr = "8080";
	}
	
	/*
	 * Set default index file (-i not used)
	 */
	if(opt_i == 0){
		index = "index.html";
	}	

	char *checkindex = malloc(sizeof(char)*255);
	sprintf(checkindex, "%s/%s", root_dir, index);
	/*
	 * Check if index file exists
	 */
	if(access(checkindex, F_OK) == -1) {
		fprintf(stderr, "%s: Index file doesn't exist\n", argv[0]);
		exit(EXIT_SUCCESS);
	} 	
	free(checkindex);
	
	/*
	 * If termination signal has occured, terminate program
	 */
	if(term == 1){
		printf("Terminating\n");
		exit(EXIT_SUCCESS);
	}
	
	int sockfd = getserversocket(portnr);
	if(sockfd<0){
		fprintf(stderr, "%s: Failed to create passive Socket\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	struct sockaddr_storage caddr;			/* Stores cliend address		*/
	socklen_t caddr_size = sizeof(caddr);
	while(1){
		if(term == 1){
			printf("Terminating\n");
			exit(EXIT_SUCCESS);
		}
		
		int clfd = accept(sockfd, (struct sockaddr*)&caddr, &caddr_size);	/* File descriptor of client		*/
		
		/*
		 * Check value of clfd
		 */	
		if(clfd<0){
			fprintf(stderr, "%s Failed to accept connection\n", argv[0]);
			close(sockfd);
			exit(EXIT_FAILURE);
		}
		
		FILE *recvfile;				/* Receives the request from the client*/
		recvfile = fdopen(clfd, "r+b");

		char *line;
		size_t len = 0;

		/*
		 * Get first line to check if request is valid
		 */
		getline(&line, &len, recvfile);
		char linecpy[strlen(line)];
		strcpy(linecpy, line);
		free(line);
		
		int sep1 = 0;			/* First space in line	*/
		int sep2 = 0;			/* Second space in line*/

		/**
		 * Get positions of spaces
		 */
		for(int i = 0; i<strlen(linecpy); i++){
			if(linecpy[i] == ' '){
				if(sep1==0){
					sep1 = i;
				}
				else{
					sep2 = i;
				}
			}
		}
		/**
		 * Check if first line contains 3 strings seperated by space
		 */
		if(sep1 == 0 || sep2 == 0){
			printf("Invalid request (invalid request format)\n");
			errmsg = "HTTP/1.1 501 Bad request\r\nConnection: close\r\n\r\n";
			send(clfd, errmsg, strlen(errmsg), 0);
			close(clfd);
			continue;
		}

		char method[sep1];				/* Method, should be GET 			*/
		char path[sep2-sep1];				/* Path to the requested file			*/
		char http[strlen(linecpy)-sep2];		/* To check if line ends with "HTTP/1.1"	*/

		strncpy(method, linecpy, (size_t)sep1);
		strncpy(path, linecpy+sep1+1, (size_t)sep2-sep1);
		strncpy(http, linecpy+sep2+1, (size_t)strlen(linecpy)-sep2);

		method[sep1] = '\0';
		path[sep2-sep1-1] = '\0';
		http[strlen(linecpy)-sep2] = '\0';

		if(checkrequest(method, http, clfd)<0){
			close(clfd);
			continue;
		}
		
		FILE *response;					/* Content will be sent to client 	*/	
		FILE *reqfile;					/* File requested by the client	*/
		char *filename = malloc(sizeof(char)*255);	/* Path and Filename as string		*/	
		response = fdopen(clfd, "wb");

		/*
		 * Check if last part of path is a directory
		 * or a file
		 */
		if(path[strlen(path)-1]=='/'){
			if(strlen(path)==1){
				sprintf(filename, "%s/%s", root_dir, index);
			}
			else{
				sprintf(filename, "%s%s%s", root_dir, path+1, index);
			}
		}
		else{
			/*
			 * Check if path starts with '/' and construct filename
			 * accordingly
			 */
			if(path[1] == '/'){
				sprintf(filename, "%s%s", root_dir, path+1);
			}
			else{
				sprintf(filename, "%s%s", root_dir, path);
			}
		}	
		
		printf("Requested file: %s\n", filename);
		
		/*
		 * Check if requested file is available
		 */
		if((reqfile = fopen(filename, "r+b")) == NULL){
			printf("Could not find requested file\n");
			errmsg = "HTTP/1.1 404 Not found\r\nConnection: close\r\n\r\n";
			send(clfd, errmsg, strlen(errmsg), 0);
			close(clfd);
			continue;
		}	
		
		char *datetime = malloc(sizeof(char)*128);		/* Date (RFC 822 according to Link in instructions)	*/
		time_t t;
		struct tm *tmp;
		
		t = time(NULL);
		tmp = localtime(&t);
		strftime(datetime, 100, "%a, %d %b %y %T %Z", tmp);

		fseek(reqfile, 0, SEEK_END);
		int filesize = ftell(reqfile); 
		fseek(reqfile, 0, SEEK_SET);

		char *header = malloc(sizeof(char)*256);
		sprintf(header, "HTTP/1.1 200 OK\r\nDate: %s\r\nContent-Length: %i\r\nConnection: close\r\n\r\n", datetime, filesize);

		/*
		 * Sending response header
		 */
		send(clfd, header, strlen(header), 0); 

		char buf[1];

		/*
		 * Writing content of requested file to response file
		 */
		/*while(fread(buf, sizeof(buf), 1, reqfile)){
			fwrite(buf, sizeof(buf), 1, response);
		}*/
		fprintf(recvfile, "TESTI\nTESTI");
		fflush(recvfile);
		printf("Transmission complete\n");
		close(clfd);

		/*
		 * Check if signal was received
		 */
		if(term == 1){
			printf("Terminating\n");
			exit(EXIT_SUCCESS);
		}
		printf("Listening...\n");
	}
	
	close(sockfd);
	return EXIT_SUCCESS;
}
