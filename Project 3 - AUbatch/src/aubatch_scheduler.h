/*
	--------------------------------------------------

	# Information

	File: aubatch_scheduler.h
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the scheduler module for AUbatch.

	--------------------------------------------------

	## References
	
	1. https://www.tutorialspoint.com/cprogramming/c_lookup_tables.htm
	2. https://www.reddit.com/r/C_Programming/comments/1b5ejqg/what_is_the_correct_way_to_define_a_lookup_table/
	-> I used these resources to build a lookup table for policy names.

	--------------------------------------------------
*/

// Prevent multiple inclusions
#ifndef AUBATCH_SCHEDULER_H
#define AUBATCH_SCHEDULER_H

// Libraries

// Project Libraries
#include "aubatch_jobs.h"

// Module Name
#define AUBATCH_MODULE_NAME "SCHEDULER"

/* 
	# Available Scheduling Policies

	This enum will offer different scheduling policies for the scheduler.

*/
enum aubatch_schedulingPolicy {
	AUBATCH_SCHEDULINGPOLICY_NOTSET = 0,
	AUBATCH_SCHEDULINGPOLICY_FCFS = 1,
	AUBATCH_SCHEDULINGPOLICY_SJF = 2,
	AUBATCH_SCHEDULINGPOLICY_PRIORITY = 3
};

/*
	# Available Scheduling Policy Names

	This array will provide string names for the scheduling policies, indexed by the enum values.
*/
static const char *aubatch_schedulingPolicyNames[] = {
	"NOT SET",
	"FCFS",
	"SJF",
	"PRIORITY"
};

/*
	# Set Scheduling Policy

	This function will set the scheduling policy for the scheduler.
	This will also re-sort the job queue according to new policy.

	## Parameters
	- enum aubatch_schedulingPolicy policy: the scheduling policy to set

	## Returns
	- 0 on success, 1 on any failure
*/
int aubatch_scheduler_setSchedulingPolicy(enum aubatch_schedulingPolicy policy);

/*
	# Get Scheduling Policy

	This function will return the current scheduling policy for the scheduler.

	## Returns
	- enum aubatch_schedulingPolicy: the current scheduling policy for the scheduler
*/
enum aubatch_schedulingPolicy aubatch_scheduler_getSchedulingPolicy();

/*
	# Scheduling Insertion

	This function will insert a job into a job queue according to the given policy.

	## Parameters
	- struct aubatch_jobQueue* queue: pointer to the job queue to insert into
	- struct aubatch_job job: the job to insert

	## Returns
	- 0 on success, 1 on any failure

*/
int aubatch_scheduler_insert(struct aubatch_job job);





#endif // AUBATCH_SCHEDULER_H