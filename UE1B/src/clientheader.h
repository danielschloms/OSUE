/**
 * @file clientheader.h
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 08.11.18
 *  
 * @brief Provides utility functions for the client.
 *
 * 
 */
 
#ifndef CLIENTHEADER_H__ 
#define CLIENTHEADER_H__
 
/**
 * @brief This function finds the cutoff point between url and path.
 * @details This function checks every char of the url string and
 * marks the position of the start of the path.
 */
int findpath(char *reqpath);

/**
 * @brief This function returns a client socket.
 * @details This function calls getaddrinfo() and uses	
 * the information provided to create a socket.
 */
int getclientsocket(char* hostname, char* portnr, char* pgm_name);

/**
 * @brief This function checks if the response header is valid.
 * @details This function takes the first line of the response
 * header and checks if it starts with "HTTP/1.1" and what code
 * was received.
 */

#include <stdio.h>
int checkresponse(FILE *sockfile, char* pgm_name);

#endif
