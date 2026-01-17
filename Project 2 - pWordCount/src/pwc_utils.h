/*
	File: pwc_utils.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02
*/

// Prevent multiple inclusions
#ifndef PWC_UTILS_H
#define PWC_UTILS_H

// Function stubs

/*
	# Print With Prefix

	Print a message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- ... : Arguments for format symbols

	## Returns
	- void
*/
void pwc_printWithPrefix(const char *message, ...);

/*
	# Warn With Prefix

	Print a warning message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- ... : Arguments for format symbols

	## Returns
	- void
*/
void pwc_warnWithPrefix(const char *message, ...);

/*
	# Error With Prefix

	Print an error message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- ... : Arguments for format symbols

	## Returns
	- void
*/
void pwc_errorWithPrefix(const char *message, ...);

#endif // PWC_UTILS_H
