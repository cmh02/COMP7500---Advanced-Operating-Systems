/*

	--------------------------------------------------

	# Information

	File: pwc_utils.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides several utilities for use throughout pWordCount.

	--------------------------------------------------

	## References

	1. https://www.geeksforgeeks.org/c/variadic-functions-in-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with making variadic functions for custom printing.

	2. https://sourceforge.net/p/predef/wiki/OperatingSystems/
	3. https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
	-> I used these to understand how to detect Apple vs Linux and define macro for memory unit.

	4. https://man7.org/linux/man-pages/man3/clock_gettime.3.html
	https://stackoverflow.com/questions/64893834/measuring-elapsed-time-using-clock-gettimeclock-monotonic
	-> I used this resource to understand best practices for tracking precise time.

	--------------------------------------------------

*/

// Libraries
#include <stdarg.h>
#include <stdbool.h>

// Prevent multiple inclusions
#ifndef PWC_UTILS_H
#define PWC_UTILS_H



// Memory Unit

/* 
	# Note on Memory Units

	As different systems report mem usage in different units (MAC is bytes, Linux is KB), this gives an easy way to print out the right unit.
*/
#if defined(__APPLE__) && defined(__MACH__)
	#define PWC_MEMORY_UNIT "bytes"
#else
	#define PWC_MEMORY_UNIT "kilobytes"
#endif



// Execution Time Monitoring (Structure and Calculation)

/*
	# Execution Time Structure

	This struct will allow for easier tracking of execution time for different modules.

	## Structure Members
	- startTime : The starting time in microseconds
	- endTime : The ending time in microseconds
	- totalTime : The total time in microseconds
*/
struct pwc_executionTimeStruct {
	struct timespec start;
	struct timespec end;
	double total;
};

/*
	# Start Execution Time Tracking

	This function will start tracking execution time. This is recorded via 
*/
void pwc_startExecutionTimeTracking(struct pwc_executionTimeStruct* execTimeStruct);
void pwc_stopExecutionTimeTracking(struct pwc_executionTimeStruct* execTimeStruct);



/*
	# Configuration Options Structure

	This structure will define the possible config options to be set from a file.
*/
struct pwc_configuration {
	bool LOGGING_SEND_DEBUG_TO_LOG;
	bool LOGGING_SEND_DEBUG_TO_STDOUT;
	unsigned long NUMBER_OF_PROCESSES;
	unsigned long BUFFER_SIZE_READER;
	unsigned long BUFFER_SIZE_COUNTERMANAGER;
	unsigned long BUFFER_SIZE_COUNTER;
	char LOGGING_DIRECTORY;
};



// Function Stubs

/*
	# Print With Prefix

	Print a message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_printWithPrefix(const char *message, va_list args);

/*
	# Warn With Prefix

	Print a warning message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_warnWithPrefix(const char *message, va_list args);

/*
	# Error With Prefix

	Print an error message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_errorWithPrefix(const char *message, va_list args);

/*
	# Load Configuration File

	Load configuration options from a config file into the given configuration struct.

	## Arguments
	- filePath : The path to the configuration file
	- config : Pointer to a pwc_configuration struct to load options into

	## Returns
	- int : 0 on success, 1 on failure
*/
int pwc_loadConfigurationFile(const char* filePath, struct pwc_configuration* config);

/*
	# Trim Whitespace

	Trim leading and trailing whitespace from a string. Must be
	a modifiable string so we can place the new null terinator.

	## Arguments
	- str : Pointer to the string to trim whitespace from

	## Returns
	- char* : Pointer to the start of the trimmed string with null terminator at end
*/
char* pwc_trimWhitespace(char *str);

/*
	# Parse Unsigned Long

	Parse unsigned int value from string with enhanced error detection.

	## Arguments
	- str : Pointer to the string to parse
	- out : Pointer to where the unsigned long variable should be stored

	## Returns
	- bool : True if parsing was successful, false otherwise
*/
bool pwc_parseUnsignedLong(const char *str, unsigned long *out);

#endif // PWC_UTILS_H
