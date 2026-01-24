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

	--------------------------------------------------
*/

// Libraries
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// Project Libraries
#include "pwc_utils.h"
#include "pwc_reader.h"
#include "pwc_counterManager.h"
#include "pwc_logger.h"

// Module Name
#define PWC_MODULE_NAME "MAIN"

/*
	# Main Execution

	Provides main program entry

	## Command-Line Syntax
	./pwordcount <file-path> <number-of-cores>

	## Arguments
	- file-path: The path to the file to be read and have its words counted.
	- number-of-cores: (Optional) The number of CPU cores to utilize for counting

*/
int main(int argc, char **argv) {

	// Log divider so log file is easier to read
	pwc_log(PWC_LOGLEVEL_INFO, PWC_MODULE_NAME, "------------------------------------------------");
	pwc_log(PWC_LOGLEVEL_INFO, PWC_MODULE_NAME, "----- New Execution of pWordCount Starting -----");
	pwc_log(PWC_LOGLEVEL_INFO, PWC_MODULE_NAME, "------------------------------------------------");

	// If we were not given any arguments, then error out
	if (argc < 2) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "No arguments were given! Correct Usage: ./pwordcount <file-path> <number-of-cores>!");
		return 1;
	}
	
	// If we were given more than two arguments, then error out
	if (argc > 3) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Too many arguments were provided! Correct Usage: ./pwordcount <file-path> <number-of-cores>!");
		return 1;
	}

	// Get the file path from the first argument
	const char* filePath = argv[1];

	// Extract file extension from path by looking for last '.' and validate text file
	char* lastDotInFilePath = strrchr(filePath, '.');
	if (lastDotInFilePath == NULL || strcasecmp(lastDotInFilePath, ".txt") != 0) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Invalid .txt file path specified: '%s'!", filePath);
		return 1;
	}

	// If number of cores given, parse it
	long numberOfProgramCores;
	const long numberOfSystemCores = sysconf(_SC_NPROCESSORS_ONLN);
	if (argc == 3) {
		
		// Convert string to long with error checking
		char *endOfParsedString;
		numberOfProgramCores = strtol(argv[2], &endOfParsedString, 10);

		// If it did not parse the entire string, then we likely got faulty input, so error out
		if (*endOfParsedString != '\0') {
			pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Invalid number of cores specified! Could not parse given argument: '%s'.", argv[2]);
			return 1;
		}

		// Make sure that we were given at least 1 core
		if (numberOfProgramCores < 1) {
			pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "Invalid number of cores specified! The number of cores parsed is less than the minimum of 1: '%ld'!", numberOfProgramCores);
			return 1;
		}

		// If we were given more cores than the system has, warn the user
		if (numberOfProgramCores > numberOfSystemCores) {
			pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "The number of cores specified (%ld) exceeds the maximum available cores (%ld)! Defaulting to maximum available cores!", numberOfProgramCores, numberOfSystemCores);
			numberOfProgramCores = numberOfSystemCores;
		}
	} 
	// If number of cores not given, default to 1
	else {
		numberOfProgramCores = 1;
		pwc_log(PWC_LOGLEVEL_WARNING, PWC_MODULE_NAME, "No number of cores specified, defaulting to 1 core! Note that there are up to %ld cores available on this system!", numberOfSystemCores);
	}

	// Log starting information
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Starting execution for '%s' using %ld cores.", filePath, numberOfProgramCores);

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
		int counterStatus = pwc_initCounterManager(numberOfProgramCores, counterToReaderPipeFileDescriptor[1], readerToCounterPipeFileDescriptor[0]);

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
	int wordCount = pwc_reader_streamFileToPipe(filePath, readerToCounterPipeFileDescriptor[1], counterToReaderPipeFileDescriptor[0]);

	// Check wordCount for errors
	if (wordCount < 0) {
		pwc_log(PWC_LOGLEVEL_ERROR, PWC_MODULE_NAME, "An error occurred while reading the file and counting words.");
		return 1;
	}

	// Print the final word count
	pwc_log(PWC_LOGLEVEL_INFO, PWC_MODULE_NAME, "The total word count from file '%s' is: %d", filePath, wordCount);

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

	// Main return
	pwc_log(PWC_LOGLEVEL_DEBUG, PWC_MODULE_NAME, "Program execution completed successfully for '%s'!", filePath);
	return 0;
}
