/**
 * Function compareline()
 * @brief This function compares 2 lines passed on by the main function.
 * @details This function takes 2 lines, compares them char by char and counts the
 * amount of mismatches. If mode_i is > 0 (option -i is used) then each
 * letter is changed to it's lowercase variant.
 * @param line1 Line of first file.
 * @param line2 Line of second file.
 * @param lineCount How many lines have been read.
 * @param mode_i Indicates if option -i was used.
 * @param *output The string where the output is stored.
 * @param buffer Size of output.
 * @return Returns nothing (void)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "util.h"

void compareLine(char line1[], char line2[], int lineCount, int mode_i, char *output, int buffer){
	
	int lineLength;		/* Length of the longer line	*/

	/*
	 * Compares the length of both lines
	 */
	if(strlen(line1)<=strlen(line2)){
		lineLength = strlen(line1);
	}	
	else{
		lineLength = strlen(line2);
	}

	int errorCount = 0;

	/*
	 * Compares the ith character of every line
	 */
	for(int i=0; i<lineLength-1; i++){
		if(line1[i] != line2[i] && mode_i == 0){
			errorCount++;
		}
		else if(tolower(line1[i]) != tolower(line2[i]) && mode_i > 0){
			errorCount++;
		}
		
	}	

	/*
	 * If there are no differences, nothing is printed
	 */
	if(errorCount != 0){
		char outLine[buffer];
		sprintf(outLine, "Line: %i, Characters: %i\n", lineCount, errorCount);
		strncpy(output, outLine, buffer);
		
	}
	else{
		*output = '\0';
	}
	output[buffer-1] = '\0';
}
