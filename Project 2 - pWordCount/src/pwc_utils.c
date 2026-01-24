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
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

// Project Libraries
#include "pwc_utils.h"

// Module Name
#define PWC_MODULE_NAME "UTILS"

/*
	UTILITIES
*/

void pwc_printWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[pWordCount] ");
	vprintf(message, args);
	printf("\n");

}

void pwc_warnWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[pWordCount WARNING] ");
	vprintf(message, args);
	printf("\n");

}

void pwc_errorWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[pWordCount ERROR] ");
	vprintf(message, args);
	if (errno != 0) {
		printf("\nDetailed Inforation: Error Code %d - %s", errno, strerror(errno));
	}
	printf("\n");
	
}

void pwc_startExecutionTimeTracking(struct pwc_executionTimeStruct* execTimeStruct) {
	
	// Get start time
	clock_gettime(CLOCK_MONOTONIC, &execTimeStruct->start);

}

void pwc_stopExecutionTimeTracking(struct pwc_executionTimeStruct* execTimeStruct) {
	
	// Get end time
	clock_gettime(CLOCK_MONOTONIC, &execTimeStruct->end);

	// Calculate total time in seconds
	execTimeStruct->total = (execTimeStruct->end.tv_sec - execTimeStruct->start.tv_sec) + 
							(execTimeStruct->end.tv_nsec - execTimeStruct->start.tv_nsec) / 1e9;

}