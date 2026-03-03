/*
	--------------------------------------------------

	# Information

	File: aubatch_jobs.c
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

// Project Libraries
#include "aubatch_jobs.h"

struct aubatch_job aubatch_createNewJob(int executionTime, int priority) {

	// Make new struct for job
	struct aubatch_job newJob;

	// Get new ID and verify success
	newJob.id = aubatch_jobs_generateNextJobID();
	if (newJob.id == 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to generate a Job ID when creating new job! Will attempt to make job with ID 0.");
		return newJob;
	}

	// Set other fields for job and return struct
	newJob.execution_time = executionTime;
	newJob.priority = priority;
	newJob.status = AUBATCH_JOBSTATUS_NEW;

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Created new job with ID %u, execution time %u, and priority %u.", newJob.id, newJob.execution_time, newJob.priority);
	return newJob;
}

uint32_t aubatch_jobs_nextJobID;
uint32_t aubatch_jobs_generateNextJobID() {

	// Check that var for next ID has been initialized
	if (aubatch_jobs_nextJobID == 0) {
		aubatch_jobs_nextJobID = 1;
		return aubatch_jobs_nextJobID;
	}

	// Check that we haven't maxed out ID yet
	if (aubatch_jobs_nextJobID == UINT32_MAX) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "The maximum Job ID has been reached - cannot generate new Job ID's!");
		return 0;
	}

	// Return next ID and increment for next job
	return aubatch_jobs_nextJobID++;
}

struct aubatch_job aubatch_jobs_runJob(struct aubatch_job job) {

	// Log job run
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Running job with ID %u, execution time %u, and priority %u.", job.id, job.execution_time, job.priority);
	job.status = AUBATCH_JOBSTATUS_RUNNING;

	// Sleep for the job's execution time
	sleep(job.execution_time);

	// Log job completion and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Completed job with ID %u.", job.id);
	job.status = AUBATCH_JOBSTATUS_COMPLETED;
	return job;

}