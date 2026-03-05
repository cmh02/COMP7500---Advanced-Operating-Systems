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

	# References

	1. https://stackoverflow.com/questions/34971220/echo-1-50-2-than-pipe-into-another-program-in-c
	-> I used this post to look at running aubatch from here.

*/

// Libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Main Execution
int main() {

	// Define the target job to use in these benchmarks
	char* targetJobName = "../jobs/job_randsleep5";

	// Define the options for scheduling policy
	char* options_schedulingPolicy[] = {"fcfs", "sjf", "priority"};
	uint32_t numOptions_schedulingPolicy = sizeof(options_schedulingPolicy) / sizeof(options_schedulingPolicy[0]);

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
	for (int iJobs = 0; iJobs < numOptions_numSubmittedJobs; iJobs++) {
		for (int iArrivalRate = 0; iArrivalRate < numOptions_arrivalRate; iArrivalRate++) {
			for (int iPriorityLevels = 0; iPriorityLevels < numOptions_priorityLevels; iPriorityLevels++) {
				for (int iLoadDistribution = 0; iLoadDistribution < numOptions_loadDistribution; iLoadDistribution++) {
					for (int iSchedulingPolicy = 0; iSchedulingPolicy < numOptions_schedulingPolicy; iSchedulingPolicy++) {

						// Print current combo being run
						printf("Running benchmark with scheduling policy '%s', number of jobs %u, arrival rate %f, number of priority levels %u, minimum CPU time %f, and maximum CPU time %f!\n", options_schedulingPolicy[iSchedulingPolicy], options_numSubmittedJobs[iJobs], options_arrivalRate[iArrivalRate], options_priorityLevels[iPriorityLevels], options_loadDistribution[iLoadDistribution][0], options_loadDistribution[iLoadDistribution][1]);

						// Build command for running aubatch
						char command[2056];

						// Copy in values
						snprintf(command, sizeof(command), "echo \"test %s %s %d %f %d %f %f\nquit\n\" | ../build/aubatch",
							targetJobName,
							options_schedulingPolicy[iSchedulingPolicy],
							options_numSubmittedJobs[iJobs],
							options_arrivalRate[iArrivalRate],
							options_priorityLevels[iPriorityLevels],
							options_loadDistribution[iLoadDistribution][0],
							options_loadDistribution[iLoadDistribution][1]
						);

						// Run command
						system(command);

					}
				}
			}
		}
	}

}