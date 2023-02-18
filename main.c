/*
 * main.c
 *
 *  Created on: Mar 17 2017
 *      Author: david
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "dsh.h"

//include builtins.h
#include "builtins.h"

#define CAP 16

int main(int argc, char **argv){
	//allocate space for user input
	char* input = (char*) malloc(MAXBUF);

	//initialize input
	strcpy(input, "");

	//get user input until 'exit' typed
	while(strcmp(input, "exit") != 0){
		//prompt user
		printf("dsh> ");

		//get user input with fgets
		fgets(input, MAXBUF, stdin); 

		//if user enters only spaces, reprompt
		int spaces = 0;
		for(int i = 0; i < strlen(input); i++){
			if(input[i] == ' '){
				spaces++; 
			}
		}
		if (strlen(input) == spaces){
			//prompt user
			printf("dsh> ");
			//get input with fgets
			fgets(input, MAXBUF, stdin); 
		}

		//deal with '\n' as user input
		for(int i = 0; i < strlen(input); i++){
			//reprompt if user types only '\n'
			if(input[0] == '\n' && strlen(input) == 1){
				//prompt user
				printf("dsh> ");
				//store input with fgets
				fgets(input, MAXBUF, stdin); 
			}
			//replace any other '\n' with '\0'
			else if(input[i] == '\n'){
				input[i] = '\0';
			}
		}

		//remove all spaces from input
		char** trimInput = split(input, " ");

		//handle built in commands

		//pwd -- print out current working dir
		char cwd[MAXBUF];
		if(strcmp(trimInput[0], "pwd") == 0){
			getcwd(cwd, sizeof(cwd));
			printf("%s\n", cwd);
		}
		//cd -- changes current working dir
		if(strcmp(trimInput[0], "cd") == 0){
			if(trimInput[1] != NULL){
				//printf("path given: %s\n", trimInput[1]);
				chdir(trimInput[1]);
			}
			else{
				//printf("path not given\n");
				char *home = getenv("HOME");
				chdir(home);
			}
		}
		// //exit -- exits dsh
		// else if(built == 4){
		// 	//clear current input
		// 	strcpy(input, "exit");
		// }
		
		
		//if user types full path 
		if(*trimInput[0] == '/'){
			mode1(trimInput);
		}
		//is user does not give full path
		else if(strcmp(trimInput[0], "pwd") != 0 && strcmp(trimInput[0], "cd") != 0 && strcmp(trimInput[0], "exit") != 0){
			mode2(trimInput);
		}

		//free heap
		int i = 0;
		for(i = 0; trimInput[i] != NULL; i++){
			//printf("i: %d \n", i);
   			free(trimInput[i]);
			trimInput[i] = NULL;
 		}

		free(trimInput);
		trimInput = NULL;

	}

	free(input);
	input = NULL;

	return 0;
}