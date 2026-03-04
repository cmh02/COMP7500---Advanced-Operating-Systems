/*
	--------------------------------------------------

	# Information

	File: aubatch_logger.c
	Project: Project 3 - AUbatch
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
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

// Project Libraries
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_config.h"

// Module Name
#define AUBATCH_MODULE_NAME "LOGGER"

// Log File Path
static char logFilePath[256];

// Initialized Flag
static bool isLogFileInitialized = false;

void aubatch_log(enum aubatch_loggerLevel level, const char* module, const char* message, ...) {

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
		case AUBATCH_LOGLEVEL_INFO:
			logLevelString = "INFO";
			break;
		case AUBATCH_LOGLEVEL_DEBUG:
			logLevelString = "DEBUG";
			break;
		case AUBATCH_LOGLEVEL_WARNING:
			logLevelString = "WARNING";
			break;
		case AUBATCH_LOGLEVEL_ERROR:
			logLevelString = "ERROR";
			break;
		case AUBATCH_LOGLEVEL_INTERACTIVE:
			logLevelString = "INTERACTIVE";
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
		aubatch_printWithPrefix(message, argsCopyForPrinting);
		va_end(args);
		va_end(argsCopyForPrinting);
		return;
	}

	// Get program configuration
	struct aubatch_configuration* config = aubatch_configuration();

	// Open file for appending
	FILE* logFile = fopen(logFilePath, "a");
	if (!logFile) {
		return;
	}

	// For each level, print to log file if configured
	if (((level == AUBATCH_LOGLEVEL_INFO) && (config->LOGGING_SEND_INFO_TO_LOG))
		|| ((level == AUBATCH_LOGLEVEL_WARNING) && (config->LOGGING_SEND_WARNING_TO_LOG))
		|| ((level == AUBATCH_LOGLEVEL_ERROR) && (config->LOGGING_SEND_ERROR_TO_LOG))
		|| ((level == AUBATCH_LOGLEVEL_DEBUG) && (config->LOGGING_SEND_DEBUG_TO_LOG))
		|| ((level == AUBATCH_LOGLEVEL_INTERACTIVE) && (config->LOGGING_SEND_INTERACTIVE_TO_LOG))
	){
		fprintf(logFile, "[%s] [%s] [%s]: ", dateTimeString, logLevelString, module);
		vfprintf(logFile, message, args);
		fputc('\n', logFile);
		fclose(logFile);
	}

	// For each level, also print to stdout if configured
	if ((level == AUBATCH_LOGLEVEL_INFO) && (config->LOGGING_SEND_INFO_TO_STDOUT)) {
		aubatch_printWithPrefix(message, argsCopyForPrinting);
	} else if ((level == AUBATCH_LOGLEVEL_WARNING) && (config->LOGGING_SEND_WARNING_TO_STDOUT)) {
		aubatch_warnWithPrefix(message, argsCopyForPrinting);
	} else if ((level == AUBATCH_LOGLEVEL_ERROR) && (config->LOGGING_SEND_ERROR_TO_STDOUT)) {
		aubatch_errorWithPrefix(message, argsCopyForPrinting);
	} else if ((level == AUBATCH_LOGLEVEL_DEBUG) && (config->LOGGING_SEND_DEBUG_TO_STDOUT)) {
		aubatch_debugWithPrefix(message, argsCopyForPrinting);
	} else if ((level == AUBATCH_LOGLEVEL_INTERACTIVE) && (config->LOGGING_SEND_INTERACTIVE_TO_STDOUT)) {
		aubatch_printWithoutPrefix(message, argsCopyForPrinting);
	}

	// Exit
	va_end(args);
	va_end(argsCopyForPrinting);
	return;
}

int aubatch_initLogFile(pid_t processPID) {
	
	// Get config
	struct aubatch_configuration* config = aubatch_configuration();

	// Make file name string for static referencing
	snprintf(logFilePath, sizeof(logFilePath), "%s/aubatch_%d.log", config->LOGGING_DIRECTORY, processPID);

	// Make sure directory exists for log files
	if (mkdir(config->LOGGING_DIRECTORY, 0755) == -1) {

		// If directory exists, continue, otherwise error out
        if (errno != EEXIST) {
			fprintf(stderr, "Failed to create logging directory at path '%s'!\n", config->LOGGING_DIRECTORY);
            return 1;
        }
		
    }

	// Create file to make sure it can exist
	FILE* logFile = fopen(logFilePath, "w");
	if (!logFile) {
		fprintf(stderr, "Failed to create log file at path '%s'!\n", logFilePath);
		return 1; 
	}
	fclose(logFile);
	isLogFileInitialized = true;
	return 0;
}