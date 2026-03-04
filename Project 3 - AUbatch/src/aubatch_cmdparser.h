/*
	--------------------------------------------------

	# Information

	File: aubatch_cmdparser.h
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the command parser module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Prevent multiple inclusions
#ifndef AUBATCH_CMDPARSER_H
#define AUBATCH_CMDPARSER_H

// Pre-Defined Messages
static const char *AUBATCH_MESSAGE_INTRO = 
	"Welcome to " AUBATCH_PROGRAM_AUTHOR "'s batch job scheduler Version " AUBATCH_PROGRAM_VERSION "\n"
	"Type 'help' to find more about AUbatch commands.\n";

static const char *AUBATCH_MESSAGE_HELP = 
	"run <job> <time> <pri>: submit a job named <job>,\n"
	"						 execution time is <time>,\n"
	"						 priority is <pri>.\n"
	"list: display the job status.\n"
	"fcfs: change the scheduling policy to FCFS.\n"
	"sjf: change the scheduling policy to SJF.\n"
	"priority: change the scheduling policy to priority.\n"
	"test <benchmark> <policy> <num_of_jobs> <priority_levels>\n"
	"	  <min_CPU_time> <max_CPU_time>\n"
	"quit: exit AUbatch.\n";

static const char *AUBATCH_MESSAGE_JOBSUBMISSION = 
	"Job %s was submitted.\n"
	"Total number of jobs in the queue: %u.\n"
	"Expected waiting time: %u seconds.\n"
	"Scheduling Policy: %s.\n";

static const char *AUBATCH_MESSAGE_EXIT = 
	"Total number of jobs submitted: %u\n"
	"Average turnaround time:\t%f seconds\n"
	"Average CPU time:\t%f seconds\n"
	"Average waiting time:\t%f seconds\n"
	"Throughput:\t\t%f No./second\n";

/*
	# Command Parser Command Loop

	Enter the command loop for the command parser.

	## Arguments
	- None

	## Returns
	- void
*/
int aubatch_cmdparser_enterCommandLoop();

#endif // AUBATCH_CMDPARSER_H