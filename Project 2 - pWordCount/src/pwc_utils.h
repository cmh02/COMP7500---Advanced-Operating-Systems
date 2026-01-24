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

	2. https://sourceforge.net/p/predef/wiki/OperatingSystems/
	3. https://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
	-> I used these to understand how to detect Apple vs Linux and define macro for memory unit.

	--------------------------------------------------

*/

// Libraries
#include <stdarg.h>

// Prevent multiple inclusions
#ifndef PWC_UTILS_H
#define PWC_UTILS_H



// Memory Unit

/* 
	# Note on Memory Units

	As different systems report mem usage in different units (MAC is bytes, Linux is KB), this gives an easy way to print out the right unit.
*/
#if defined(__APPLE__) && defined(__MACH__)
	#define PWC_MEMORY_UNIT "bytes"
#else
	#define PWC_MEMORY_UNIT "kilobytes"
#endif



// Function Stubs

/*
	# Print With Prefix

	Print a message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_printWithPrefix(const char *message, va_list args);

/*
	# Warn With Prefix

	Print a warning message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_warnWithPrefix(const char *message, va_list args);

/*
	# Error With Prefix

	Print an error message to std out with pWordCount prefix.

	## Arguments
	- message: The message to print with format symbols
	- args : Arguments for format symbols

	## Returns
	- void
*/
void pwc_errorWithPrefix(const char *message, va_list args);

#endif // PWC_UTILS_H
