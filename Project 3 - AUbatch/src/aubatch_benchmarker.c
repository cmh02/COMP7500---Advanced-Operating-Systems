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
	
	1.https://stackoverflow.com/questions/13566082/how-to-check-if-a-file-has-content-or-not-using-c
	-> I used this post to remember how to check for empty file.

	--------------------------------------------------
*/

// Libraries
#include <stdio.h>
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
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Starting benchmark '%s' with scheduling policy '%s', number of jobs %u, arrival rate %u, number of priority levels %u, minimum CPU time %u, and maximum CPU time %u!", config.name, aubatch_scheduler_getSchedulingPolicyName(), config.numberOfJobs, config.arrivalRate, config.numberOfPrioritylevels, config.minCPUTime, config.maxCPUTime);

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
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Sending off job %u with ID %u!", i, job.id);
		aubatch_scheduler_insert(job);

		// Sleep for arrival rate
		sleep(config.arrivalRate);
	}
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "All jobs for benchmark '%s' have been submitted!", config.name);

	// Wait for all jobs to finish
	int benchmarkFinishes = aubatch_scheduler_waitForNJobsToFinish(config.numberOfJobs);
	if (benchmarkFinishes != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Benchmark '%s' failed to finish properly !", config.name);
		return 1;
	}

	// Log benchmark results
	aubatch_scheduler_printJobQueue(AUBATCH_LOGLEVEL_INFO);
	uint32_t totalSeenJobs = aubatch_scheduler_getCurrentTotalSeenJobs();
	double averageTurnaroundTime = aubatch_scheduler_getCurrentAverageTurnaroundTime();
	double averageCPUTime = aubatch_scheduler_getCurrentAverageCPUTime();
	double averageWaitTime = aubatch_scheduler_getCurrentAverageWaitTime();
	double throughput = aubatch_scheduler_getCurrentThroughput();
	aubatch_log(AUBATCH_LOGLEVEL_INTERACTIVE, AUBATCH_MODULE_NAME, AUBATCH_MESSAGE_EXIT, totalSeenJobs, averageTurnaroundTime, averageCPUTime, averageWaitTime, throughput);
	aubatch_benchmark_writeBenchmarkToCSV(config, totalSeenJobs, averageTurnaroundTime, averageCPUTime, averageWaitTime, throughput);
	return 0;
}

void aubatch_benchmarker_writeBenchmarkToCSV(struct aubatch_benchmarkConfiguration config, uint32_t totalSeenJobs, double averageTurnaroundTime, double averageCPUTime, double averageWaitTime, double throughput) {

	// Open file for appending (fopen creates if it doesnt exist)
	FILE *csv = fopen("../report-data/aubatch_benchmarks.csv", "a");
	if (!csv) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to open benchmark CSV file for writing!");
		return;
	}

	// Write header if file is new
	fseek(csv, 0, SEEK_END);
	long csvSize = ftell(csv);
	if (csvSize == 0) {
		fprintf(csv, "Name,Scheduling Policy,Number of Jobs,Arrival Rate,Number of Priority Levels,Min CPU Time,Max CPU Time,Total Seen Jobs,Average Turnaround Time,Average CPU Time,Average Wait Time,Throughput\n");
	}

	// Write benchmark data and close
	fprintf(csv, "%s,%s,%u,%u,%u,%u,%u,%u,%f,%f,%f,%f\n", config.name, aubatch_scheduler_getSchedulingPolicyName(), config.numberOfJobs, config.arrivalRate, config.numberOfPrioritylevels, config.minCPUTime, config.maxCPUTime, totalSeenJobs, averageTurnaroundTime, averageCPUTime, averageWaitTime, throughput);
	fclose(csv);
}

// Module Name
#define AUBATCH_MODULE_NAME "BENCHMARKER"