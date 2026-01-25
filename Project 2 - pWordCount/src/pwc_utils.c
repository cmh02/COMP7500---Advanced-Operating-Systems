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

int pwc_loadConfigurationFile(const char* filePath, struct pwc_configuration* config) {

	// Open config file and detect any errors
	FILE* configFile = fopen(filePath, "r");
	if (configFile == NULL) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to open the configuration file at path '%s' has failed!", filePath);
		return 1;
	}

	// Make a null pointer to a buffer to be used for parsing entire lines, avoids setting too small buffer size
	char* lineContents = NULL;
	size_t lineContentsBufferLength = 0;
	ssize_t lineContentsReadLength;

	// Read line by line through the config file
	bool withinComment = false;
	while ((lineContentsReadLength = getline(&lineContents, &lineContentsBufferLength, configFile)) != -1) {

		// If the line has no content, skip it
		if (lineContentsReadLength == 0) { continue; }

		// Trim whitesapce
		char *lineContentsTrimmed = pwc_trimWhitespace(lineContents);

		// If the line begins a multiline comment (/*), set flag and skip
		if (!strncmp(lineContentsTrimmed, "/*", 2)) {
			withinComment = true;
			continue;
		}

		// If we are within a multiline comment, check for end (*/) and skip
		if (withinComment) {
			if (!strncmp(lineContentsTrimmed, "*/", 2)) {
				withinComment = false;
			}
			continue;
		}

		// Erase (in essence) comments starting with //
		char *commentStart = strstr(lineContentsTrimmed, "//");
		if (commentStart) {
			*commentStart = '\0';
		}

		// Trim remaining whitespace after clearing comment
		lineContentsTrimmed = pwc_trimWhitespace(lineContentsTrimmed);

		// Find the key and value by splitting at =
		char *seperator = strchr(lineContentsTrimmed, '=');

		// Check that we found a valid seperator, if not, skip and warn
		if (!seperator) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid non-comment line in configuration file, missing '=': %s!", lineContentsTrimmed);
			continue;
		}

		// Split at the seperator
		*seperator = '\0';
		char *key = pwc_trimWhitespace(lineContentsTrimmed);
		char *value = pwc_trimWhitespace(seperator + 1);

		// Validate that key and value are actually present
		if (strlen(key) == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line, missing key before '=': %s!", lineContentsTrimmed);
			continue;
		}
		if (strlen(value) == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line, missing value after '=': %s!", lineContentsTrimmed);
			continue;
		}
		if (key == "\0") {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line, empty key before '=': %s!", lineContentsTrimmed);
			continue;
		}
		if (value == "\0") {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line, empty value after '=': %s!", lineContentsTrimmed);
			continue;
		}

		// Check for keys, update struct, detect errors
		if (!strcmp(key, "LOGGING_SEND_DEBUG_TO_LOG")) {
			if (strcmp(value, "true") == 0) {
				config->LOGGING_SEND_DEBUG_TO_LOG = true;
			} 
			else if (strcmp(value, "false") == 0) {
				config->LOGGING_SEND_DEBUG_TO_LOG = false;
			}
			else {
				config->LOGGING_SEND_DEBUG_TO_LOG = true;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid boolean value for LOGGING_SEND_DEBUG_TO_LOG: %s, using default value of '%s'!", value, config->LOGGING_SEND_DEBUG_TO_LOG ? "true" : "false");
			}
		}
		if (!strcmp(key, "LOGGING_SEND_DEBUG_TO_STDOUT")) {
			if (strcmp(value, "true") == 0) {
				config->LOGGING_SEND_DEBUG_TO_STDOUT = true;
			} 
			else if (strcmp(value, "false") == 0) {
				config->LOGGING_SEND_DEBUG_TO_STDOUT = false;
			}
			else {
				config->LOGGING_SEND_DEBUG_TO_STDOUT = true;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid boolean value for LOGGING_SEND_DEBUG_TO_STDOUT: %s, using default value of '%s'!", value, config->LOGGING_SEND_DEBUG_TO_STDOUT ? "true" : "false");
			}
		}
		else if (!strcmp(key, "LOGGING_DIRECTORY")) {
			config->LOGGING_DIRECTORY = value[0];
		}
		else if (!strcmp(key, "NUMBER_OF_PROCESSES")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->NUMBER_OF_PROCESSES = parsedValue;
			} else {
				config->NUMBER_OF_PROCESSES = 1;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for NUMBER_OF_PROCESSES: %s, using default value of '%lu'!", value, config->NUMBER_OF_PROCESSES);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_READER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_READER = parsedValue;
			} else {
				config->BUFFER_SIZE_READER = 4096;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for BUFFER_SIZE_READER: %s, using default value of '%lu'!", value, config->BUFFER_SIZE_READER);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_COUNTERMANAGER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_COUNTERMANAGER = parsedValue;
			} else {
				config->BUFFER_SIZE_COUNTERMANAGER = 4096;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for BUFFER_SIZE_COUNTERMANAGER: %s, using default value of '%lu'!", value, config->BUFFER_SIZE_COUNTERMANAGER);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_COUNTER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_COUNTER = parsedValue;
			} else {
				config->BUFFER_SIZE_COUNTER = 4096;
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for BUFFER_SIZE_COUNTER: %s, using default value of '%lu'!", value, config->BUFFER_SIZE_COUNTER);
			}
		}

	}
	
	// Still have to free the buffer even though we don't allocate (per man page)
	free(lineContents);

	// Close config file
	fclose(configFile);

	// Return success
	return 0;

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