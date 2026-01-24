/*
	--------------------------------------------------

	# Information

	File: pwc_reader.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the reader module for pWordCount.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/c/input-output-system-calls-c-create-open-close-read-write/
	-> I used this GeeksForGeeks article to further understand behavior of open(), read(), and write() calls.

	--------------------------------------------------
*/

// System Libraries
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_reader.h"

// Module Name
#define PWC_MODULE_NAME "READER"

// Constants
#define FILE_READ_BUFFER_SIZE 4096

int pwc_reader_streamFileToPipe(const char* filePath, int writePipeFileDescriptor, int readPipeFileDescriptor) {
	
	// Open the file in read-only mode
	int textFileDescriptor = open(filePath, O_RDONLY);

	// Confirm that the file was opened successfully
	if (textFileDescriptor < 0) {
		pwc_errorWithPrefix("The attempt to open file '%s' has failed!", filePath);
		return -1;
	}

	// Ceate a buffer to hold chunks of text from file
	char textReadingBuffer[FILE_READ_BUFFER_SIZE];

	// Keep reading from the file in chunks until end of file is reached
	ssize_t numberBytesRead;
	while ((numberBytesRead = read(textFileDescriptor, textReadingBuffer, FILE_READ_BUFFER_SIZE)) > 0) {

		// Write the bytes to the pipe
		ssize_t totalBytesWritten = 0;
		while (totalBytesWritten < numberBytesRead) {
			ssize_t numberBytesWritten = write(writePipeFileDescriptor, textReadingBuffer, numberBytesRead);
			totalBytesWritten += numberBytesWritten;
		}

		// Validate that we wrote all bytes, else exit with error
		if (totalBytesWritten != numberBytesRead) {
			pwc_errorWithPrefix("An error occurred while writing to the pipe in the reader module!");
			close(writePipeFileDescriptor);
			close(readPipeFileDescriptor);
			close(textFileDescriptor);
			return -1;
		}
	}

	// Check for any errors while reading from the file
	if (numberBytesRead < 0) {
		pwc_errorWithPrefix("The attempt to read data from file '%s' has failed!", filePath);
		close(writePipeFileDescriptor);
		close(readPipeFileDescriptor);
		close(textFileDescriptor);
		return -1;
	}

	// Close the write end of the pipe to signal end of data for the counter
	close(writePipeFileDescriptor);

	// Read the final count from the counter via the read pipe
	int finalWordCount;
	ssize_t numberBytesReadFromPipe = read(readPipeFileDescriptor, &finalWordCount, sizeof(finalWordCount));

	// Check for any errors while reading from the pipe
	if (numberBytesReadFromPipe < 0) {
		pwc_errorWithPrefix("The attempt to read the final word count from the pipe has failed!");
		close(readPipeFileDescriptor);
		close(textFileDescriptor);
		return -1;
	}

	// Close the read end of the pipe
	close(readPipeFileDescriptor);

	// Close the file descriptor
	close(textFileDescriptor);

	// Return the final word count
	return finalWordCount;
}
