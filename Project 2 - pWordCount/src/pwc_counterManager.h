/*
	--------------------------------------------------

	# Information

	File: pwc_counterManager.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the counter manager module for pWordCount.

	--------------------------------------------------

	## References
	
	1.

	--------------------------------------------------
*/

/*
	# pwc_initCounterManager

	Initializes the counter manager to track counter processes and distribute text.

	## Arguments
	- int numberOfCounterProcesses: The number of counter processes to create.
	- int writePipeFileDescriptor: The file descriptor of the write end of the counter-to-reader pipe.
	- int readPipeFileDescriptor: The file descriptor of the read end of the reader-to-counter pipe.

	## Returns
	- int: 0 if successful, -1 for error.
*/
int pwc_initCounterManager(int numberOfCounterProcesses, int writePipeFileDescriptor, int readPipeFileDescriptor);