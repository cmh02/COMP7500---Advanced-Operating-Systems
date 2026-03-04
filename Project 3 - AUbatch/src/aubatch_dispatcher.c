/*
	--------------------------------------------------

	# Information

	File: aubatch_dispatcher.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the dispatcher module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Libraries
#include <errno.h>
#include <pthread.h>

// Project Libraries
#include "aubatch.h"
#include "aubatch_jobs.h"
#include "aubatch_logger.h"
#include "aubatch_scheduler.h"
#include "aubatch_dispatcher.h"

// Module Name
#define AUBATCH_MODULE_NAME "DISPATCHER"

int aubatch_dispatcher_loop() {

	while (aubatch_flag_programRunning) {

		// Wait for next job from scheduler
		struct aubatch_job job = aubatch_scheduler_popJobQueue();

		// Calculate wait time for job
		job.time_wait = time(NULL) - job.time_arrival;

		// Move job to running status and set start time
		job.status = AUBATCH_JOBSTATUS_RUNNING;
		time(&job.time_start);

		// Try to fork to get a child process for job
		pid_t childPID = fork();
		if (childPID < 0) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error %d occurred when trying to fork process for job with ID %d!", errno, job.id);
			job.status = AUBATCH_JOBSTATUS_FAILED;
			return 1;

		// Launch job (sleep with execv)
		} else if (childPID == 0) {
			execv(job.name, NULL);
		}

		// Wait for job to complete
		int childStatus;
		pid_t waitResult = waitpid(childPID, &childStatus, 0);
		if (waitResult < 0) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error %d occurred when trying to wait for child process for job with ID %d!", errno, job.id);
			job.status = AUBATCH_JOBSTATUS_FAILED;

		// Check for success or error in execution
		} else if (WIFEXITED(childStatus) && WEXITSTATUS(childStatus) == 0) {
			job.status = AUBATCH_JOBSTATUS_COMPLETED;
		} else {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Child process for job with ID %d did not exit successfully!", job.id);
			job.status = AUBATCH_JOBSTATUS_FAILED;
		}

		// Set completion time, real execution time, turnaround time for job
		job.time_completion = time(NULL);
		job.time_actualExecution = job.time_completion - job.time_start;
		job.time_turnaround = job.time_completion - job.time_arrival;
		
		// Add job to new node in finished job queue
		int jobRecordStatus = aubatch_scheduler_recordFinishedJob(job);
		if (jobRecordStatus != 0) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error %d occurred when trying to record finished job with ID %d!", jobRecordStatus, job.id);
		}

		// Log and continue loop
		aubatch_log(AUBATCH_LOGLEVEL_INFO, AUBATCH_MODULE_NAME, "Finished executing job with ID %d! Wait time was %ld seconds and turnaround time was %ld seconds.", job.id, job.time_wait, job.time_turnaround);
	}

}

int aubatch_dispatcher_start() {

	// Begin the dispatch loop in new pthread
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Starting the dispatcher loop in a new thread!");
	pthread_t dispatcherThread;
	int threadCreationResult = pthread_create(&dispatcherThread, NULL, (void*)aubatch_dispatcher_loop, NULL);
	if (threadCreationResult != 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Error %d occured when trying to create dispatcher thread!", threadCreationResult);
		return 1;
	}
}