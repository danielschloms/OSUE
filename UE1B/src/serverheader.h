/**
 * @file serverheader.h
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 08.11.18
 *  
 * @brief Provides utility functions for the server.
 *
 * 
 */
 
#ifndef SERVERHEADER_H__
#define SERVERHEADER_H__
#define LISTEN_BACKLOG 5
/**
 * @brief This function returns a passive server socket.
 * @details This function calls getaddrinfo() and uses	
 * the information provided to create a socket, after which
 * it's bound to a port and set as passive.
 */
int getserversocket(char* portnr);

/**
 * @brief This function checks if the request is valid.
 * @details This function takes the request and checks if
 * method = "GET" and if it ends with "HTTP/1.1".
 */

int checkrequest(char *method, char *http, int clfd);

#endif
