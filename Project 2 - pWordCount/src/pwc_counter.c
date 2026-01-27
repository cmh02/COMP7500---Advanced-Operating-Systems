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
#include "pwc_config.h"
#include "pwc_logger.h"
#include "pwc_counter.h"

// Module Name
#define PWC_MODULE_NAME "COUNTER"

int pwc_counter_countWordsFromPipe(int writePipeFileDescriptor, int readPipeFileDescriptor) {
	
	// Log
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Counter process with PID %d has started counting words from the pipe!", getpid());

	// Get config
	struct pwc_configuration* config = pwc_configuration();

	// Initialize word count
	int wordCount = 0;

	// Create a buffer to hold chunks of data from the pipe
	char textCountingBuffer[config->BUFFER_SIZE_COUNTER];

	// Keep reading from the pipe until no more data is available
	ssize_t numberBytesRead;
	while ((numberBytesRead = read(readPipeFileDescriptor, textCountingBuffer, config->BUFFER_SIZE_COUNTER)) > 0) {

		// Count words in the buffer using helper function
		int chunkWordCount = pwc_countWordsInBuffer(textCountingBuffer, numberBytesRead);
		wordCount += chunkWordCount;
	}

	// Check for any errors while reading from the pipe
	if (numberBytesRead < 0) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to read data from the pipe has failed!");
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
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to write the final word count to the pipe has failed!");
		close(writePipeFileDescriptor);
		exit(-1);
	}

	// Close the write end of the pipe
	close(writePipeFileDescriptor);

	// Log
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Counter process with PID %d finished counting with a total of %d words!", getpid(), wordCount);

	// Log counter usage stats
	struct rusage cStats;
	if (getrusage(RUSAGE_SELF, &cStats) == 0) {
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Memory Usage Statistics for Counter process with PID %d:", getpid());
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> User CPU Time Used: %ld seconds and %ld microseconds", cStats.ru_utime.tv_sec, cStats.ru_utime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> System CPU Time Used: %ld seconds and %ld microseconds", cStats.ru_stime.tv_sec, cStats.ru_stime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Maximum Resident Set Size: %ld %s", cStats.ru_maxrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Shared Memory Size: %ld %s", cStats.ru_ixrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Data Size: %ld %s", cStats.ru_idrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Stack Size: %ld %s", cStats.ru_isrss, PWC_MEMORY_UNIT);
	} else {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "An error occurred while attempting to get resource usage statistics for counter process with PID %d.", getpid());
	}

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