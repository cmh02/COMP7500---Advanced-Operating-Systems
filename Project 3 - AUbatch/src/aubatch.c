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
#include "aubatch_dispatcher.h"

// Module Name
#define AUBATCH_MODULE_NAME "MAIN"

// Program State
int aubatch_flag_programRunning = 1;

// Main Execution
int main(int argc, char* argv[]) {

	// Get default config struct
	struct aubatch_configuration* config = aubatch_configuration();
	aubatch_populateDefaultConfiguration(config);

	// Load configuration file from path (hard-coded for now)
	int loadConfigurationStatus = aubatch_loadConfigurationFile("../config/aubatch.config", config);
	if (loadConfigurationStatus != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to load configuration file!");
		return 1;
	}

	// Initialize logger with config path
	int logFileInitStatus = aubatch_initLogFile(getpid());
	if (logFileInitStatus != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to initialize log file!");
		return 1;
	}

	// Initialize scheduler with FCFS
	int schedulerSetPolicyStatus = aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_FCFS);
	if (schedulerSetPolicyStatus != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to set scheduler policy!");
		return 1;
	}

	// Start up dispatcher
	int dispatcherStartStatus = aubatch_dispatcher_start();
	if (dispatcherStartStatus != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to start dispatcher!");
		return 1;
	}

	// Launch commmand loop
	aubatch_cmdparser_enterCommandLoop();
	
}