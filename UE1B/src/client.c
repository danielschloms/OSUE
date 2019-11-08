/**
 * @file client.c
 * @author Daniel Elias Schloms <e11701253@student.tuwien.ac.at>
 * @date 06.11.18
 *
 * @brief HTTP Client
 * 
 * This program request a file via http and writes the
 * file to stdout or a specified file.
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
#include "clientheader.h"

static char *pgm_name; 			/*!< Program name */

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	fprintf(stderr, "USAGE: %s [-p PORT] [ -o FILE | -d DIR ] URL\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief After checking options and arguments this function connects to a host and requests a file
 * @details The function first checks for options and arguments. If -p is used, a port can be specified
 * (standard = 80). -o is used to write the content to a specific file and -d is used to write the content
 * into a file in a specific directory. After parsing all arguments getaddrinfo() is called, a socket is 
 * created and a connection is established. This client then reads the requested file and prints
 * its content to stdout or a specific file.
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
	pgm_name = argv[0];

	if (argc < 2){
		usage();	
	}

	int opt_p = 0;		/* opt_p = 1 if -p is used 				*/
	int opt_o = 0;		/* opt_o = 1 if -o is used 				*/
	int opt_d = 0;		/* opt_d = 1 if -d is used 				*/
	int c;			/* Used to check options 				*/
	char *portnr = "80";	/* Port Number, default is 80				*/
	FILE *fp;		/* fp is the file to which the content will be written 	*/
	char *outfile = NULL;	/* -o: name of the file 				*/
	char *dirname = NULL;	/* -d: name of the directory 				*/

	/*
	 * Checks options
	 */	
	while((c=getopt(argc, argv, "p:o:d:"))!= -1){
		switch(c){
			case 'p': opt_p++;
				portnr = optarg;
				break;
			case 'o': opt_o++;
				outfile = optarg;
				break;
			case 'd': opt_d++;
				dirname = optarg;
				break;
			case '?':
				usage(); 
				break;
		}
		
	}
	
	
	if(opt_p > 1 || opt_o+opt_d > 1){
		usage();
	}
	if(argc < opt_p + opt_o + opt_d + 2){
		usage();
	}

	char *url = argv[argc-1];	/* url takes a string from the command line 		*/
	
	char httpcheck[7];		/* httpcheck equals the first 7 chars from the url 	*/	
	strncpy(httpcheck, url, 7);
	httpcheck[7] = '\0';
	
	/*
	 * Checks if first 7 chars of URL equal "http://"
	 */
	if(strcmp(httpcheck, "http://") != 0){
		fprintf(stderr, "%s: URLs must start with http://\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	char *reqfile = url+7;		/* reqfile is the url without "http://" 		*/
	int cutoff = strlen(reqfile);	/* cutoff is the point where the hostname ends 	*/

	cutoff = findpath(reqfile);			/* cutoff is the point in the url where the path begins		*/
	char hostname[cutoff];				/* hostname is reqfile without the additional path		*/
	strncpy(hostname, reqfile, cutoff);
	hostname[cutoff] = '\0';

	char *filepath = reqfile + cutoff;	      	/* filepath is the path to the requested file (w/o hostname)	*/
	char *filename = strrchr(filepath, '/')+1;    	/* filename is the name of the requested file 		    	*/

	/*
	 * If no file is specified, index.html is chosen
	 */
	if(strlen(reqfile) == cutoff){
		filename = "index.html";
	}
	
	
	
	int sockfd = getclientsocket(hostname, portnr, argv[0]);
	
	printf("Connected\n\n");

	FILE *sockfile = fdopen(sockfd, "r+b");			/* Content will be read from sockfile */
	char req[strlen(hostname)+strlen(filepath)+64];		/* Request string			*/
	sprintf(req, "GET /%s HTTP/1.1\r\nHOST: %s\r\nConnection: close\r\n\r\n", filepath, hostname);

	/*
	 * If options -o or -d are used, open specified files
	 */
	if(opt_o == 1){
		fp = fopen(outfile, "wb");
	}
	else if(opt_d == 1){
		DIR* dir = opendir(dirname);
		if(dir){
			closedir(dir);
			char pathfile[FILENAME_MAX];
			sprintf(pathfile, "%s/%s", dirname, filename);
			fp = fopen(pathfile, "wb");
		}
		else if(ENOENT == errno){
			close(sockfd);
			fprintf(stderr, "%s: Failed to open directory\n", argv[0]);
			exit(EXIT_FAILURE);
		}
		else{
			close(sockfd);
			fprintf(stderr, "%s: Failed to open directory\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	fputs(req, sockfile);
	fflush(sockfile); 

	/*
	 * If checkresponse<0, the response was either invalid or negative
	 */
	if(checkresponse(sockfile, argv[0])<0){
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	char buf[1];	/* Buffer for fread() und fwrite()		*/

	/*
	 * freads to buf and then writes to specified file or stdout
	 */						
	while(fread(buf, sizeof(buf), 1, sockfile) == 1){
		if(opt_o == 1 || opt_d == 1){
			fwrite(buf, sizeof(buf), 1, fp);
		}
		else{
			fwrite(buf, sizeof(buf), 1, stdout);
		}
	}
	close(sockfd);
	return EXIT_SUCCESS;
}
