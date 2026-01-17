/*
	File: main.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02



	References:
	
	1. https://www.geeksforgeeks.org/c/fork-system-call/
	-> I used this GeeksForGeeks article to help understand the fork() call and its underlying behavior when creating processes.

*/

// Libraries
#include <stdio.h>
#include <unistd.h>
#include "pwc_utils.h"

// Main Execution
int main(int argc, char **argv) {

	pwc_printWithPrefix("Program started.");
	int counter = 0;

	// Split into parent and child processes
	pid_t pid = fork();

	// Handle child process, which will be used for counting the number of words via pipe
	if (pid == 0) {
		// child process
		int i = 0;
		for (; i < 5; ++i) {
			pwc_printWithPrefix("child process: counter=%d\n", ++counter);
		}	
	}

	// Handle parent process, which will be used for reading words from specified file
	else if (pid > 0) {
		// parent process
		int j = 0;
		for (; j < 5; ++j) {
			pwc_printWithPrefix("parent process: counter=%d\n", ++counter);
		}
	}

	// Error detection incase fork fails
	else {

		pwc_errorWithPrefix("fork() failed!");
		return 1;
	}


	pwc_printWithPrefix("Program completed successfully.");

	// Main return
	return 0;
}
