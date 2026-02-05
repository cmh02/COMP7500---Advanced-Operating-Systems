/*
	--------------------------------------------------

	# Information

	File: pwc_counter.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the counter module for pWordCount.

	--------------------------------------------------

	## References
	
	1.

	--------------------------------------------------
*/

// Prevent multiple inclusions
#ifndef PWC_COUNTER_H
#define PWC_COUNTER_H

/*
	# pwc_counter_countWordsFromPipe

	Counts the number of words from a specified pipe.

	## Arguments
	- int writePipeFileDescriptor: The file descriptor of the write end of the counter-to-reader pipe.
	- int readPipeFileDescriptor: The file descriptor of the read end of the reader-to-counter pipe.

	## Returns
	- int: 0 if successful, -1 for error.
*/
int pwc_counter_countWordsFromPipe(int writePipeFileDescriptor, int readPipeFileDescriptor);

/*
	# pwc_countWordsInBuffer

	Counts the number of words in a specified buffer.

	## Arguments
	- char* buffer: The buffer containing text to count words from.
	- ssize_t bufferSize: The size of the buffer in bytes.

	## Returns
	- int: The number of words counted in the buffer.
*/
int pwc_countWordsInBuffer(char* buffer, ssize_t bufferSize);

#endif // PWC_COUNTER_H