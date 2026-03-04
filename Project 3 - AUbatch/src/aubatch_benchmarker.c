/*
	--------------------------------------------------

	# Information

	File: aubatch_benchmarker.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the benchmarking module for AUbatch.

	--------------------------------------------------

	## References
	
	1.

	--------------------------------------------------
*/

// Libraries
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

// Project Libraries
#include "aubatch.h"
#include "aubatch_jobs.h"
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_scheduler.h"
#include "aubatch_cmdparser.h"
#include "aubatch_benchmarker.h"

// Module Name
#define AUBATCH_MODULE_NAME "BENCHMARKER"

int aubatch_benchmarker_runBenchmark(struct aubatch_benchmarkConfiguration config) {

	// Log benchmark start
	aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, "Starting benchmark '%s' with scheduling policy '%s', number of jobs %u, arrival rate %u, number of priority levels %u, minimum CPU time %u, and maximum CPU time %u!", config.name, aubatch_scheduler_getSchedulingPolicyName(config.policy), config.numberOfJobs, config.arrivalRate, config.numberOfPrioritylevels, config.minCPUTime, config.maxCPUTime);

	// Set scheduling policy for benchmark
	aubatch_scheduler_setSchedulingPolicy(config.policy);

	// Seed rng for benchmark
	srand(time(NULL));

	// Generate jobs and submit them according to specification
	for (int i = 0; i < config.numberOfJobs; i++) {

		// Generate priority level (between 1 and number of priority levels, inclusive)
		uint32_t jobPriority = (rand() % config.numberOfPrioritylevels) + 1;

		// Generate CPU time (between min and max CPU time, inclusive)
		uint32_t jobCPUTime = (rand() % (config.maxCPUTime - config.minCPUTime + 1)) + config.minCPUTime;

		// Make job and insert
		struct aubatch_job job = aubatch_jobs_createNewJob(config.name, jobCPUTime, jobPriority);
		aubatch_scheduler_insert(job);

		// Sleep for arrival rate
		sleep(config.arrivalRate);
	}

	// Wait for all jobs to finish
	int benchmarkFinishes = aubatch_scheduler_waitForNJobsToFinish(config.numberOfJobs);
	if (benchmarkFinishes != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Benchmark '%s' failed to finish properly !", config.name);
		return 1;
	}

	// Log benchmark results
	aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, AUBATCH_MESSAGE_EXIT, aubatch_scheduler_getCurrentTotalSeenJobs(), aubatch_scheduler_getCurrentAverageTurnaroundTime(), aubatch_scheduler_getCurrentAverageWaitTime(), aubatch_scheduler_getCurrentAverageWaitTime(), aubatch_scheduler_getCurrentThroughput());
	return 0;
}

// Module Name
#define AUBATCH_MODULE_NAME "BENCHMARKER"