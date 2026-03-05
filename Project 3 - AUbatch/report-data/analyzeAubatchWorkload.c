/*
	--------------------------------------------------

	# Information

	File: analyzeAubatchWorkload.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file will test the AUbatch tool under various workload
	conditions from a completely external (system-level) perspective.

	Results will be outputted into a CSV alongside this program from the
	benchmarker module. Note that this is not a real-world implementation,
	but serves the purpose for the scope of this project.

	--------------------------------------------------

*/

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Main Execution
int main() {

	// Define the target job to use in these benchmarks
	char* targetJobName = "../jobs/job_randsleep5";

	// Define the options for number of submitted jobs
	uint32_t options_numSubmittedJobs[] = {5, 10, 25};
	uint32_t numOptions_numSubmittedJobs = sizeof(options_numSubmittedJobs) / sizeof(options_numSubmittedJobs[0]);

	// Define the options for arrival rate
	double options_arrivalRate[] = {0.1, 0.5, 1.0};
	double numOptions_arrivalRate = sizeof(options_arrivalRate) / sizeof(options_arrivalRate[0]);

	// Define the options for priority levels
	uint32_t options_priorityLevels[] = {1, 3, 5};
	uint32_t numOptions_priorityLevels = sizeof(options_priorityLevels) / sizeof(options_priorityLevels[0]);

	// Define the options for load distribution
	double options_loadDistribution[][2] = {
		{0.1, 0.5},
		{0.5, 1},
		{1, 10}
	};
	double numOptions_loadDistribution = sizeof(options_loadDistribution) / sizeof(options_loadDistribution[0]);

	// Loop through all combinations
	 

}