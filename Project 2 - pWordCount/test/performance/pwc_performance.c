/*
	--------------------------------------------------

	# Information

	File: pwc_performance.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This program will seek to measure performance of pWordCount on the variety of test cases possible.
	Note that these tests are being performed purely at the system level, to capture the overal memory
	consumption and time for execution, and do not seek to validate every edge case. This responsibility
	is being left with the unit testing portion of this project.

	Three main goals will be measured here:
	- Execution time for each test file at different process counts.
	- Memory consumption for each test file at different process counts.
	- The above two goals at different buffer sizes for each module.

	--------------------------------------------------

	## References
	
	1. https://stackoverflow.com/questions/1271064/how-do-i-loop-through-all-files-in-a-folder-using-c
	-> I used this article to understand how to loop through directory.

	2. https://linux.die.net/man/3/execl
	3. https://stackoverflow.com/questions/69526224/how-to-use-execl
	-> I used these resources to remember execl syntax.

	--------------------------------------------------
*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>

int launchPWordCount(const char* testFilePath, const char* processCount, const char* buffersize_reader, const char* buffersize_counterManager, const char* buffersize_counter, long* memoryUsageHolder, double* executionTimeHolder) {
	
	// Begin measuring execution time (externally)
	struct timespec startTime, endTime;
	clock_gettime(CLOCK_MONOTONIC, &startTime);

	// Make struct for mem usage
	struct rusage stats;

	// Make fork to launch process
	pid_t pid = fork();
	if (pid < 0) {
		printf("Failed to fork process to launch pwordcount!\n");
		return -1;
	}

	// Child process to launch pwordcount
	if (pid == 0) {

		// Launch pwordcount
		execl("../../build/pwordcount", 
			"pwordcount", 
			"-c", "../../config/pwordcount.config",
			"-f", testFilePath,
			"-n", processCount, 
			"-x", buffersize_reader,
			"-y", buffersize_counterManager,
			"-z", buffersize_counter,
			(char *)NULL
		);

		// Detect any failures
		printf("Failed to launch pwordcount!");

	}

	// Wait for child process to finish
	int status;
	if (wait4(pid, &status, 0, &stats) == -1) {
		printf("Failed to wait for pwordcount process to finish!\n");
		return -1;
	}

	// End measuring execution time and calculate total time
	clock_gettime(CLOCK_MONOTONIC, &endTime);
	*executionTimeHolder = (endTime.tv_sec - startTime.tv_sec) + (endTime.tv_nsec - startTime.tv_nsec) / 1e9;

	// Get memory usage for child after execution
	*memoryUsageHolder = stats.ru_maxrss;

	// Return success
	return 0;
}

int main(int argc, char **argv) {

	// Initialize a csv to log memory results
	FILE* memoryLogFile = fopen("results/performance_memory.csv", "w");
	if (!memoryLogFile) {
		printf("Failed to create performance memory log file!\n");
		return 1;
	}

	// Write headers for mem
	fprintf(memoryLogFile, "Test File,Process Count,Reader Buffer Size,Counter Manager Buffer Size, Counter Buffer Size,Memory Usage\n");

	// Initialize a csv to log time results
	FILE* timeLogFile = fopen("results/performance_time.csv", "w");
	if (!timeLogFile) {
		printf("Failed to create performance time log file!\n");
		fclose(memoryLogFile);
		return 1;
	}

	// Write headers for time
	fprintf(timeLogFile, "Test File,Process Count,Reader Buffer Size,Counter Manager Buffer Size, Counter Buffer Size,Execution Time (seconds)\n");

	// Get the test file directory which should be located at ../testfiles/*.txt
	DIR *testFileDirectory = opendir("../testfiles");
	if (!testFileDirectory) {
		return 1;
	}

	// Iterate over test files
	struct dirent *entry;
	while ((entry = readdir(testFileDirectory)) != NULL) {
		
		// Get file name in sep variable for easier use
		const char* testFileName = entry->d_name;

		// If hidden, skip
		if (testFileName[0] == '.') {
			continue;
		}

		// If not a .txt file, skip (lazy check)
		if (strstr(testFileName, ".txt") == NULL) {
			continue;
		}

		// Iterate over process counts
		for (int processCount = 1; processCount <= 8; processCount++) {

			// Iterate over reader buffer sizes
			for (int bufferSize_reader = 1024; bufferSize_reader <= 1024 * 8; bufferSize_reader *= 2) {

				// Iterate over counter manager buffer sizes
				for (int bufferSize_counterManager = 1024; bufferSize_counterManager <= 1024 * 8; bufferSize_counterManager *= 2) {

					// Iterate over counter buffer sizes
					for (int bufferSize_counter = 1024; bufferSize_counter <= 1024 * 8; bufferSize_counter *= 2) {

						// Make holders for timeand memory
						long memoryUsage;
						double executionTime;

						// Turn process count into string
						char processCountString[4];
						snprintf(processCountString, sizeof(processCountString), "%d", processCount);

						// Turn buffer sizes into strings
						char bufferSize_readerString[16];
						snprintf(bufferSize_readerString, sizeof(bufferSize_readerString), "%d", bufferSize_reader);
						char bufferSize_counterManagerString[16];
						snprintf(bufferSize_counterManagerString, sizeof(bufferSize_counterManagerString), "%d", bufferSize_counterManager);
						char bufferSize_counterString[16];
						snprintf(bufferSize_counterString, sizeof(bufferSize_counterString), "%d", bufferSize_counter);

						// Get relative path to test file for the pwordcount
						char testFilePath[256];
						snprintf(testFilePath, sizeof(testFilePath), "../testfiles/%s", testFileName);

						// Launch pwordcount for this test file and process count
						int launchStatus = launchPWordCount(testFilePath, processCountString, bufferSize_readerString, bufferSize_counterManagerString, bufferSize_counterString, &memoryUsage, &executionTime);
						if (launchStatus != 0) {
							printf("Failed to launch pwordcount for test file '%s' with %d processes!\n", testFileName, processCount);
							closedir(testFileDirectory);
							fclose(memoryLogFile);
							fclose(timeLogFile);
							return 1;
						}

						// Log memory to csv
						fprintf(memoryLogFile, "%s,%d,%d,%d,%d,%ld\n", testFileName, processCount, bufferSize_reader, bufferSize_counterManager, bufferSize_counter, memoryUsage);

						// Log time to csv
						fprintf(timeLogFile, "%s,%d,%d,%d,%d,%.6f\n", testFileName, processCount, bufferSize_reader, bufferSize_counterManager, bufferSize_counter, executionTime);
					}
				}
			}
		}

	}

	// Close files and dir
	fclose(memoryLogFile);
	fclose(timeLogFile);
	closedir(testFileDirectory);

	// Return
	return 0;
}