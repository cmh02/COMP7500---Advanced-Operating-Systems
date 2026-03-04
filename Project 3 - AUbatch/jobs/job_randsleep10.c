/*
	--------------------------------------------------

	# Information

	File: randsleep10.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides a simple program that sleeps for a random amount of time between 1 and 10 seconds.
	This will be used as a target job for the aubatch program.

	--------------------------------------------------
*/


// Libraries
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

// Main Execution
int main() {

	// Get random number
	srand(time(NULL));
	int randomSleepTime = (rand() % 10) + 1;

	// Sleep
	sleep(randomSleepTime);
	return 0;

}