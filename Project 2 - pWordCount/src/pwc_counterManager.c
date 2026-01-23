/*
	--------------------------------------------------

	# Information

	File: pwc_counter.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the counter module for pWordCount.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/c/isspace-in-c/
	-> I used this GeeksForGeeks article to safely find any whitespace characters.

	2. https://www.w3schools.com/c/c_structs.php
	3. https://www.geeksforgeeks.org/c/structures-c/
	-> I used these articles to refamiliarize myself with struct syntax.

	4. https://stackoverflow.com/questions/10468128/how-do-you-make-an-array-of-structs-in-c
	5. https://www.geeksforgeeks.org/c/dynamic-array-in-c/
	-> I used these resources to understand malloc/calloc for dynamic allocation of arrays with structs.

	--------------------------------------------------
*/

// System Libraries
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_counter.h"
#include "pwc_counterManager.h"

// Constants
#define COUNT_BUFFER_SIZE 4096

// Struct for per-counter sets of pipes
struct pwc_counterPipes {
	pid_t counterPID;
	int writePipeFileDescriptor; // for writing to counter
	int readPipeFileDescriptor; // for reading from counter
};

int pwc_initCounterManager(int numberOfCounterProcesses, int writePipeFileDescriptor, int readPipeFileDescriptor) {

	// Validate number of counter processes
	if (numberOfCounterProcesses < 1) {
		pwc_errorWithPrefix("The number of counter processes must be at least 1!");
		exit(-1);
	}

	// Create array to hold all pipes for all counters (has to be dynamic array)
	struct pwc_counterPipes *counterPipesArray = calloc(numberOfCounterProcesses, sizeof(struct pwc_counterPipes));
	if (!counterPipesArray) {
		pwc_errorWithPrefix("An error occurred while allocating memory for the counter pipes array!");
		exit(-1);
	}

	// Loop to create the specified number of counter processes
	for (int i = 0; i < numberOfCounterProcesses; i++) {

		// Make new pipe for the manager -> counter
		int managerToCounterPipeFileDescriptor[2];
		if (pipe(managerToCounterPipeFileDescriptor) == -1) {
			pwc_errorWithPrefix("The attempt to create the manager-to-counter pipe has failed!");
			exit(-1);
		}

		// Make new pipe for the counter -> manager
		int counterToManagerPipeFileDescriptor[2];
		if (pipe(counterToManagerPipeFileDescriptor) == -1) {
			pwc_errorWithPrefix("The attempt to create the counter-to-manager pipe has failed!");
			exit(-1);
		}

		// Fork a new process
		pid_t pid = fork();

		// Validate fork success
		if (pid < 0) {
			pwc_errorWithPrefix("An error occurred while forking a new counter process!");
			exit(-1);
		}

		// Catch each child process
		if (pid == 0) {

			// Close unused pipe ends
			close(managerToCounterPipeFileDescriptor[1]);
			close(counterToManagerPipeFileDescriptor[0]);

			// Start up the counter module to count words from the pipe
			int counterStatus = pwc_counter_countWordsFromPipe(counterToManagerPipeFileDescriptor[1], managerToCounterPipeFileDescriptor[0]);

			// Exit child process
			exit(0);
		}

		// Close unused pipe ends
		close(managerToCounterPipeFileDescriptor[0]);
		close(counterToManagerPipeFileDescriptor[1]);	

		// Save the counter PID and pipe file descriptors in a struct
		struct pwc_counterPipes counterPipes;
		counterPipes.counterPID = pid;
		counterPipes.writePipeFileDescriptor = managerToCounterPipeFileDescriptor[1];
		counterPipes.readPipeFileDescriptor = counterToManagerPipeFileDescriptor[0];
		counterPipesArray[i] = counterPipes;
	}

	// Initialize a flag to track if the last-read chunk ended with a character
	bool flag_didLastChunkEndWithCharacter = false;

	// Initialize a counter to track which counter process to send data to
	int currentCounterIndex = 0;

	// Create a buffer to hold chunks of data from the pipe
	char textCountingBuffer[COUNT_BUFFER_SIZE];

	// Keep reading from the pipe until no more data is available
	ssize_t numberBytesRead;
	while ((numberBytesRead = read(readPipeFileDescriptor, textCountingBuffer, COUNT_BUFFER_SIZE)) > 0) {

		// Validate that we read bytes, else exit with error
		if (numberBytesRead <= 0) {
			pwc_errorWithPrefix("An error occurred while reading from the pipe in the counter manager!");
			exit(-1);
		}

		// If the previous chunk ended with a character, and the current chunk starts with character, then we have a split word
		if (flag_didLastChunkEndWithCharacter) {

			// Replace all sequential non-whitespace characters at the start of the buffer with spaces
			for (ssize_t i = 0; i < numberBytesRead; i++) {
				if (!isspace(textCountingBuffer[i])) {
					textCountingBuffer[i] = ' ';
				} else {
					break;
				}
			}
		}

		// Check the end of the current chunk to see if it ends with a character (if isspace is false, then it's a character)
		flag_didLastChunkEndWithCharacter = !isspace(textCountingBuffer[numberBytesRead - 1]);

		// Write the current chunk to the current counter process's pipe
		ssize_t numberBytesWritten = write(counterPipesArray[currentCounterIndex].writePipeFileDescriptor, textCountingBuffer, numberBytesRead);

		// Validate that we wrote all bytes, else exit with error
		if (numberBytesWritten != numberBytesRead) {
			pwc_errorWithPrefix("An error occurred while writing to the counter process pipe in the counter manager!");
			exit(-1);
		}
		// Increment counter and check if we need to reset the current counter index back to 0
		currentCounterIndex++;
		if (currentCounterIndex >= numberOfCounterProcesses) {
			currentCounterIndex = 0;
		}
	}

	// Once all chunks have been sent off, close all the write ends of pipes to signal counters to return counts
	for (int i = 0; i < numberOfCounterProcesses; i++) {
		close(counterPipesArray[i].writePipeFileDescriptor);
	}

	// Read final word counts from each counter process and sum them
	int totalWordCount = 0;
	for (int i = 0; i < numberOfCounterProcesses; i++) {
		int wordCount = 0;

		// Read and validate count
		ssize_t numberBytesReadFromPipe = read(counterPipesArray[i].readPipeFileDescriptor, &wordCount, sizeof(wordCount));
		if (numberBytesReadFromPipe != sizeof(wordCount)) {
			pwc_errorWithPrefix("An error occurred while reading from the counter process pipe in the counter manager!");
			exit(-1);
		}

		// Close the read end of the pipe
		close(counterPipesArray[i].readPipeFileDescriptor);

		// Add to total count
		totalWordCount += wordCount;
	}

	// Free up the pipes array (mem cleanup)
	free(counterPipesArray);

	// Write the total word count to the provided write pipe
	ssize_t numberBytesWrittenToOutputPipe = write(writePipeFileDescriptor, &totalWordCount, sizeof(totalWordCount));
	if (numberBytesWrittenToOutputPipe != sizeof(totalWordCount)) {
		pwc_errorWithPrefix("An error occurred while writing the total word count to the output pipe in the counter manager!");
		exit(-1);
	}

	// Return success
	exit(0);
}