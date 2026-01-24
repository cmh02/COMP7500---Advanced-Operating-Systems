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

	--------------------------------------------------
*/

// System Libraries
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_counter.h"

// Module Name
#define PWC_MODULE_NAME "COUNTER"

// Constants
#define COUNT_BUFFER_SIZE 4096

int pwc_counter_countWordsFromPipe(int writePipeFileDescriptor, int readPipeFileDescriptor) {
	
	// Initialize word count
	int wordCount = 0;

	// Create a buffer to hold chunks of data from the pipe
	char textCountingBuffer[COUNT_BUFFER_SIZE];

	// Keep reading from the pipe until no more data is available
	ssize_t numberBytesRead;
	while ((numberBytesRead = read(readPipeFileDescriptor, textCountingBuffer, COUNT_BUFFER_SIZE)) > 0) {

		// Count words in the buffer using helper function
		int chunkWordCount = pwc_countWordsInBuffer(textCountingBuffer, numberBytesRead);
		wordCount += chunkWordCount;
	}

	// Check for any errors while reading from the pipe
	if (numberBytesRead < 0) {
		pwc_errorWithPrefix("The attempt to read data from the pipe has failed!");
		close(readPipeFileDescriptor);
		close(writePipeFileDescriptor);
		exit(-1);
	}

	// Close the read end of the pipe
	close(readPipeFileDescriptor);

	// Write the final word count to the write pipe
	ssize_t numberBytesWritten = write(writePipeFileDescriptor, &wordCount, sizeof(wordCount));

	// Check for any errors while writing to the pipe
	if (numberBytesWritten != sizeof(wordCount)) {
		pwc_errorWithPrefix("The attempt to write the final word count to the pipe has failed!");
		close(writePipeFileDescriptor);
		exit(-1);
	}

	// Close the write end of the pipe
	close(writePipeFileDescriptor);

	// Exit successfully
	exit(0);
}

int pwc_countWordsInBuffer(char* buffer, ssize_t bufferSize) {

	// Initialize word count
	int wordCount = 0;

	// Initialize a flag to track if we are inside a word
	bool flag_currentlyInWord = false;

	// Iterate over the bytes currently in the buffer
	for (ssize_t i = 0; i < bufferSize; i++) {
		
		// If the current character is not whitespace and we are not currently in word, then we are now in a word
		if (!isspace(buffer[i]) && !flag_currentlyInWord) {
			flag_currentlyInWord = true;
			wordCount++;
		}

		// If we reach whitespace, then we are no longer in a word
		else if (isspace(buffer[i])) {
			flag_currentlyInWord = false;
		}
	}

	// Return the word count
	return wordCount;
}