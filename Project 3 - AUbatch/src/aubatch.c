/*
	--------------------------------------------------

	# Information

	File: aubatch.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file provides the main execution for AUbatch.

	--------------------------------------------------

	# References



	--------------------------------------------------
*/

// Libraries

// Project Libraries
#include "aubatch.h"


// main
int main(int argc, char* argv[]) {

	// Get default config struct
	struct aubatch_configuration* config = aubatch_configuration();
	aubatch_populateDefaultConfiguration(config);

	// Load configuration file from path (hard-coded for now)
	aubatch_loadConfigurationFile("../config/aubatch.config", config);

	// Initialize logger with config path
	aubatch_initLogFile(getpid());

	// Launch commmand loop
	aubatch_cmdparser_enterCommandLoop();
	
}