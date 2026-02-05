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

// Prevent multiple inclusions
#ifndef PWC_COUNTERMANAGER_H
#define PWC_COUNTERMANAGER_H

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

#endif // PWC_COUNTERMANAGER_H