/*
	--------------------------------------------------

	# Information

	File: pwc_config.h
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


// Prevent multiple inclusions
#ifndef PWC_CONFIG_H
#define PWC_CONFIG_H

// Libraries
#include <stdbool.h>


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
	const char *TEXT_FILE_PATH;
	const char *CONFIG_FILE_PATH;
	const char *LOGGING_DIRECTORY;
};



/*
	# Unset Unsigned Long Value

	To assist with configuration, this will define a value to be used when a unsigned long config is not set.
*/
#define PWC_UNSET_UNSIGNED_LONG (unsigned long)20262002

/*
	# Program Configuration Access

	Provides global access to the configuration struct.

	# Arguments
	- void

	# Returns
	- struct pwc_configuration* : Pointer to the global configuration struct
*/
struct pwc_configuration *pwc_configuration(void);

/*
	# Populate Default Configuration

	Populate a pwc_configuration struct with the default values.

	## Arguments
	- config : Pointer to a pwc_configuration struct to populate

	## Returns
	- void
*/
void pwc_populateDefaultConfiguration(struct pwc_configuration *config);

/*
	# Populate Null Configuration

	Populate a pwc_configuration struct with null values.

	## Arguments
	- config : Pointer to a pwc_configuration struct to populate

	## Returns
	- void
*/
void pwc_populateNullConfiguration(struct pwc_configuration *config);

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

#endif // PWC_CONFIG_H