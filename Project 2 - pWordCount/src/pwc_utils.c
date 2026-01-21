/*
	--------------------------------------------------

	# Information

	File: pwc_utils.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file provides several utilities for use throughout pWordCount.

	--------------------------------------------------

	# References

	1. https://www.geeksforgeeks.org/c/variadic-functions-in-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with making variadic functions for custom printing.

	--------------------------------------------------
*/

// Libraries
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include "pwc_utils.h"

/*
	UTILITIES
*/

void pwc_printWithPrefix(const char *message, ...) {

	// Prepare variadic args
	va_list args;
	va_start(args, message);

	// Print with prefix
	printf("[pWordCount] ");
	vprintf(message, args);
	printf("\n");
	
	// Cleanup variadic args
	va_end(args);
}

void pwc_warnWithPrefix(const char *message, ...) {

	// Prepare variadic args
	va_list args;
	va_start(args, message);

	// Print with prefix
	printf("[pWordCount WARNING] ");
	vprintf(message, args);
	printf("\n");

	// Cleanup variadic args
	va_end(args);
}

void pwc_errorWithPrefix(const char *message, ...) {

	// Prepare variadic args
	va_list args;
	va_start(args, message);

	// Print with prefix
	printf("[pWordCount ERROR] ");
	vprintf(message, args);
	if (errno != 0) {
		printf("\nDetailed Inforation: Error Code %d - %s", errno, strerror(errno));
	}
	printf("\n");

	// Cleanup variadic args
	va_end(args);
}