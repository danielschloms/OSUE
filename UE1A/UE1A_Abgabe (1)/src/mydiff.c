/**
 * @file mydiff.c
 * @author Daniel Elias Schloms <e11701253@student.tuwien.ac.at>
 * @date 23.10.2018
 *
 * @brief Main program module.
 * 
 * This program compares 2 textfiles and prints the number of
 * differences per line to stdout or a textfile.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "util.h"

static char *pgm_name;			/*!< Program name */

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	(void) fprintf(stderr, "USAGE: %s [-i] [-o file] file file\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief After checking options and arguments this function opens 2 textfiles and passes
 * corresponding lines to compareLine(). The differences are then printed to either stdout or
 * a file specified as an option argument.
 * like here.
 * @details The function first checks for options and arguments. If -i is used, the program will
 * differentiate between upper- and lowercase. If -o optarg is used, the results will be printed
 * to a new file named after the argument. To compare the files both are opened and the lines
 * are passed to compareLine(), which prints the differences to output[].
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){	
	pgm_name = argv[0];

	if(argc < 3){
		usage();
	}

	char *outFile = NULL;				/* Content is written to outfile (-o) */
	int c;						/* Used to check options */
	int opt_i = 0;					/* opt_i > 0 if -i is used*/
	int opt_o = 0;

	while((c=getopt(argc, argv, "io:"))!= -1){
		switch(c){
			case 'i': opt_i++;
				break;
			case 'o': opt_o++;
				outFile = optarg;
				if(optind == argc-1){
					usage();
				}
				break;
			case '?':
				usage(); 
				break;
		}
		
	}

	FILE *fp1;			/* First file for mydiff		*/
	FILE *fp2;			/* Second file for mydiff		*/
	FILE *outp;			/* Output file if -o is used		*/

	char *str1 = NULL;		/* A line of the first file		*/
	char *str2 = NULL;		/* A line of the second file		*/
	size_t len1 = 0;		/* Length of line of the first file	*/
	size_t len2 = 0;		/* Length of line of the second file	*/
	ssize_t toread1;		/* Used to show if line was last line	*/
	ssize_t toread2;		/* Used to show if line was last line	*/
	char output[64];		/* Output message			*/

	fp1 = fopen(argv[argc-2], "r");
	fp2 = fopen(argv[argc-1], "r");

	/*
	 * Opens file for output (-o)
	 */
	if(outFile != NULL){
		outp = fopen(outFile, "w");
		if(outp == NULL){
			(void)fprintf(stderr, "%s: Could not open output file\n", argv[0]);
			exit(EXIT_FAILURE);
		}
	}

	/*
	 * Check if files were opened successfully
	 */
	if(fp1 == NULL || fp2 == NULL){
		fprintf(stderr, "%s: Could not open files\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int lineCount = 0;		/* Current line	*/
	
	/*
	 * each line of both are read and given to compareLine(), the results are written to output
	 */
	while((toread1 = getline(&str1, &len1, fp1)) != -1 && (toread2 = getline(&str2, &len2, fp2)) != -1){

		lineCount++;
		compareLine(str1, str2, lineCount, opt_i, output, sizeof(output));
		if (opt_o == 0){
			printf("%s", output);
			
		}
		else{
			fprintf(outp, "%s", output);
		}
 	
	}

	free(str1);
	free(str2);
   	fclose(fp1);
	fclose(fp2);
	if(opt_o != 0){
		fclose(outp);
	}

	return(EXIT_SUCCESS);
}
