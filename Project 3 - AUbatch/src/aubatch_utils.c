/*
	--------------------------------------------------

	# Information

	File: aubatch_utils.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file provides several utilities for use throughout AUbatch.

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
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Project Libraries
#include "aubatch_utils.h"
#include "aubatch_logger.h"

// Module Name
#define AUBATCH_MODULE_NAME "UTILS"

/*
	UTILITIES
*/

void aubatch_printWithoutPrefix(const char *message, va_list args) {

	// Print without prefix
	vprintf(message, args);

}

void aubatch_printWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[AUBatch] ");
	vprintf(message, args);
	printf("\n");

}

void aubatch_warnWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[AUBatch WARNING] ");
	vprintf(message, args);
	printf("\n");

}

void aubatch_errorWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[AUBatch ERROR] ");
	vprintf(message, args);
	if (errno != 0) {
		printf("\nDetailed Inforation: Error Code %d - %s", errno, strerror(errno));
	}
	printf("\n");
	
}

void aubatch_debugWithPrefix(const char *message, va_list args) {

	// Print with prefix
	printf("[AUBatch DEBUG] ");
	vprintf(message, args);
	printf("\n");

}

char* aubatch_trimWhitespace(char *str) {

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

bool aubatch_parseUnsignedLong(const char *str, unsigned long *out) {

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

bool aubatch_parseUnsignedInt32(const char *str, uint32_t *out) {

	// Parse as unsigned long
	unsigned long parsedValue;
	bool parsingSuccess = aubatch_parseUnsignedLong(str, &parsedValue);
	if (!parsingSuccess) { return false; }

	// Check that the parsed value fits in uint32_t
	if (parsedValue > UINT32_MAX) { return false; }

	// Set output and return
	*out = (uint32_t)parsedValue;
	return true;

}

bool aubatch_parseDouble(const char *str, double *out) {

	// Set errno to 0 to detect errors from parsing
	errno = 0;

	// Check that string starts with a number or a decimal
	if (!isdigit(str[0]) && str[0] != '.') {
		return false;
	}

	// Try to parse
	char *endCharacter;
	double parsedValue = strtod(str, &endCharacter);

	// Check for any errors
	if ((errno != 0) || (*endCharacter != '\0') || (endCharacter == str)) { 
		return false; 
	}

	// Set output value and return success
	*out = parsedValue;
	return true;

}

bool aubatch_parseBool(const char *str, bool *out) {

	// Check for true
	if ((strcmp(str, "true") == 0) || (strcmp(str, "1") == 0) || (strcmp(str, "yes") == 0)) {
		*out = true;
		return true;
	} 
	// Check for false
	else if ((strcmp(str, "false") == 0) || (strcmp(str, "0") == 0) || (strcmp(str, "no") == 0)) {
		*out = false;
		return true;
	}

	// If we got here, parsing failed
	return false;

}

void aubatch_convertTimeToHHMMSS(time_t timeInSeconds, char* outputBuffer) {

	// Get ctime of input time
	char* ctimestring = ctime(&timeInSeconds);

	// Copy just the HH:MM:SS part of the ctime string to output
	strncpy(outputBuffer, ctimestring + 11, 8);
	outputBuffer[8] = '\0';

}