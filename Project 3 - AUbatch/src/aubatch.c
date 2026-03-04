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
#include <libc.h>

// Project Libraries
#include "aubatch.h"
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_config.h"
#include "aubatch_scheduler.h"
#include "aubatch_cmdparser.h"

// Module Name
#define AUBATCH_MODULE_NAME "MAIN"

// Main Execution
int main(int argc, char* argv[]) {

	// Get default config struct
	struct aubatch_configuration* config = aubatch_configuration();
	aubatch_populateDefaultConfiguration(config);

	// Load configuration file from path (hard-coded for now)
	aubatch_loadConfigurationFile("../config/aubatch.config", config);

	// Initialize logger with config path
	aubatch_initLogFile(getpid());

	// Set program state to running
	aubatch_programRunning = 1;

	// Initialize scheduler with FCFS
	aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_FCFS);

	// Launch commmand loop
	aubatch_cmdparser_enterCommandLoop();
	
}