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

	2. https://stackoverflow.com/questions/3501338/c-read-file-line-by-line
	3. https://man7.org/linux/man-pages/man3/getline.3.html
	-> I used these resources to find best practice for reading line by line.

	4. https://stackoverflow.com/questions/17307275/what-is-the-printf-format-specifier-for-bool
	-> I used this resource to find a nice way of printing bool values.

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
#include "pwc_logger.h"

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

char* pwc_trimWhitespace(char *str) {

	// Find the first non-whitespace character
	while (isspace((unsigned char)*str)) { str++; }

	// If the string is all spaces, then we return pointer to end, meaning empty string
	if (*str == 0) {
		return str;
	}

	// Find the last non-whitespace character
	char *end;
	end = str + strlen(str) - 1;
	while (end > str && isspace((unsigned char)*end)) { end--; }

	// Write new null terminator to remove trailing whitespace
	*(end + 1) = '\0';

	// str now points to first non-whitespace character, and terminator fixes trailing whitespace
	return str;

}

bool pwc_parseUnsignedLong(const char *str, unsigned long *out) {

	// Set errno to 0 to detect errors from parsing
	errno = 0;

	// Check that string starts with a number
	if (!isdigit(str[0])) {
		return false;
	}

	// Try to parse
	char *endCharacter;
	unsigned long parsedValue = strtoul(str, &endCharacter, 10);

	// Check for any errors
	if ((errno != 0) || (*endCharacter != '\0') || (endCharacter == str)) { 
		return false; 
	}

	// Set output value and return success
	*out = parsedValue;
	return true;

}