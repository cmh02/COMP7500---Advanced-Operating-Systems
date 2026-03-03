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

static const char *AUBATCH_MESSAGE_JOBSUBMISSION = 
	"Job %s was submitted.\n"
	"Total number of jobs in the queue: %u.\n"
	"Expected waiting time: %u seconds.\n"
	"Scheduling Policy: %s.\n";

/*
	# Maximum Command Length in Characters
*/
#define AUBATCH_MAX_COMMAND_LENGTH_CHAR 2048

/*
	# Maximum Command Length in Arguments
*/
#define AUBATCH_MAX_COMMAND_LENGTH_ARGS 5

int aubatch_cmdparser_enterCommandLoop() {

	// Print intro message
	aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_INTRO);

	// Continue looping until user quits
	while (true) {

		// Get user input
		char input[AUBATCH_MAX_COMMAND_LENGTH_CHAR];
		aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "> ");
		if (fgets(input, sizeof(input), stdin) == NULL) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error reading input.");
			continue;
		}

		// Trim/clean input
		char *command = aubatch_trimWhitespace(input);

		// Split command into parts (by spaces)
		char *args[AUBATCH_MAX_COMMAND_LENGTH_ARGS];
		int argCount = 0;
		char *token = strtok(command, " ");
		while (token != NULL && argCount < AUBATCH_MAX_COMMAND_LENGTH_ARGS) {
			args[argCount++] = token;
			token = strtok(NULL, " ");
		}

		// Help message
		if (strcmp(args[0], "help") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_HELP);
		
		// Set policy: FCFS
		} else if (strcmp(args[0], "fcfs") == 0) {
			aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_FCFS);
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.", aubatch_schedulingPolicyNames[aubatch_scheduler_getSchedulingPolicy()], 0);

		// Set policy: SJF
		} else if (strcmp(args[0], "sjf") == 0) {
			aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_SJF);
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.", aubatch_schedulingPolicyNames[aubatch_scheduler_getSchedulingPolicy()], 0);

		// Set policy: Priority
		} else if (strcmp(args[0], "priority") == 0) {
			aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_PRIORITY);
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.", aubatch_schedulingPolicyNames[aubatch_scheduler_getSchedulingPolicy()], 0);

		// Insert job
		} else if (strcmp(args[0], "run") == 0) {

			// Take name as second argument
			char* jobName = args[1];
			if (jobName == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Please provide a name when submitting a new job!");
				continue;
			}

			// Take execution time as third argument
			char* executionTimeRaw = args[2];
			if (executionTimeRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Please provide an execution time when submitting a new job!");
				continue;
			}
			uint32_t executionTime;
			aubatch_parseUnsignedLong(executionTimeRaw, &executionTime);

			// Make job and insert
			struct aubatch_job job = aubatch_jobs_createNewJob(jobName[0], executionTime, 1);
			aubatch_scheduler_insert(job);

			// Print submission message
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, AUBATCH_MESSAGE_JOBSUBMISSION, job.name, aubatch_scheduler_getCurrentQueueSize(), aubatch_scheduler_getCurrentWaitTime(), aubatch_schedulingPolicyNames[aubatch_scheduler_getSchedulingPolicy()]);

		// Quit command
		} else if (strcmp(args[0], "quit") == 0) {
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Exiting AUbatch. Goodbye!");
			break;

		// Handle unknown commands
		} else {
			aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Unknown command: %s", command);
		}
	}
	return 0;
}