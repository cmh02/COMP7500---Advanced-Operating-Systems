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

// Project Libraries
#include "aubatch_jobs.h"

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

	## Parameters
	- None

	## Returns
	- enum aubatch_schedulingPolicy: the current scheduling policy for the scheduler
*/
enum aubatch_schedulingPolicy aubatch_scheduler_getSchedulingPolicy();

/*
	# Get Scheduling Policy Name

	This function will return the name of the current scheduling policy for the scheduler.

	## Parameters
	- None

	## Returns
	- const char*: the name of the current scheduling policy for the scheduler
*/
const char* aubatch_scheduler_getSchedulingPolicyName();

/*
	# Get Current Wait Time

	This function will get current (estimated) wait time for queue.

	## Parameters
	- None

	## Returns
	- uint8_t: the current (estimated) wait time for queue
*/
uint8_t aubatch_scheduler_getCurrentWaitTime();

/*
	# Get Current Queue Size

	This function will get the current size of queue.

	## Parameters
	- None

	## Returns
	- uint8_t: the current size of queue
*/
uint8_t aubatch_scheduler_getCurrentQueueSize();

/*
	# Get Current Total Seen Jobs

	This function will get the current total seen jobs for queue.

	## Parameters
	- None

	## Returns
	- uint8_t: the current total seen jobs for the current queue
*/
uint8_t aubatch_scheduler_getCurrentTotalSeenJobs();

/*
	# Get Current Job Queue

	This function will provide a copy of the current job queue.
	Changing it will NOT change the actual job queue.

	## Parameters
	- None

	## Returns
	- struct aubatch_jobQueue: a copy of the current job queue
*/
struct aubatch_jobQueue aubatch_scheduler_getCurrentJobQueue();

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

/*
	# Screenshot Job Queue

	This function will provide a deep copy of the current job queue as a linked list of job nodes.
	
	## Parameters
	- None
	
	## Returns
	- struct aubatch_jobNode*: pointer to the first node in the screenshot linked list
*/
struct aubatch_jobNode* aubatch_scheduler_screenshotJobQueue();

/*
	# Pop Job Queue

	This function will pop the first job from the job queue and return it.
	The queue will already have been sorted prior to this occuring.

	## Parameters
	- None

	## Returns
	- struct aubatch_job: the job that was popped from the queue
*/
struct aubatch_job aubatch_scheduler_popJobQueue();


#endif // AUBATCH_SCHEDULER_H