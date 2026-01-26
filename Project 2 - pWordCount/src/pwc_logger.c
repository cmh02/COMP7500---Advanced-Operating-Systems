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

	9. https://man7.org/linux/man-pages/man2/mkdir.2.html
	10. https://stackoverflow.com/questions/7430248/creating-a-new-directory-in-c
	-> I used these resources to understand best practice for creating directories.

	--------------------------------------------------
*/

// Libraries
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/types.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_logger.h"
#include "pwc_config.h"

// Module Name
#define PWC_MODULE_NAME "LOGGER"

// Log File Path
static char logFilePath[256];

// Initialized Flag
static bool isLogFileInitialized = false;

void pwc_log(enum pwc_loggerLevel level, const char* module, const char* message, ...) {

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

	// Get variadic arguments and make copy
	va_list args, argsCopyForPrinting;
	va_start(args, message);
	va_copy(argsCopyForPrinting, args);

	// If log file not initialized, print and exit
	if (!isLogFileInitialized) {
		pwc_printWithPrefix(message, argsCopyForPrinting);
		va_end(args);
		va_end(argsCopyForPrinting);
		return;
	}

	// Open file for appending
	FILE* logFile = fopen(logFilePath, "a");
	if (!logFile) {
		return;
	}

	// Write log entry to file
	fprintf(logFile, "[%s] [%s] [%s]: ", dateTimeString, logLevelString, module);
	vfprintf(logFile, message, args);
	fputc('\n', logFile);
	fclose(logFile);

	// For info, warn, and error levels, also print to stdout using util functions (seperate prefix formatting)
	if (level == PWC_LOGLEVEL_INFO) {
		pwc_printWithPrefix(message, argsCopyForPrinting);
	} else if (level == PWC_LOGLEVEL_WARNING) {
		pwc_warnWithPrefix(message, argsCopyForPrinting);
	} else if (level == PWC_LOGLEVEL_ERROR) {
		pwc_errorWithPrefix(message, argsCopyForPrinting);
	}

	// Exit
	va_end(args);
	va_end(argsCopyForPrinting);
	return;
}

int pwc_initLogFile(pid_t processPID) {
	
	// Get config
	struct pwc_configuration* config = pwc_configuration();

	// Make file name string for static referencing
	snprintf(logFilePath, sizeof(logFilePath), "%s/pwc_%d.log", config->LOGGING_DIRECTORY, processPID);

	// Make sure directory exists for log files
	if (mkdir(config->LOGGING_DIRECTORY, 0755) == -1) {

		// If directory exists, continue, otherwise error out
        if (errno != EEXIST) {
            return 1;
        }
		
    }

	// Create file to make sure it can exist
	FILE* logFile = fopen(logFilePath, "w");
	if (!logFile) { 
		return 1; 
	}
	fclose(logFile);
	isLogFileInitialized = true;
	return 0;
}