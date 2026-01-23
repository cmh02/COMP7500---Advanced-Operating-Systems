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
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_counter.h"

// Constants
#define COUNT_BUFFER_SIZE 4096

int pwc_counter_countWordsFromPipe(int writePipeFileDescriptor, int readPipeFileDescriptor) {
	
	// Initialize word count
	int wordCount = 0;

	// Initialize a flag to track if we are inside a word across seperate pipe reads
	bool flag_currentlyInWord = false;

	// Create a buffer to hold chunks of data from the pipe
	char textCountingBuffer[COUNT_BUFFER_SIZE];

	// Keep reading from the pipe until no more data is available
	ssize_t numberBytesRead;
	while ((numberBytesRead = read(readPipeFileDescriptor, textCountingBuffer, COUNT_BUFFER_SIZE)) > 0) {
		// Iterate over the bytes currently in the buffer
		for (ssize_t i = 0; i < numberBytesRead; i++) {
			
			// If the current character is a whitespace character and we are currently in a word, then increment
			if (isspace(textCountingBuffer[i]) && flag_currentlyInWord) {
				wordCount++;
				flag_currentlyInWord = false;
			}

			// If the current character is not a whitespace character, then we are now in a word
			else if (!isspace(textCountingBuffer[i]) && !flag_currentlyInWord) {
				flag_currentlyInWord = true;
			}
		}
	}

	// Check for any errors while reading from the pipe
	if (numberBytesRead < 0) {
		pwc_errorWithPrefix("The attempt to read data from the pipe has failed!");
		close(readPipeFileDescriptor);
		close(writePipeFileDescriptor);
		return -1;
	}

	// After finishing reading, if we are still in a word, increment the count
	if (flag_currentlyInWord) {
		wordCount++;
	}

	// Close the read end of the pipe
	close(readPipeFileDescriptor);

	// Write the final word count to the write pipe
	ssize_t numberBytesWritten = write(writePipeFileDescriptor, &wordCount, sizeof(wordCount));

	// Check for any errors while writing to the pipe
	if (numberBytesWritten != sizeof(wordCount)) {
		pwc_errorWithPrefix("The attempt to write the final word count to the pipe has failed!");
		close(writePipeFileDescriptor);
		return -1;
	}

	// Close the write end of the pipe
	close(writePipeFileDescriptor);

	// Return success
	return 0;
}