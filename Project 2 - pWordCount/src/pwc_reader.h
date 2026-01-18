/*
	--------------------------------------------------

	# Information

	File: pwc_reader.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the reader module for pWordCount.

	--------------------------------------------------

	## References
	
	1.

	--------------------------------------------------
*/

/*
	# pwc_reader_streamFileToPipe

	Streams the contents of a file to a specified pipe.

	## Arguments
	- const char* filePath: The path to the file to be read.
	- int writePipeFileDescriptor: The file descriptor of the write end of the reader-to-counter pipe.
	- int readPipeFileDescriptor: The file descriptor of the read end of the counter-to-reader pipe.

	## Returns
	- int: 0 if successful, -1 for error.
*/
int pwc_reader_streamFileToPipe(const char* filePath, int writePipeFileDescriptor, int readPipeFileDescriptor);