/*

	--------------------------------------------------

	# Information

	File: pwc_utils.h
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides several utilities for use throughout pWordCount.

	--------------------------------------------------

	## References

	1. https://www.geeksforgeeks.org/c/variadic-functions-in-c/
	-> I used this GeeksForGeeks article to refamiliarize myself with making variadic functions for custom printing.

	--------------------------------------------------

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
