/**
 * Function findpath()
 * @brief This function finds the point in the url where the hostname ends
 * and the path begins
 * @details This function checks the url (without the http:// part) and
 * checks every character. If it's one of the defined characters, the hostname
 * ends and the path begins.
 * @param char reqfile The file requested by the client.
 * @return Returns the cutoff point between hostname and path
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int findpath(char *reqfile){

	int cutoff = strlen(reqfile);			/* Point between hostname and path	*/

	/*
	 * Looks for the first use of one of the
	 * specified seperating symbols
	 */
	for(int i = 0; i<strlen(reqfile); i++){
		int found = 0;			
		char symbols[] = {';','/','?',':','@','=','&'};
		for(int j = 0; j<=6; j++){
			if(reqfile[i] == symbols[j]){
				cutoff = i;
				found = 1;
				break;
				}
			}
			
		if(found == 1){
			break;
		}
	}
	return cutoff;
}
