/*
	--------------------------------------------------

	# Information

	File: aubatch_config.h
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


// Prevent multiple inclusions
#ifndef AUBATCH_CONFIG_H
#define AUBATCH_CONFIG_H

// Libraries
#include <limits.h>
#include <stdbool.h>

/*
	# Configuration Options Structure

	This structure will define the possible config options to be set from a file.
*/
struct aubatch_configuration {
	bool LOGGING_SEND_DEBUG_TO_LOG;
	bool LOGGING_SEND_DEBUG_TO_STDOUT;
	char CONFIG_FILE_PATH[PATH_MAX];
	char LOGGING_DIRECTORY[PATH_MAX];
};



/*
	# Unset Unsigned Long Value

	To assist with configuration, this will define a value to be used when a unsigned long config is not set.
*/
#define AUBATCH_UNSET_UNSIGNED_LONG (unsigned long)20262002

/*
	# Program Configuration Access

	Provides global access to the configuration struct.

	## Arguments
	- void

	## Returns
	- struct aubatch_configuration* : Pointer to the global configuration struct
*/
struct aubatch_configuration *aubatch_configuration(void);

/*
	# Populate Default Configuration

	Populate a aubatch_configuration struct with the default values.

	## Arguments
	- config : Pointer to a aubatch_configuration struct to populate

	## Returns
	- void
*/
void aubatch_populateDefaultConfiguration(struct aubatch_configuration *config);

/*
	# Populate Null Configuration

	Populate a aubatch_configuration struct with null values.

	## Arguments
	- config : Pointer to a aubatch_configuration struct to populate

	## Returns
	- void
*/
void aubatch_populateNullConfiguration(struct aubatch_configuration *config);

/*
	# Load Configuration File

	Load configuration options from a config file into the given configuration struct.

	## Arguments
	- filePath : The path to the configuration file
	- config : Pointer to a aubatch_configuration struct to load options into

	## Returns
	- int : 0 on success, 1 on failure
*/
int aubatch_loadConfigurationFile(const char* filePath, struct aubatch_configuration* config);

#endif // AUBATCH_CONFIG_H