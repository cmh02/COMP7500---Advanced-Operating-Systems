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
	
	1. https://pubs.opengroup.org/onlinepubs/009695299/functions/access.html
	-> I used this reference for syntax on checking execution permissions.

	--------------------------------------------------
*/

// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Project Libraries
#include "aubatch.h"
#include "aubatch_jobs.h"
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_scheduler.h"
#include "aubatch_cmdparser.h"
#include "aubatch_benchmarker.h"

// Module Name
#define AUBATCH_MODULE_NAME "COMMANDPARSER"

/*
	# Maximum Command Length in Characters
*/
#define AUBATCH_MAX_COMMAND_LENGTH_CHAR 2048

/*
	# Maximum Command Length in Arguments
*/
#define AUBATCH_MAX_COMMAND_LENGTH_ARGS 10

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
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.\n", aubatch_scheduler_getSchedulingPolicyName(), 0);

		// Set policy: SJF
		} else if (strcmp(args[0], "sjf") == 0) {
			aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_SJF);
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.\n", aubatch_scheduler_getSchedulingPolicyName(), 0);

		// Set policy: Priority
		} else if (strcmp(args[0], "priority") == 0) {
			aubatch_scheduler_setSchedulingPolicy(AUBATCH_SCHEDULINGPOLICY_PRIORITY);
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Scheduling policy is switched to %s. All the %u waiting jobs have been rescheduled.\n", aubatch_scheduler_getSchedulingPolicyName(), 0);

		// Insert job
		} else if (strcmp(args[0], "run") == 0) {

			// Take name as second argument
			char* jobName = args[1];
			if (jobName == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a name when submitting a new job!\n");
				continue;
			}
			else if (strlen(jobName) >= AUBATCH_MAX_JOB_NAME_LENGTH) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Job name cannot be longer than %d characters!\n", AUBATCH_MAX_JOB_NAME_LENGTH - 1);
				continue;
			}

			// Make sure that the job is an actual executable
			if (access(jobName, X_OK) != 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Job '%s' is not an executable job!\n", jobName);
				continue;
			}

			// Take execution time as third argument
			char* executionTimeRaw = args[2];
			if (executionTimeRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide an execution time when submitting a new job!\n");
				continue;
			}
			uint32_t executionTime;
			aubatch_parseUnsignedInt32(executionTimeRaw, &executionTime);

			// Verify execution time is positive
			if (executionTime <= 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Execution time must be a positive integer!\n");
				continue;
			}

			// Take priority as fourth argument
			char* priorityRaw = args[3];
			if (priorityRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a priority when submitting a new job!\n");
				continue;
			}
			uint32_t priority;
			aubatch_parseUnsignedInt32(priorityRaw, &priority);

			// Verify priority is positive
			if (priority <= 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Priority must be a positive integer!\n");
				continue;
			}

			// Make job and insert
			struct aubatch_job job = aubatch_jobs_createNewJob(jobName, executionTime, priority);
			aubatch_scheduler_insert(job);

			// Print submission message
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, AUBATCH_MESSAGE_JOBSUBMISSION, job.name, aubatch_scheduler_getCurrentQueueSize(), aubatch_scheduler_getCurrentWaitTime() - job.time_requestedExecution, aubatch_scheduler_getSchedulingPolicyName());

		// Automated Performance Evaluation
		} else if (strcmp(args[0], "test") == 0) {
			
			// Take name as second argument (both benchmark & job name)
			char* benchmarkName = args[1];
			if (benchmarkName == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a benchmark name when submitting a new test!\n");
				continue;
			}
			else if (strlen(benchmarkName) >= AUBATCH_MAX_JOB_NAME_LENGTH) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Benchmark name cannot be longer than %d characters!\n", AUBATCH_MAX_JOB_NAME_LENGTH - 1);
				continue;
			}

			// Make sure that the benchmark is an actual executable
			if (access(benchmarkName, X_OK) != 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Benchmark '%s' is not an executable job!\n", benchmarkName);
				continue;
			}

			// Take policy as third argument
			char* policyRaw = args[2];
			if (policyRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a scheduling policy when submitting a new benchmark!\n");
				continue;
			}
			enum aubatch_schedulingPolicy policy;
			if (strcmp(policyRaw, "fcfs") == 0) {
				policy = AUBATCH_SCHEDULINGPOLICY_FCFS;
			} else if (strcmp(policyRaw, "sjf") == 0) {
				policy = AUBATCH_SCHEDULINGPOLICY_SJF;
			} else if (strcmp(policyRaw, "priority") == 0) {
				policy = AUBATCH_SCHEDULINGPOLICY_PRIORITY;
			} else {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Invalid scheduling policy '%s' provided for benchmark! Options are 'fcfs', 'sjf', or 'priority'!\n", policyRaw);
				continue;
			}

			// Take number of jobs as fourth argument
			char* numOfJobsRaw = args[3];
			if (numOfJobsRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a number of jobs when submitting a new benchmark!\n");
				continue;
			}
			uint32_t numOfJobs;
			aubatch_parseUnsignedInt32(numOfJobsRaw, &numOfJobs);

			// Verify number of jobs is positive
			if (numOfJobs <= 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Number of jobs must be a positive integer!\n");
				continue;
			}

			// Take arrival rate as fifth argument
			char* arrivalRateRaw = args[4];
			if (arrivalRateRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide an arrival rate when submitting a new benchmark!\n");
				continue;
			}
			uint32_t arrivalRate;
			aubatch_parseUnsignedInt32(arrivalRateRaw, &arrivalRate);

			// Verify arrival rate is positive
			if (arrivalRate <= 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Arrival rate must be a positive integer!\n");
				continue;
			}

			// Take number of priority levels as sixth argument
			char* priorityLevelsRaw = args[5];
			if (priorityLevelsRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a number of priority levels when submitting a new benchmark!\n");
				continue;
			}
			uint32_t priorityLevels;

			// Verify priority levels is positive
			aubatch_parseUnsignedInt32(priorityLevelsRaw, &priorityLevels);
			if (priorityLevels <= 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Number of priority levels must be a positive integer!\n");
				continue;
			}

			// Take minimum CPU time as seventh argument
			char* minCPUTimeRaw = args[6];
			if (minCPUTimeRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a minimum CPU time when submitting a new benchmark!\n");
				continue;
			}
			uint32_t minCPUTime;
			aubatch_parseUnsignedInt32(minCPUTimeRaw, &minCPUTime);

			// Verify minimum CPU time is positive or 0
			if (minCPUTime < 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Minimum CPU time must be a non-negative integer!\n");
				continue;
			}

			// Take maximum CPU time as eighth argument
			char* maxCPUTimeRaw = args[7];
			if (maxCPUTimeRaw == NULL) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Please provide a maximum CPU time when submitting a new benchmark!\n");
				continue;
			}
			uint32_t maxCPUTime;
			aubatch_parseUnsignedInt32(maxCPUTimeRaw, &maxCPUTime);

			// Verify maximum CPU time is positive or 0
			if (maxCPUTime < 0) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Maximum CPU time must be a non-negative integer!\n");
				continue;
			}

			// Verify that maximum CPU time is greater than or equal to minimum CPU time
			if (maxCPUTime < minCPUTime) {
				aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Maximum CPU time must be greater than or equal to minimum CPU time!\n");
				continue;
			}

			// Hand off to benchmarker
			struct aubatch_benchmarkConfiguration benchmarkConfig = {
				.name = "",
				.policy = policy,
				.numberOfJobs = numOfJobs,
				.arrivalRate = arrivalRate,
				.numberOfPrioritylevels = priorityLevels,
				.minCPUTime = minCPUTime,
				.maxCPUTime = maxCPUTime
			};
			strncpy(benchmarkConfig.name, benchmarkName, AUBATCH_MAX_JOB_NAME_LENGTH);
			aubatch_benchmarker_runBenchmark(benchmarkConfig);

		// List jobs
		} else if (strcmp(args[0], "list") == 0) {

			// Make call to scheduler to print job queue into interactive channel
			aubatch_scheduler_printJobQueue(AUBATCH_LOGLEVEL_INTERACTIVE);

		// Quit command
		} else if (strcmp(args[0], "quit") == 0) {

			// Flip running flag
			aubatch_flag_programRunning = 0;

			// Send final stats
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, AUBATCH_MESSAGE_EXIT, aubatch_scheduler_getCurrentTotalSeenJobs(), aubatch_scheduler_getCurrentAverageTurnaroundTime(), aubatch_scheduler_getCurrentAverageWaitTime(), aubatch_scheduler_getCurrentAverageWaitTime(), aubatch_scheduler_getCurrentThroughput());
			break;

		// Handle unknown commands
		} else {
			aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Unknown command: %s\n", command);
			continue;
		}
	}
	return 0;
}