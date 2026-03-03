/*
	--------------------------------------------------

	# Information

	File: aubatch_jobs.h
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the jobs module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Prevent multiple inclusions
#ifndef AUBATCH_JOBS_H
#define AUBATCH_JOBS_H

// Libraries
#include <unistd.h>
#include <stdint.h>

// Project Libraries
#include "aubatch_logger.h"

// Module Name
#define AUBATCH_MODULE_NAME "JOBS"

/*
	# Job Status

	This enum will represent the status of a job in the system.
*/
enum aubatch_jobStatus {
	AUBATCH_JOBSTATUS_NEW = 0,
	AUBATCH_JOBSTATUS_READY = 1,
	AUBATCH_JOBSTATUS_RUNNING = 2,
	AUBATCH_JOBSTATUS_COMPLETED = 3,
	AUBATCH_JOBSTATUS_FAILED = 4
};

/*
	# Job Data Structure

	This struct will represent a job in the system

	## Fields
	- id: the unique identifier for the job
	- execution_time: the execution time for the job
	- priority: the priority for the job

*/
struct aubatch_job {
	uint32_t id;
	uint32_t execution_time;
	uint32_t priority;
	enum aubatch_jobStatus status;
};

/*
	# Next Job ID

	This variable will track the next Job ID to use.
*/
extern uint32_t aubatch_jobs_nextJobID;

/*
	# Job ID Generator

	This function will generate the next ID for a new job.

	## Fields
	- None

	## Returns
	- 0 if an error occurs, or a positive uint32_t for the new ID
*/
uint32_t aubatch_jobs_generateNextJobID();

/*
	# Create New Job

	This function will create a new job with the given information.

	## Parameters
	- executionTime: the execution time for the job
	- priority: the priority for the job

	## Returns
	- struct aubatch_job: the new job struct with the given information and a generated ID
*/
struct aubatch_job aubatch_jobs_createNewJob(int executionTime, int priority);

/*
	# Run Job

	This function will run a job by sleeping for the execution time of the job.

	## Parameters
	- struct aubatch_job job: the job to run

	## Returns
	- 0 on success, 1 on any failure

*/
struct aubatch_job aubatch_jobs_runJob(struct aubatch_job job);

#endif // AUBATCH_JOBS_H