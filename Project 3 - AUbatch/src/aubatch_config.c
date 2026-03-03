/*
	--------------------------------------------------

	# Information

	File: aubatch_config.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the configuration module for AUbatch.

	--------------------------------------------------

	## References
	
	1. https://stackoverflow.com/questions/9449241/where-is-path-max-defined-in-linux
	2. https://insanecoding.blogspot.com/2007/11/pathmax-simply-isnt.html
	-> I used these resources to understand best practices for PATH_MAX.

	--------------------------------------------------
*/


// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Project Libraries
#include "aubatch_config.h"
#include "aubatch_logger.h"

// Module Name
#define AUBATCH_MODULE_NAME "CONFIGURATION"

// Program-Wide Configuration
static struct aubatch_configuration globalConfiguration;

// Configuration Functions

struct aubatch_configuration *aubatch_configuration(void) {
	return &globalConfiguration;
}

void aubatch_populateDefaultConfiguration(struct aubatch_configuration *config) {

	// Set default configuration values
	config->LOGGING_SEND_DEBUG_TO_LOG = true;
	config->LOGGING_SEND_DEBUG_TO_STDOUT = true;
	strncpy(config->CONFIG_FILE_PATH, "../config/aubatch.config", PATH_MAX - 1);
	config->CONFIG_FILE_PATH[PATH_MAX - 1] = '\0';
	strncpy(config->LOGGING_DIRECTORY, "../logs/", PATH_MAX - 1);
	config->LOGGING_DIRECTORY[PATH_MAX - 1] = '\0';

}

void aubatch_populateNullConfiguration(struct aubatch_configuration *config) {

	// Set all configuration values to null/zero
	config->LOGGING_SEND_DEBUG_TO_LOG = false;
	config->LOGGING_SEND_DEBUG_TO_STDOUT = false;
	config->CONFIG_FILE_PATH[0] = '\0';
	config->LOGGING_DIRECTORY[0] = '\0';

}

int aubatch_loadConfigurationFile(const char* filePath, struct aubatch_configuration* config) {

	// Open config file and detect any errors
	FILE* configFile = fopen(filePath, "r");
	if (configFile == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "The attempt to open the configuration file at path '%s' has failed!", filePath);
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

		// Check if line is now empty, if so skip
		if (strlen(lineContentsTrimmed) == 0) { continue; }

		// Find the key and value by splitting at =
		char *seperator = strchr(lineContentsTrimmed, '=');

		// Check that we found a valid seperator, if not, skip and warn
		if (!seperator) {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Skipping invalid non-comment line %d in configuration file, missing '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}

		// Split at the seperator
		*seperator = '\0';
		char *key = pwc_trimWhitespace(lineContentsTrimmed);
		char *value = pwc_trimWhitespace(seperator + 1);

		// Validate that key and value are actually present
		if (strlen(key) == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Skipping invalid configuration line %d, missing key before '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strlen(value) == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Skipping invalid configuration line %d, missing value after '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strcmp(key, "\0") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Skipping invalid configuration line %d, empty key before '=': %s!", lineCounter, lineContentsTrimmed);
			continue;
		}
		if (strcmp(value, "\0") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Skipping invalid configuration line %d, empty value after '=': %s!", lineCounter, lineContentsTrimmed);
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
				aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Invalid boolean value for LOGGING_SEND_DEBUG_TO_LOG: %s, using default value of '%s'!", value, config->LOGGING_SEND_DEBUG_TO_LOG ? "true" : "false");
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
				aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Invalid boolean value for LOGGING_SEND_DEBUG_TO_STDOUT: %s, using default value of '%s'!", value, config->LOGGING_SEND_DEBUG_TO_STDOUT ? "true" : "false");
			}
		}
		else if (!strcmp(key, "LOGGING_DIRECTORY")) {
			strncpy(config->LOGGING_DIRECTORY, value, PATH_MAX - 1);
			config->LOGGING_DIRECTORY[PATH_MAX - 1] = '\0';
		}

	}
	
	// Still have to free the buffer even though we don't allocate (per man page)
	free(lineContents);

	// Close config file
	fclose(configFile);

	// Return success
	return 0;

}