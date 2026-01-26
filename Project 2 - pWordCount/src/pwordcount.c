/*
	--------------------------------------------------

	# Information

	File: main.c
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	Main execution file for pWordCount.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/c/fork-system-call/
	-> I used this GeeksForGeeks article to help understand the fork() call and its underlying behavior when creating processes.

	2. https://www.geeksforgeeks.org/c/pipe-system-call/
	-> I used this GeeksForGeeks article to familarize myself with pipe() and correct read/write syntax.

	3. https://stackoverflow.com/questions/7021725/how-to-convert-a-string-to-integer-in-c
	5. https://www.geeksforgeeks.org/c/converting-string-to-long-in-c/
	-> I used these articles/resources to help understand how to properly convert strings to integers with error checking.

	6. https://man7.org/linux/man-pages/man3/sysconf.3.html
	7. https://stackoverflow.com/questions/150355/programmatically-find-the-number-of-cores-on-a-machine
	-> I used these articles/resources to learn different ways of finding the number of cores on a system.

	8. https://www.geeksforgeeks.org/c/strrchr-in-c/
	-> I used this article to understand safe ways of extracting file extension from a string path.

	9. https://stackoverflow.com/questions/5820810/case-insensitive-string-comparison-in-c
	10. https://pubs.opengroup.org/onlinepubs/9699919799/functions/strcasecmp.html
	-> I used these resources to understand how to do case-insensitive string comparisons.

	11. https://man7.org/linux/man-pages/man2/getrusage.2.html
	12. https://www.geeksforgeeks.org/c/wait-system-call-c/
	13. https://stackoverflow.com/questions/35316374/why-did-wait4-get-replaced-by-waitpid
	14. https://man7.org/linux/man-pages/man3/waitpid.3p.html
	15. https://man7.org/linux/man-pages/man2/getrusage.2.html
	-> I used these resources to understand / debate best practices for waiting and getting resource stats for processes.

	16. https://stackoverflow.com/questions/8957222/are-there-standards-for-linux-command-line-switches-and-arguments
	17. https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap12.html
	18. https://man7.org/linux/man-pages/man3/getopt.3.html
	-> I used these resources to understand / debate best practices for command-line flags/args.

	--------------------------------------------------
*/

// Libraries
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_reader.h"
#include "pwc_counterManager.h"
#include "pwc_logger.h"
#include "pwc_config.h"

// Module Name
#define PWC_MODULE_NAME "MAIN"

// Command Options Table
static struct option longOptions[] = {
	{"file", required_argument, 0, 'f'},
	{"config", required_argument, 0, 'c'},
	{"nprocesses", required_argument, 0, 'n'},
	{"logdir", required_argument, 0, 'l'},
	{"debug_stdout", no_argument, 0, 's'},
	{"debug_log", no_argument, 0, 'd'},
	{0, 0, 0, 0}
};

/*
	# Main Execution

	Provides main program entry

	## Command-Line Syntax
	./pwordcount <file-path> <number-of-cores>

	## Arguments
	- file-path: The path to the file to be read and have its words counted.
	- number-of-processes: (Optional) The number of counter processes to be ran. This should be between 1 and the number of cores available on your system.

*/
int main(int argc, char **argv) {

	// Create config and populate defaults
	struct pwc_configuration *config = pwc_configuration();
	pwc_populateDefaultConfiguration(config);

	// Create a temporary config struct and parse command line args into it with getopt
	int opt;
	struct pwc_configuration tempConfig;
	pwc_populateNullConfiguration(&tempConfig);
	while ((opt = getopt_long(argc, argv, "f:c:n:l:s:d:", longOptions, NULL)) != -1) {
		switch (opt) {

			// Text File Path
			case 'f': 
				strncpy(tempConfig.TEXT_FILE_PATH, optarg, PATH_MAX - 1);
				tempConfig.TEXT_FILE_PATH[PATH_MAX - 1] = '\0';
				break;

			// Config File Path
			case 'c':
				strncpy(tempConfig.CONFIG_FILE_PATH, optarg, PATH_MAX - 1);
				tempConfig.CONFIG_FILE_PATH[PATH_MAX - 1] = '\0';
				break;

			// Number of Processes
			case 'n':
				pwc_parseUnsignedLong(optarg, &tempConfig.NUMBER_OF_PROCESSES);
				break;
			
			// Logging Directory
			case 'l':
				strncpy(tempConfig.LOGGING_DIRECTORY, optarg, PATH_MAX - 1);
				tempConfig.LOGGING_DIRECTORY[PATH_MAX - 1] = '\0';
				break;

			// Send Debug to Stdout
			case 's':
				pwc_parseBool(optarg, &tempConfig.LOGGING_SEND_DEBUG_TO_STDOUT);
				break;

			// Send Debug to Log
			case 'd':
				pwc_parseBool(optarg, &tempConfig.LOGGING_SEND_DEBUG_TO_LOG);
				break;

			// Unknown Option
			default:
				fprintf(stderr, "Unknown command line option provided! Correct Usage: ./pwordcount --file <file-path> [--nprocesses <number-of-processes>] [--config <config-file-path>] [--logdir <logging-directory>] [--debug_stdout] [--debug_log]!\n");
				return 1;
		}
	}

	// Override config location if needed and load file
	if (tempConfig.CONFIG_FILE_PATH[0] != '\0') { 
		strncpy(config->CONFIG_FILE_PATH, tempConfig.CONFIG_FILE_PATH, PATH_MAX - 1); 
		config->CONFIG_FILE_PATH[PATH_MAX - 1] = '\0'; 
	}
	pwc_loadConfigurationFile(config->CONFIG_FILE_PATH, config);

	// Override log location if needed and initialize logging
	if (tempConfig.LOGGING_DIRECTORY[0] != '\0') { 
		strncpy(config->LOGGING_DIRECTORY, tempConfig.LOGGING_DIRECTORY, PATH_MAX - 1); 
		config->LOGGING_DIRECTORY[PATH_MAX - 1] = '\0';
	}
	if (pwc_initLogFile(getpid())) {
		fprintf(stderr, "Failed to initialize log file for main process with PID %d!\n", getpid());
		return 1;
	}

	// If file was not given, error out, else copy to global config
	if ((strcmp(tempConfig.TEXT_FILE_PATH, "") == 0) || (tempConfig.TEXT_FILE_PATH[0] == '\0')) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "No text file path was specified! Please use the -f <file_name.txt> argument to specify a text file!");
		return 1;
	}
	strncpy(config->TEXT_FILE_PATH, tempConfig.TEXT_FILE_PATH, PATH_MAX - 1);
	config->TEXT_FILE_PATH[PATH_MAX - 1] = '\0';

	// Extract file extension from path by looking for last '.' and validate text file
	char* lastDotInFilePath = strrchr(tempConfig.TEXT_FILE_PATH, '.');
	if (lastDotInFilePath == NULL || strcasecmp(lastDotInFilePath, ".txt") != 0) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Invalid .txt file path specified: '%s'!", tempConfig.TEXT_FILE_PATH);
		return 1;
	}

	// If number of processes given, validate and copy to global
	const long numberOfSystemCores = sysconf(_SC_NPROCESSORS_ONLN);
	if (isdigit(tempConfig.NUMBER_OF_PROCESSES) && (tempConfig.NUMBER_OF_PROCESSES != PWC_UNSET_UNSIGNED_LONG)) {

		// Make sure that we were given at least 1 process
		if (tempConfig.NUMBER_OF_PROCESSES < 1) {
			pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Invalid number of processes specified! The number of cores parsed is less than the minimum of 1: '%ld'!", tempConfig.NUMBER_OF_PROCESSES);
			return 1;
		}

		// If we were given more processes than available cores, warn the user
		if (tempConfig.NUMBER_OF_PROCESSES > numberOfSystemCores) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "The number of processes specified (%ld) exceeds the maximum available cores (%ld)! Defaulting to maximum available cores!", tempConfig.NUMBER_OF_PROCESSES, numberOfSystemCores);
			tempConfig.NUMBER_OF_PROCESSES = numberOfSystemCores;
		}

		// Copy
		config->NUMBER_OF_PROCESSES = tempConfig.NUMBER_OF_PROCESSES;
	}

	// Copy over logging options if set in command line
	if (tempConfig.LOGGING_SEND_DEBUG_TO_LOG) { config->LOGGING_SEND_DEBUG_TO_LOG = tempConfig.LOGGING_SEND_DEBUG_TO_LOG; }
	if (tempConfig.LOGGING_SEND_DEBUG_TO_STDOUT) { config->LOGGING_SEND_DEBUG_TO_STDOUT = tempConfig.LOGGING_SEND_DEBUG_TO_STDOUT; }

	// Log starting information
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Starting execution for '%s' using %ld counting processes.", config->TEXT_FILE_PATH, config->NUMBER_OF_PROCESSES);

	// Start time tracking
	struct pwc_executionTimeStruct execTimeStruct;
	pwc_startExecutionTimeTracking(&execTimeStruct);

	// Initialize two pipes, one for reader->counter and one for counter->reader
	int readerToCounterPipeFileDescriptor[2];
	int counterToReaderPipeFileDescriptor[2];

	// Create the pipes and handle any errors
	if (pipe(readerToCounterPipeFileDescriptor) == -1) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to create the reader-to-counter pipe has failed!");
		return 1;
	}
	if (pipe(counterToReaderPipeFileDescriptor) == -1) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to create the counter-to-reader pipe has failed!");
		return 1;
	}

	// Split into parent and child processes
	pid_t pid = fork();

	// Handle child process, which will be used for counting the number of words via pipe
	if (pid == 0) {
		
		// Only read from the reader-to-counter pipe
		close(readerToCounterPipeFileDescriptor[1]);

		// Only write to the counter-to-reader pipe
		close(counterToReaderPipeFileDescriptor[0]);

		// Start up the counter module to count words from the pipe
		int counterStatus = pwc_initCounterManager(config->NUMBER_OF_PROCESSES, counterToReaderPipeFileDescriptor[1], readerToCounterPipeFileDescriptor[0]);

		// Exit child process
		exit(0);
	}

	// Error detection incase fork fails
	else if (pid < 0) {

		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "The attempt to fork() into parent and child processes has failed!");
		exit(1);
	}
		
	// Only write to the reader-to-counter pipe
	close(readerToCounterPipeFileDescriptor[0]);

	// Only read from the counter-to-reader pipe
	close(counterToReaderPipeFileDescriptor[1]);

	// Start up the reader module to stream file to the pipe
	int wordCount = pwc_reader_streamFileToPipe(config->TEXT_FILE_PATH, readerToCounterPipeFileDescriptor[1], counterToReaderPipeFileDescriptor[0]);

	// Check wordCount for errors
	if (wordCount < 0) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "An error occurred while reading the file and counting words.");
		return 1;
	}

	// Print the final word count
	pwc_log(PWC_LOGLEVEL_INFO, PWC_MODULE_NAME, "The total word count from file '%s' is: %d", config->TEXT_FILE_PATH, wordCount);
	// Wait for child processes to finish
	int status;
	waitpid(pid, &status, 0);
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Main process with PID %d has observed child Counter-Manager process with PID %d finish with status code %d!", getpid(), pid, status);

	// Get all stats for child processes
	struct rusage childUsageStats;
	if (getrusage(RUSAGE_CHILDREN, &childUsageStats) == 0) {
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Memory Usage Statistics for child processes:");
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> User CPU Time Used: %ld seconds and %ld microseconds", childUsageStats.ru_utime.tv_sec, childUsageStats.ru_utime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> System CPU Time Used: %ld seconds and %ld microseconds", childUsageStats.ru_stime.tv_sec, childUsageStats.ru_stime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Maximum Resident Set Size: %ld %s", childUsageStats.ru_maxrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Shared Memory Size: %ld %s", childUsageStats.ru_ixrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Data Size: %ld %s", childUsageStats.ru_idrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Stack Size: %ld %s", childUsageStats.ru_isrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Page Reclaims (Soft Page Faults): %ld", childUsageStats.ru_minflt);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Page Faults (Hard Page Faults): %ld", childUsageStats.ru_majflt);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Swaps: %ld", childUsageStats.ru_nswap);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Block Input Operations: %ld", childUsageStats.ru_inblock);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Block Output Operations: %ld", childUsageStats.ru_oublock);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Voluntary Context Switches: %ld", childUsageStats.ru_nvcsw);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Involuntary Context Switches: %ld", childUsageStats.ru_nivcsw);
	} else {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "An error occurred while attempting to get resource usage statistics for child processes.");
	}

	// Get all stats for main process
	struct rusage mainUsageStats;
	if (getrusage(RUSAGE_SELF, &mainUsageStats) == 0) {
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Memory Usage Statistics for main process:");
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> User CPU Time Used: %ld seconds and %ld microseconds", mainUsageStats.ru_utime.tv_sec, mainUsageStats.ru_utime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> System CPU Time Used: %ld seconds and %ld microseconds", mainUsageStats.ru_stime.tv_sec, mainUsageStats.ru_stime.tv_usec);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Maximum Resident Set Size: %ld %s", mainUsageStats.ru_maxrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Shared Memory Size: %ld %s", mainUsageStats.ru_ixrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Data Size: %ld %s", mainUsageStats.ru_idrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Integral Unshared Stack Size: %ld %s", mainUsageStats.ru_isrss, PWC_MEMORY_UNIT);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Page Reclaims (Soft Page Faults): %ld", mainUsageStats.ru_minflt);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Page Faults (Hard Page Faults): %ld", mainUsageStats.ru_majflt);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Swaps: %ld", mainUsageStats.ru_nswap);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Block Input Operations: %ld", mainUsageStats.ru_inblock);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Block Output Operations: %ld", mainUsageStats.ru_oublock);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Voluntary Context Switches: %ld", mainUsageStats.ru_nvcsw);
		pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, " -> Involuntary Context Switches: %ld", mainUsageStats.ru_nivcsw);
	} else {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "An error occurred while attempting to get resource usage statistics for main process.");
	}

	// Stop time tracking and log total time
	pwc_stopExecutionTimeTracking(&execTimeStruct);
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Total Execution Time for processing file '%s' with %ld processes: %.6f seconds.", config->TEXT_FILE_PATH, config->NUMBER_OF_PROCESSES, execTimeStruct.total);

	// Main return
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Program execution completed successfully for '%s'!", config->TEXT_FILE_PATH);
	return 0;
}
