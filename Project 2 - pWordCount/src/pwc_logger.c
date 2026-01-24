/*
	--------------------------------------------------

	# Information

	File: pwc_utils.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file provides a convenient logger for the project.

	--------------------------------------------------

	## References
	
	1.https://www.geeksforgeeks.org/c/enumeration-enum-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with enums in C.

	2. https://stackoverflow.com/questions/3134757/define-or-enum
	-> I used this article to theorize between using an enum or defines for my log levels.

	3. https://stackoverflow.com/questions/1442116/how-can-i-get-the-date-and-time-values-in-a-c-program
	4. https://stackoverflow.com/questions/9101590/fprintf-and-ctime-without-passing-n-from-ctime
	5. https://www.geeksforgeeks.org/cpp/strftime-function-in-c/
	6. https://man7.org/linux/man-pages/man3/strftime.3.html
	-> I used these resources to debate the best way to format my datetime string.

	7. https://www.geeksforgeeks.org/c/c-switch-statement/
	-> I used this GeeksForGeeks article to refamiliarize myself with switch syntax.

	8. https://www.geeksforgeeks.org/c/variadic-functions-in-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with making variadic functions for custom logging.

	--------------------------------------------------
*/

// Libraries
#include <time.h>
#include <stdio.h>
#include <stdarg.h>

// Project Libraries
#include "pwc_logger.h"

// Module Name
#define PWC_MODULE_NAME "LOGGER"

void pwc_logToFile(enum pwc_loggerLevel level, const char* module, const char* message, ...) {

	// Get the current time 
	time_t currentTime = time(NULL);
	struct tm dateTimeInformation = *localtime(&currentTime);

	// Prepare date/time into formatted string
	char dateTimeString[100];
	ssize_t numBytesForTimeString = strftime(dateTimeString, sizeof(dateTimeString), "%m-%d-%Y %H:%M:%S", &dateTimeInformation);
	
	// Catch errors when preparing time string
	if (!numBytesForTimeString) {
		return;
	}

	// Prepare log level string
	char* logLevelString;
	switch (level) {
		case PWC_LOGLEVEL_INFO:
			logLevelString = "INFO";
			break;
		case PWC_LOGLEVEL_DEBUG:
			logLevelString = "DEBUG";
			break;
		case PWC_LOGLEVEL_WARNING:
			logLevelString = "WARNING";
			break;
		case PWC_LOGLEVEL_ERROR:
			logLevelString = "ERROR";
			break;
		default:
			logLevelString = "UNKNOWN";
	}

	// Open file for appending
	FILE* logFile = fopen(PWC_LOGFILE_PATH, "a");
	if (!logFile) {
		return;
	}

	// Write log entry to file
	fprintf(logFile, "[%s] [%s] [%s]: ", dateTimeString, logLevelString, module);
	va_list args;
	va_start(args, message);
	vfprintf(logFile, message, args);
	va_end(args);
	fputc('\n', logFile);
	fclose(logFile);

	// Exit
	return;
}