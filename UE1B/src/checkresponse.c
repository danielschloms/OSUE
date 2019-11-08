/**
 * Function checkresponse()
 * @brief This function checks if the response header is valid and
 * Finds the beginning of the requested file.
 * @details This function checks the first line of the response header
 * and determines if it's a valid response. Then it looks for the first
 * empty line which marks the beginning of the requested file.
 * @param sockfile File in which request is written and from which response is read.
 * @return Returns 0 if successful, otherwise -1.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int checkresponse(FILE *sockfile, char* pgm_name){
	
	char *line;					/* Used to store lines of the response header		*/
	size_t len = 0;
	ssize_t toread;
	
	/*
	 * Get first line to check if response is valid
	 */
	getline(&line, &len, sockfile);
	char check[strlen(line)];			/* First 8 chars of the header are copied to check 	*/
	strncpy(check, line , 8);
	check[8] = '\0';
	
	/*
	 * Check if header starts with "HTTP/1.1"
	 */
	if(strcmp(check, "HTTP/1.1") != 0){
		free(line);
		fprintf(stderr, "%s: Protocol error!\n", pgm_name);
		return -1;
	}
	
	char *ptr;					/* Pointer used for strtol 			*/
	int httpcode = strtol(line+9, &ptr, 10); 	/* Status code is converted so int 		*/

	/*
	 * If httpcode == 0, the response is invalid
	 */
	if(httpcode == 0){
		free(line);
		fprintf(stderr, "%s: Protocol error!\n", pgm_name);
		return -1;
	}

	/*
	 * If the status code is not 200,  the status code wil be printed to stderr
	 */
	if(httpcode != 200){
		free(line);	
		fprintf(stderr, "%s\n", line+9);
		return -1;
	}

	/*
	 * Reads every line of the received file until an empty line is found
	 */
	while((toread = getline(&line, &len, sockfile)) != -1){
		if(strcmp(line, "\r\n") == 0){
			break;
		}
		
	}
	free(line);
	return 0;
}
