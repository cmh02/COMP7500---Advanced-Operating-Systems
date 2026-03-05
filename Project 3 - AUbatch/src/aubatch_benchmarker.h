/*
	--------------------------------------------------

	# Information

	File: aubatch_benchmarker.h
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

// Prevent Multiple Inclusions
#ifndef AUBATCH_BENCHMARKER_H
#define AUBATCH_BENCHMARKER_H

// Project Libraries
#include "aubatch.h"
#include "aubatch_jobs.h"
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_scheduler.h"

struct aubatch_benchmarkConfiguration {
	char name[AUBATCH_MAX_JOB_NAME_LENGTH];
	enum aubatch_schedulingPolicy policy;
	uint32_t numberOfJobs;
	uint32_t numberOfPrioritylevels;
	double arrivalRate;
	double minCPUTime;
	double maxCPUTime;
};

/*
	# Run Benchmark

	This function will run a benchmark with the provided configuration. 
	It will block until the benchmark finishes as it is not expected to be run
	while using the program for other purposes.

	## Parameters
	- struct aubatch_benchmarkConfiguration config: the configuration for the benchmark to be run

	## Returns
	- int: 0 on success, 1 on any failure
*/
int aubatch_benchmarker_runBenchmark(struct aubatch_benchmarkConfiguration config);

/*
	# Write Benchmark to CSV

	This function will write the information about a benchmark and its results to a CSV file.
	This is just hard-coded for use with the workload testing, since parsing it out would have
	been a hassle.

	## Parameters
	- struct aubatch_benchmarkConfiguration config: the configuration for the benchmark that was run
	- uint32_t totalSeenJobs: the total number of jobs that were seen by the scheduler during the benchmark
	- double averageTurnaroundTime: the average turnaround time of jobs during the benchmark
	- double averageCPUTime: the average CPU time of jobs during the benchmark
	- double averageWaitTime: the average wait time of jobs during the benchmark
	- double throughput: the throughput of the scheduler during the benchmark

	## Returns
	- void
*/
void aubatch_benchmarker_writeBenchmarkToCSV(struct aubatch_benchmarkConfiguration config, uint32_t totalSeenJobs, double averageTurnaroundTime, double averageCPUTime, double averageWaitTime, double throughput);

#endif // AUBATCH_BENCHMARKER_H