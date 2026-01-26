/*
	--------------------------------------------------

	# Information

	File: pwc_config.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the configuration module for pWordCount.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/


// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Project Libraries
#include "pwc_config.h"
#include "pwc_utils.h"
#include "pwc_logger.h"

// Module Name
#define PWC_MODULE_NAME "CONFIGURATION"

// Program-Wide Configuration
static struct pwc_configuration globalConfiguration;

// Configuration Functions

struct pwc_configuration *pwc_configuration(void) {
	return &globalConfiguration;
}

void pwc_populateDefaultConfiguration(struct pwc_configuration *config) {

	// Set default configuration values
	config->LOGGING_SEND_DEBUG_TO_LOG = true;
	config->LOGGING_SEND_DEBUG_TO_STDOUT = true;
	config->NUMBER_OF_PROCESSES = 1;
	config->BUFFER_SIZE_READER = 4096;
	config->BUFFER_SIZE_COUNTERMANAGER = 4096;
	config->BUFFER_SIZE_COUNTER = 4096;
	config->TEXT_FILE_PATH = NULL;
	config->CONFIG_FILE_PATH = "../config/pwordcount.config";
	config->LOGGING_DIRECTORY = "../logs/";

}

void pwc_populateNullConfiguration(struct pwc_configuration *config) {

	// Set all configuration values to null/zero
	config->LOGGING_SEND_DEBUG_TO_LOG = false;
	config->LOGGING_SEND_DEBUG_TO_STDOUT = false;
	config->NUMBER_OF_PROCESSES = PWC_UNSET_UNSIGNED_LONG;
	config->BUFFER_SIZE_READER = PWC_UNSET_UNSIGNED_LONG;
	config->BUFFER_SIZE_COUNTERMANAGER = PWC_UNSET_UNSIGNED_LONG;
	config->BUFFER_SIZE_COUNTER = PWC_UNSET_UNSIGNED_LONG;
	config->TEXT_FILE_PATH = NULL;
	config->CONFIG_FILE_PATH = NULL;
	config->LOGGING_DIRECTORY = NULL;

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
	int lineCounter = 0;
	bool withinComment = false;
	while ((lineContentsReadLength = getline(&lineContents, &lineContentsBufferLength, configFile)) != -1) {
		lineCounter++;

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
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid non-comment line %d in configuration file, missing '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}

		// Split at the seperator
		*seperator = '\0';
		char *key = pwc_trimWhitespace(lineContentsTrimmed);
		char *value = pwc_trimWhitespace(seperator + 1);

		// Validate that key and value are actually present
		if (strlen(key) == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line %d, missing key before '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strlen(value) == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line %d, missing value after '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strcmp(key, "\0") == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line %d, empty key before '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strcmp(value, "\0") == 0) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Skipping invalid configuration line %d, empty value after '=': %s!", lineCounter, lineContentsTrimmed);
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
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid boolean value for LOGGING_SEND_DEBUG_TO_STDOUT: %s, using default value of '%s'!", value, config->LOGGING_SEND_DEBUG_TO_STDOUT ? "true" : "false");
			}
		}
		else if (!strcmp(key, "LOGGING_DIRECTORY")) {
			config->LOGGING_DIRECTORY = &value[0];
		}
		else if (!strcmp(key, "NUMBER_OF_PROCESSES")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->NUMBER_OF_PROCESSES = parsedValue;
			} else {
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for NUMBER_OF_PROCESSES: %s, using default value of '%lu'!", value, config->NUMBER_OF_PROCESSES);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_READER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_READER = parsedValue;
			} else {
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for BUFFER_SIZE_READER: %s, using default value of '%lu'!", value, config->BUFFER_SIZE_READER);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_COUNTERMANAGER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_COUNTERMANAGER = parsedValue;
			} else {
				pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "Invalid unsigned long value for BUFFER_SIZE_COUNTERMANAGER: %s, using default value of '%lu'!", value, config->BUFFER_SIZE_COUNTERMANAGER);
			}
		}
		else if (!strcmp(key, "BUFFER_SIZE_COUNTER")) {
			unsigned long parsedValue;
			if (pwc_parseUnsignedLong(value, &parsedValue)) {
				config->BUFFER_SIZE_COUNTER = parsedValue;
			} else {
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