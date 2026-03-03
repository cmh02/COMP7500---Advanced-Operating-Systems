/*
	--------------------------------------------------

	# Information

	File: aubatch_cmdparser.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the command parser module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Libraries

// Project Libraries
#include "aubatch_cmdparser.h"

// Pre-Defined Messages
static const char *AUBATCH_MESSAGE_INTRO = 
	"Welcome to " AUBATCH_PROGRAM_AUTHOR "'s batch job scheduler Version " AUBATCH_PROGRAM_VERSION "\n"
	"Type 'help' to find more about AUbatch commands.\n";

static const char *AUBATCH_MESSAGE_HELP = 
	"run <job> <time> <pri>: submit a job named <job>,\n"
	"						 execution time is <time>,\n"
	"						 priority is <pri>.\n"
	"list: display the job status.\n"
	"fcfs: change the scheduling policy to FCFS.\n"
	"sjf: change the scheduling policy to SJF.\n"
	"priority: change the scheduling policy to priority.\n"
	"test <benchmark> <policy> <num_of_jobs> <priority_levels>\n"
	"	  <min_CPU_time> <max_CPU_time>\n"
	"quit: exit AUbatch.\n";

// Max Command Length
#define AUBATCH_MAX_COMMAND_LENGTH 2048

int aubatch_cmdparser_enterCommandLoop() {

	// Print intro message
	aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_INTRO);

	// Continue looping until user quits
	while (true) {

		// Get user input
		char input[AUBATCH_MAX_COMMAND_LENGTH];
		printf("> ");
		if (fgets(input, sizeof(input), stdin) == NULL) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error reading input.");
			continue;
		}

		// Trim/clean input
		char *command = aubatch_trimWhitespace(input);

		// Help message
		if (strcmp(command, "help") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_HELP);

		// Quit command
		} else if (strcmp(command, "quit") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Exiting AUbatch. Goodbye!");
			break;

		// Handle unknown commands
		} else {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Unknown command: %s", command);
		}
	}
	return 0;
}