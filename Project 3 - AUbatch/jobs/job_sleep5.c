/*
	--------------------------------------------------

	# Information

	File: sleep5.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides a simple program that sleeps for 5 seconds.
	This will be used as a target job for the aubatch program.

	--------------------------------------------------
*/


// Libraries
#include <unistd.h>

// Main Execution
int main() {

	// Sleep
	sleep(5);

	return 0;
}