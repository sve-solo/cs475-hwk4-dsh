/*
 * dsh.c
 *
 *  Created on: Aug 2, 2013
 *      Author: chiu
 */
#include "dsh.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <errno.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include "builtins.h"

#define CAP 256

// TODO: Your function definitions (declarations in dsh.h)
char** split(char* str, char* delim){
	int numTokens = 0;

	//count number of delim's that exist in string str
	for(int i = 0; i < strlen(str); i++){
		if(str[i] == *delim){
			numTokens++;
		}
	}

	numTokens = numTokens + 1;

	//allocate space on heap with malloc
	char **array = (char**) malloc((numTokens + 1) * sizeof(char*));

	//loop through each array element and instantiates an array of capacity CAP
 	for(int i = 0; i < numTokens; i++){
   		array[i] = (char*) malloc(CAP * sizeof(char));
 	}

    //split str on delims
	char *token;
	
	//get first token
	token = strtok(str, delim);
	
	int i = 0;
	//loop through other tokens in string
	while(token != NULL){
		strcpy(array[i], token);
		i++;
		token = strtok(NULL, delim);
	}

	//assign final spot in array to null
	array[i] = NULL;

    //return split array
	return array;
}

int mode1(char** path){
    //printf("Mode 1!\n");

    //checks if we want to execute command in background or not
    int background = 0; //set background to FALSE

    //check if given path exists with access()
    if(access(*path, F_OK | X_OK) == 0){
        //File exists and is executable! Can run!
        //printf("File exists\n");
        
        //count number of arguments in path
        int symbol = 0;
		while (path[symbol] != NULL){
			symbol++;
		}

        //check if last char is & symbol
        if(*path[symbol - 1] == '&'){
            //printf("& symbol\n");
            //set background to TRUE
            background = 1;
        }

        pid_t childID;


        //malloc space for args on heap
        char **args = (char**) malloc(CAP * sizeof(char*));
        
        //loop through each arg element and instantiates an array of capacity CAP
 	    for(int i = 0; i < symbol; i++){
   		    args[i] = (char*) malloc(CAP * sizeof(char));
 	    }

        //store full path to executable file in args[0]
        strcpy(args[0], path[0]);

        //store additional args
        int i = 1;
        while (path[i] != NULL){
            strcpy(args[i], path[i]);
            //printf("args[%d]: %s\n", i, args[i]);
            i++;
        }

        //set last element in args to null
        args[i] = NULL;

        //fork
        childID = fork();

        //prints if fork failed
        if(childID == -1){
            printf("Failed fork");
        }

        //in parent process
        if (childID != 0){
            //printf("I am a parent, pid=%d, my child is pid=%d\n", getpid(), childID);
            
            //if background is FALSE, wait() bc we want to execute in foreground
            if(background == 0){
                //parent waits until child terminates to continue
                wait(NULL);
            }

            //loop through each array element and frees it
 		    i = 0;
		    for(i = 0; args[i] != NULL; i++){
			    //printf("i: %d \n", i);
   			    free(args[i]);
			    args[i] = NULL;
 		    }

		    free(args);
		    args = NULL;

            //printf("Done with execv\n");
            return 0;
        }
        //in child process
        else if(childID == 0){
            //printf("I am a child, pid=%d, my parent is pid=%d\n", getpid(), getppid());

            //execute file
            int status = execv(args[0], args);

            //prints if execv() failed
            if(status == -1){
                printf("execv() error\n");
            }

            //loop through each array element and frees it
 		    i = 0;
		    for(i = 0; args[i] != NULL; i++){
			    //printf("i: %d\n", i);
   			    free(args[i]);
			    args[i] = NULL;
 		    }

		    free(args);
		    args = NULL;

            //printf("returning?");
            return 1;
        }

    }
    //File doesn't exist or is not executable
    else{
        //Alert user and re-prompt
        printf("File does not exist or is not executable\n");
        return 1;
    }

    return 0;

}

int mode2(char** path){
    //count number args in path
    int numArgs = 0;
	while (path[numArgs] != NULL){
		numArgs++;
    }
    
    //check if file exists in curr dir

    //get curr dir
    char cwd[MAXBUF];
    getcwd(cwd, sizeof(cwd));
    
    //concat file to curr dir
    strcat(cwd, "/");
    strcat(cwd, path[0]);
    
    //check if file exists
    if(access(cwd, F_OK | X_OK) == 0){
        //File exists and is executable! Can run!
        //printf("File exists\n");

        //move full path into path[0]
        strcpy(path[0], cwd);

        mode1(path);

        return 1;
    }
    else{
        //printf("File does not exist!\n");
        //get paths stores in environmental var PATH
        char *envPath = getenv("PATH");
		//printf("PATH: %s\n", envPath);

        //create copy of path
        char *copyPath = (char*) malloc((strlen(envPath) + 1) * sizeof(char));
        
        //move PATH into copyPath
        strcpy(copyPath, envPath);

        //split copyPath on ":"
        char** splitPath = split(copyPath, ":");

        //check if the file exists in splitPath
        int i = 0;
		for(i = 0; splitPath[i] != NULL; i++){
            //concat file to curr dir
            strcat(splitPath[i], "/");
            strcat(splitPath[i], path[0]);
            
            //check if file exists
            if(access(splitPath[i], F_OK | X_OK) == 0){
                //File exists and is executable! Can run!
                //printf("File exists\n");
                
                //pass args onto cwd_ptr
                strcpy(path[0], splitPath[i]);

                mode1(path);

                //free copyPath
                free(copyPath);
		        copyPath = NULL;
                
                //free splitPath
		        i = 0;
		        for(i = 0; splitPath[i] != NULL; i++){
   			        free(splitPath[i]);
			        splitPath[i] = NULL;
 		        }

		        free(splitPath);
		        splitPath = NULL;

                return 1;
            }
            //check[0] = '\0';
        }

 		//free heap
		i = 0;
		for(i = 0; splitPath[i] != NULL; i++){
   			free(splitPath[i]);
			splitPath[i] = NULL;
 		}

		free(splitPath);
		splitPath = NULL;

        free(copyPath);
		copyPath = NULL;

        //if command not found, let user know
        printf("ERROR: Command not found!\n");

    }

    return 0;
}