/**
 * @file intmul.c
 * @author Daniel Schloms <e11701253@student.tuwien.ac.at>
 * @date 14.12.18
 *
 * @brief Main program module.
 * 
 * This program multiplies 2 hexadecimal numbers.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#define READ 0
#define WRITE 1

static char *pgm_name; 						/*!< Program name 	*/
static int createchild(char *A_str, char *B_str, char *res_ret);	/*!< fork and exec	*/
static void add(char *a, char *b, char *c, char *d, char *add_res, int n);

/**
 * Usage function.
 * @brief This function writes helpful usage information about the program to stderr.
 * @details global variables: pgm_name
 */
static void usage(void) {
	fprintf(stderr, "USAGE: %s\n", pgm_name);
	exit(EXIT_FAILURE);
}

/**
 * Program entry point.
 * @brief This function reads 2 hexadecimal numbers from stdin and splits them in half for further calculation.
 * @details This function reads 2 hexadecimal numbers from stdin, splits them in half (if length > 1)
 * and gives them to createchild() as parameters. The return values are used to calculate a result with add(), which 
 * is printed to stdout.
 * global variables: pgm_name
 * @param argc The argument counter.
 * @param argv The argument vector.
 * @return Returns EXIT_SUCCESS.
 */
int main(int argc, char *argv[]){
	pgm_name = argv[0];

	if(argc>1){
		usage();
	}

	char *A_str = NULL;
	char *B_str = NULL;
	size_t lenA = 0;
	size_t lenB = 0;

	getline(&A_str, &lenA, stdin);
	getline(&B_str, &lenB, stdin);	

	if(strlen(A_str) != strlen(B_str)){
		fprintf(stderr, "%s: Numbers must be of equal length\n", pgm_name);
		free(A_str);
		free(B_str);
		exit(EXIT_FAILURE);
	}	
	
	if(strlen(A_str) < 3){	
		int valA = 0;
		int valB = 0;

		//Check if A and B are valid numbers
		if (A_str[0] >= '0' && A_str[0] <= '9'){
			valA = 1;
		}
   		else if (A_str[0] >= 'A' && A_str[0] <= 'F'){
      			valA = 1;
		}
   		else if (A_str[0] >= 'a' && A_str[0] <= 'f'){
     			valA = 1;	
		}

		if (B_str[0] >= '0' && B_str[0] <= '9'){
			valB = 1;
		}
   		else if (B_str[0] >= 'A' && B_str[0] <= 'F'){
      			valB = 1;
		}
   		else if (B_str[0] >= 'a' && B_str[0] <= 'f'){
     			valB = 1;	
		}

		if(valA != 1 ||	valB != 1){
			fprintf(stderr, "%s: Input contains non number\n", pgm_name);
			free(A_str);
			free(B_str);
			exit(EXIT_FAILURE);
		}

		int A = strtol(A_str, NULL, 16);
		int B = strtol(B_str, NULL, 16);
		free(A_str);
		free(B_str);
		int res = A*B;
		fprintf(stdout, "%X", res);		
		exit(EXIT_SUCCESS);
	}

	if(((strlen(A_str)+1)%2) != 0){
		fprintf(stderr, "%s: Length of numbers must be even\n", pgm_name);
		free(A_str);
		free(B_str);
		exit(EXIT_FAILURE);
	}

	int len = strlen(A_str)-1;
	int newlen = len/2;

	char *AH = calloc(newlen+1, sizeof(char));
	char *AL = calloc(newlen+1, sizeof(char));
	char *BH = calloc(newlen+1, sizeof(char));
	char *BL = calloc(newlen+1, sizeof(char));

	//Split A and B in half
	strncpy(AH, A_str, newlen);
	strncpy(AL, A_str+newlen, newlen);
	strncpy(BH, B_str, newlen);
	strncpy(BL, B_str+newlen, newlen);	

	char *first = calloc(2*newlen+1, sizeof(char));
	char *second = calloc(2*newlen+1, sizeof(char));
	char *third = calloc(2*newlen+1, sizeof(char));
	char *fourth = calloc(2*newlen+1, sizeof(char));

	//fork and execute 4 times (for each partial result)
	//if function returns -1, clean up resources and exit
	
	int c1 = createchild(AH, BH, first);
	int c2 = createchild(AH, BL, second);
	int c3 = createchild(AL, BH, third);
	int c4 = createchild(AL, BL, fourth);

	if((c1 < 0) || (c2 < 0) || (c3 < 0) || (c4 < 0)){
		free(A_str);
		free(B_str);
		free(AH);
		free(AL);
		free(BH);
		free(BL);
		free(first);
		free(second);
		free(third);
		free(fourth);	
		exit(EXIT_FAILURE);
	}


	char *result = calloc(2*len+1, sizeof(char));
	add(first, second, third, fourth, result, len);

	fprintf(stdout, "%s", result);
	free(A_str);
	free(B_str);
	free(AH);
	free(AL);
	free(BH);
	free(BL);
	free(first);
	free(second);
	free(third);
	free(fourth);
	free(result);
	return EXIT_SUCCESS;
}

/**
 * Forking and executing function.
 * @brief This function forks and executes with no arguments, but the parent writes
 * 2 new strings to the childs stdin
 * global variables: pgm_name
 * @param A_str First hexadecimal number.
 * @param B_str Second hexadecimal number.
 * @param res_ret Parent reads result from pipe and stores it in res_ret.
 */
static int createchild(char *A_str, char *B_str, char* res_ret) {

	FILE *rfile;
	FILE *wfile;
	int wpipe[2]; 	//Parent writes to wpipe
	int rpipe[2];	//Parent reads from rpipe

	if(pipe(wpipe) == -1 || pipe(rpipe) == -1){
		fprintf(stderr, "Failed to create unnamed pipe(s)\n");
		return -1;
	}

	pid_t cpid = fork();
	switch(cpid){
		case -1:
			fprintf(stderr, "%s: Cannot fork\n", pgm_name);
			return -1;
		case 0:
			//CHILD PROCESS

			//Close unused pipe ends
			close(rpipe[READ]);
			close(wpipe[WRITE]);	

			//redirect stdin, stdout
			dup2(wpipe[READ], STDIN_FILENO);
			dup2(rpipe[WRITE], STDOUT_FILENO);
			close(wpipe[READ]);
			close(rpipe[WRITE]);

			//execute
			execlp("./intmul", "./intmul", NULL);
			fprintf(stderr, "%s: Execute failed\n", pgm_name);
			return -1;
			break;	
		default:
			//PARENT PROCESS

			//Close unused pipe ends
			close(rpipe[WRITE]);
			close(wpipe[READ]);

			//Give child new strings
			wfile = fdopen(wpipe[WRITE], "w");
			fprintf(wfile, "%s\n%s\n", A_str, B_str);
			fclose(wfile);
			close(wpipe[WRITE]);

			//Wait for termination of child
			pid_t pid;
			int status;
			pid = wait(&status);
			if(WEXITSTATUS(status) != EXIT_SUCCESS){
				fprintf(stderr,"%s: Error ... pid: %d status: %d.\n", pgm_name, pid, WEXITSTATUS(status));
				return -1;
			}		

			//Read result from pipe
			char *res = NULL;
			rfile = fdopen(rpipe[0], "r");
			size_t len = 0;
			getline(&res, &len, rfile);
			close(rpipe[READ]);
	
			//Copy result to res_ret
			strcpy(res_ret, res);
			free(res);
			return 1;
			break;
	}
	return -1;
}

/**
 * Adding function.
 * @brief This function adds 4 hexadecimal (as strings) numbers together and writes the
 * result to add_res.
 * @details Addition formula: A*B = a^n + b^(n/2) + c^(n/2) +d
 * global variables: pgm_name
 * @param a First number.
 * @param b Second number.
 * @param c Third number.
 * @param d Fourth number.
 * @param add_res Result is written to add_res.
 * @param n n from the formula ... Length 2 strings to multiply (from main())
 */
static void add(char *a, char *b, char *c, char *d, char* add_res, int n){
	//a * 16^n
	//b * 16^(n/2)
	//c * 16^(n/2)
	//d * 1
	char *res_str = calloc(2*n+1, sizeof(char));

	//Initialize res_str with 0's
	for(int i = 0; i<2*n; i++){
		res_str[i] = '0';
	}	

	int a_len = strlen(a);
	int b_len = strlen(b);
	int c_len = strlen(c);
	int d_len = strlen(d);
	
	//Add 0s to a,b,c
	int carry = 0;			
	int res_digit;		//Next digit to be written (from tempres)
	int dig;		//Next digit from string
	int arrdig;		//Digit in res_str at current index
	char arrdig_chr[1];	//arrdig as char
	int tmpres;		//temp. Result for calculating carry and res_digit
	char tmphex[1];		//res_digit as char
	char tmpstr[2];		//for conversion int -> str -> char

	//d can be printed directly since res_str = 0
	for(int i = 0; i<d_len; i++){
		res_str[(2*n-1)-i] = d[d_len-1-i];
		d[d_len-1-i] = '\0';
	}

	//Calculate digits from res_str and c, shifted to the left
	for(int i = 0; i<c_len; i++){
		dig = strtol(c+c_len-1-i, NULL, 16);
		arrdig_chr[0] = res_str[(3*n/2)-1-i];

		if (arrdig_chr[0] >= '0' && arrdig_chr[0] <= '9'){
			arrdig = arrdig_chr[0] - '0';
		}
   		else if (arrdig_chr[0] >= 'A' && arrdig_chr[0] <= 'F'){
      			arrdig = arrdig_chr[0] - 'A' + 10;
		}
   		else if (arrdig_chr[0] >= 'a' && arrdig_chr[0] <= 'f'){
     			arrdig = arrdig_chr[0] - 'a' + 10;	
		}	

		tmpres = dig+arrdig+carry;
		carry = tmpres/(0x10);
		res_digit = tmpres%(0x10);

		sprintf(tmpstr, "%X", res_digit);
		strncpy(tmphex, tmpstr, 1);
		res_str[(3*n/2)-1-i] = tmphex[0];

		c[c_len-1-i] = '\0';
	}
	
	//Write carry to next index
	if(carry != 0){
		sprintf(tmpstr, "%X", carry);
		strncpy(tmphex, tmpstr, 1);
		res_str[(3*n/2)-1-c_len] = tmphex[0];
	}
	carry = 0;

	//Calculate digits from res_str and b, shifted to the left
	for(int i = 0; i<b_len; i++){
		dig = strtol(b+b_len-1-i, NULL, 16);
		arrdig_chr[0] = res_str[(3*n/2)-1-i];

		if (arrdig_chr[0] >= '0' && arrdig_chr[0] <= '9'){
			arrdig = arrdig_chr[0] - '0';
		}
   		else if (arrdig_chr[0] >= 'A' && arrdig_chr[0] <= 'F'){
      			arrdig = arrdig_chr[0] - 'A' + 10;
		}
   		else if (arrdig_chr[0] >= 'a' && arrdig_chr[0] <= 'f'){
     			arrdig = arrdig_chr[0] - 'a' + 10;	
		}	

		tmpres = dig+arrdig+carry;
		carry = tmpres/(0x10);
		res_digit = tmpres%(0x10);

		sprintf(tmpstr, "%X", res_digit);
		strncpy(tmphex, tmpstr, 1);
		res_str[(3*n/2)-1-i] = tmphex[0];

		b[b_len-1-i] = '\0';
	}

	//Calculate digits if b is shorter than c
	if((carry != 0) && (b_len < c_len)){
		for(int i = 0; i<c_len-b_len; i++){
			arrdig_chr[0] = res_str[(3*n/2)-1-b_len-i];

			if (arrdig_chr[0] >= '0' && arrdig_chr[0] <= '9'){
				arrdig = arrdig_chr[0] - '0';
			}
   			else if (arrdig_chr[0] >= 'A' && arrdig_chr[0] <= 'F'){
      				arrdig = arrdig_chr[0] - 'A' + 10;
			}
   			else if (arrdig_chr[0] >= 'a' && arrdig_chr[0] <= 'f'){
     				arrdig = arrdig_chr[0] - 'a' + 10;	
			}

			tmpres = arrdig+carry;
			carry = tmpres/(0x10);
			res_digit = tmpres%(0x10);

			sprintf(tmpstr, "%X", res_digit);
			strncpy(tmphex, tmpstr, 1);
			res_str[(3*n/2)-1-b_len-i] = tmphex[0];

			if(carry == 0){
				break;
			}
		}
	}
	else if(carry != 0){
		sprintf(tmpstr, "%X", carry);
		strncpy(tmphex, tmpstr, 1);
		res_str[(3*n/2)-1-b_len] = tmphex[0];
	}
	carry = 0;

	//Calculate digits from res_str and a, shifted to the left
	for(int i = 0; i<a_len; i++){
		dig = strtol(a+a_len-1-i, NULL, 16);
		arrdig_chr[0] = res_str[n-1-i];

		if (arrdig_chr[0] >= '0' && arrdig_chr[0] <= '9'){
			arrdig = arrdig_chr[0] - '0';
		}
   		else if (arrdig_chr[0] >= 'A' && arrdig_chr[0] <= 'F'){
      			arrdig = arrdig_chr[0] - 'A' + 10;
		}
   		else if (arrdig_chr[0] >= 'a' && arrdig_chr[0] <= 'f'){
     			arrdig = arrdig_chr[0] - 'a' + 10;	
		}	
	
		tmpres = dig+arrdig+carry;
		carry = tmpres/(0x10);
		res_digit = tmpres%(0x10);	
		
		sprintf(tmpstr, "%X", res_digit);
		strncpy(tmphex, tmpstr, 1);
		res_str[n-1-i] = tmphex[0];

		a[a_len-i-1] = '\0';
	}

	//Write carry to the next index
	if(carry != 0){
		sprintf(tmpstr, "%X", carry);
		strncpy(tmphex, tmpstr, 1);
		res_str[n/2-1] = tmphex[0];
	}

	strcpy(add_res, res_str);
	free(res_str);
}
