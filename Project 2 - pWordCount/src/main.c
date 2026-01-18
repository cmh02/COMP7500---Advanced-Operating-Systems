/*
	--------------------------------------------------

	# Information

	File: main.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	Main execution file for pWordCount.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/c/fork-system-call/
	-> I used this GeeksForGeeks article to help understand the fork() call and its underlying behavior when creating processes.

	2. https://www.geeksforgeeks.org/c/pipe-system-call/
	-> I used this GeeksForGeeks article to familarize myself with pipe() and correct read/write syntax.

	--------------------------------------------------
*/

// Libraries
#include <stdio.h>
#include <unistd.h>
#include "pwc_utils.h"
#include "pwc_reader.h"
#include "pwc_counter.h"

// Main Execution
int main(int argc, char **argv) {

	// Initialize two pipes, one for reader->counter and one for counter->reader
	int readerToCounterPipeFileDescriptor[2];
	int counterToReaderPipeFileDescriptor[2];

	// Create the pipes and handle any errors
	if (pipe(readerToCounterPipeFileDescriptor) == -1) {
		pwc_errorWithPrefix("The attempt to create the reader-to-counter pipe has failed!");
		return 1;
	}
	if (pipe(counterToReaderPipeFileDescriptor) == -1) {
		pwc_errorWithPrefix("The attempt to create the counter-to-reader pipe has failed!");
		return 1;
	}

	// Split into parent and child processes
	pid_t pid = fork();

	// Handle child process, which will be used for counting the number of words via pipe
	if (pid == 0) {
		
		// Only read from the reader-to-counter pipe
		close(readerToCounterPipeFileDescriptor[1]);

		// Only write to the counter-to-reader pipe
		close(counterToReaderPipeFileDescriptor[0]);

		// Start up the counter module to count words from the pipe
		int counterStatus = pwc_counter_countWordsFromPipe(counterToReaderPipeFileDescriptor[1], readerToCounterPipeFileDescriptor[0]);

		// Close pipes when done
		close(readerToCounterPipeFileDescriptor[0]);
		close(counterToReaderPipeFileDescriptor[1]);
	}

	// Handle parent process, which will be used for reading words from specified file
	else if (pid > 0) {
		
		// Only write to the reader-to-counter pipe
		close(readerToCounterPipeFileDescriptor[0]);

		// Only read from the counter-to-reader pipe
		close(counterToReaderPipeFileDescriptor[1]);

		// Start up the reader module to stream file to the pipe
		const char* filePath = "sample.txt"; // Example file path
		int readerStatus = pwc_reader_streamFileToPipe(filePath, readerToCounterPipeFileDescriptor[1], counterToReaderPipeFileDescriptor[0]);

		// Close pipes when done
		close(readerToCounterPipeFileDescriptor[1]);
		close(counterToReaderPipeFileDescriptor[0]);
	}

	// Error detection incase fork fails
	else {

		pwc_errorWithPrefix("The attempt to fork() into parent and child processes has failed!");
		return 1;
	}

	// Main return
	return 0;
}
