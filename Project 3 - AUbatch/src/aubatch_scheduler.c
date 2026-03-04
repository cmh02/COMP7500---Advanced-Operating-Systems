/*
	--------------------------------------------------

	# Information

	File: aubatch_scheduler.c
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the scheduler module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Libraries
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

// Project Libraries
#include "aubatch_jobs.h"
#include "aubatch_utils.h"
#include "aubatch_logger.h"
#include "aubatch_scheduler.h"

// Module Name
#define AUBATCH_MODULE_NAME "SCHEDULER"

/*
	# Current Scheduling Policy

	This variable will identify the current scheduling policy in-use.
*/
static enum aubatch_schedulingPolicy aubatch_scheduler_currentSchedulingPolicy;

/*
	# Current Job Queue

	This variable will reference the current main queue to be used for jobs.
	Other queues may exist, but only this queue will actually serve jobs.
*/
static struct aubatch_jobQueue aubatch_scheduler_currentJobQueue;

/*
	# Currently Executing Job

	This variable will track the currently execution job. This allows for estimations
	based on last-known information about job timing and such.
*/
struct aubatch_currentJobMetrics {
	struct aubatch_job job;
	time_t time_poppedFromQueue;
	bool isCurrentlyExecuting;
} aubatch_scheduler_currentJobMetrics;
struct aubatch_currentJobMetrics aubatch_scheduler_currentJobMetrics = { 
	.job = (struct aubatch_job){ .id = 0 }, 
	.time_poppedFromQueue = 0,
	.isCurrentlyExecuting = false
};

/*
	# Job Queue Mutex

	This mutex will be used to lock the job queue.
*/
static pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

/* 
	# Job Queue Mutex - Lock

	This function will lock the job queue mutex, with debug logging.
*/
int aubatch_scheduler_lockQueueMutex() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Locking the scheduler waiting job queue mutex!");
	return pthread_mutex_lock(&queueMutex);
}

/*
	# Job Queue Mutex - Unlock

	This function will unlock the job queue mutex, with debug logging.
*/
int aubatch_scheduler_unlockQueueMutex() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Unlocking the scheduler waiting job queue mutex!");
	return pthread_mutex_unlock(&queueMutex);
}

/*
	# Job Queue Condition Variable

	This CV will be used to signal when the job queue is no longer empty.
*/
static pthread_cond_t queueNotEmptyCV = PTHREAD_COND_INITIALIZER;

/*
	# Finished Job Queue

	This variable will reference the queue of all finished jobs.
	This is purely for metric tracking post-run.
*/
static struct aubatch_jobQueue aubatch_scheduler_finishedJobQueue;

/*
	# Finished Job Queue Mutex

	This mutex will be used to lock the finished job queue.
*/
static pthread_mutex_t finishedQueueMutex = PTHREAD_MUTEX_INITIALIZER;

/*
	# Finished Job Queue Mutex - Lock
*/
int aubatch_scheduler_lockFinishedQueueMutex() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Locking the scheduler finished job queue mutex!");
	return pthread_mutex_lock(&finishedQueueMutex);
}

/*
	# Finished Job Queue Mutex - Unlock
*/
int aubatch_scheduler_unlockFinishedQueueMutex() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Unlocking the scheduler finished job queue mutex!");
	return pthread_mutex_unlock(&finishedQueueMutex);
}

/*
	# Finished Job Queue Condition Variable
*/
static pthread_cond_t finishedQueueUpdatedCV = PTHREAD_COND_INITIALIZER;

int aubatch_scheduler_setSchedulingPolicy(enum aubatch_schedulingPolicy policy) {

	// Make sure the given policy is valid
	if (policy == AUBATCH_SCHEDULINGPOLICY_NOTSET) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Cannot set scheduling policy to NOTSET!");
		return 1;
	}

	// Lock the queue mutex
	aubatch_scheduler_lockQueueMutex();

	// Set the current scheduling policy
	aubatch_scheduler_currentSchedulingPolicy = policy;

	// Make a new queue and sort all jobs according to new policy
	struct aubatch_jobQueue oldQueue = aubatch_scheduler_currentJobQueue;
	aubatch_scheduler_currentJobQueue = (struct aubatch_jobQueue){ .head = NULL, .tail = NULL, .size = 0, .totalSeenJobs = 0, .totalExpectedWaitTime = 0, .queueBeginTime = 0, .queueEndTime = 0 };

	// Re-insert all jobs with new policy
	struct aubatch_jobNode* nextNode;
	struct aubatch_jobNode* currentNode = oldQueue.head;
	while (currentNode != NULL) {

		// Insert
		aubatch_scheduler_insert(currentNode->job);

		// Safely free old node since we make a new node for each job
		nextNode = currentNode->next;
		free(currentNode);
		currentNode = nextNode;
	}

	// Set queue begin time
	aubatch_scheduler_currentJobQueue.queueBeginTime = time(NULL);

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Set scheduling policy to %s.", aubatch_scheduler_getSchedulingPolicyName());
	return 0;
}

enum aubatch_schedulingPolicy aubatch_scheduler_getSchedulingPolicy() {
	return aubatch_scheduler_currentSchedulingPolicy;
}

const char* aubatch_scheduler_getSchedulingPolicyName() {
	return aubatch_schedulingPolicyNames[aubatch_scheduler_currentSchedulingPolicy];
}

uint8_t aubatch_scheduler_getCurrentWaitTime() {
	aubatch_scheduler_lockQueueMutex();
	uint8_t totalExpectedWaitTime = aubatch_scheduler_currentJobQueue.totalExpectedWaitTime;
	aubatch_scheduler_unlockQueueMutex();
	if ((aubatch_scheduler_currentJobMetrics.isCurrentlyExecuting) && (aubatch_scheduler_currentJobMetrics.job.id != 0)) {
		return (totalExpectedWaitTime + (aubatch_scheduler_currentJobMetrics.job.time_requestedExecution - (time(NULL) - aubatch_scheduler_currentJobMetrics.time_poppedFromQueue)));
	} else {
		return totalExpectedWaitTime;
	}
}

uint8_t aubatch_scheduler_getCurrentQueueSize() {
	aubatch_scheduler_lockQueueMutex();
	uint8_t size = aubatch_scheduler_currentJobQueue.size;
	aubatch_scheduler_unlockQueueMutex();
	return size;
}

uint8_t aubatch_scheduler_getCurrentTotalSeenJobs() {
	aubatch_scheduler_lockQueueMutex();
	uint8_t totalSeenJobs = aubatch_scheduler_currentJobQueue.totalSeenJobs;
	aubatch_scheduler_unlockQueueMutex();
	return totalSeenJobs;
}

double aubatch_scheduler_getCurrentAverageTurnaroundTime() {

	// Define vars before locking
	uint32_t totalTurnaroundTime = 0;
	double averageTurnaroundTime = 0.0;

	// Lock mutex while we calculate
	aubatch_scheduler_lockFinishedQueueMutex();
	if (aubatch_scheduler_finishedJobQueue.size == 0) {
		averageTurnaroundTime = 0.0;
	} else {
		struct aubatch_jobNode* currentNode = aubatch_scheduler_finishedJobQueue.head;
		while (currentNode != NULL) {
			totalTurnaroundTime += currentNode->job.time_turnaround;
			currentNode = currentNode->next;
		}
		averageTurnaroundTime = (double)totalTurnaroundTime / (double)aubatch_scheduler_finishedJobQueue.size;
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Calculated average turnaround time of %f seconds based on total turnaround time of %u seconds and total seen job count of %u!", averageTurnaroundTime, totalTurnaroundTime, aubatch_scheduler_finishedJobQueue.size);
	}

	// Unlock and return
	aubatch_scheduler_unlockFinishedQueueMutex();
	return averageTurnaroundTime;
}

double aubatch_scheduler_getCurrentAverageCPUTime() {

	// Define vars before locking
	uint32_t totalCPUTime = 0;
	double averageCPUTime = 0.0;

	// Lock mutex while we calculate
	aubatch_scheduler_lockFinishedQueueMutex();
	if (aubatch_scheduler_finishedJobQueue.size == 0) {
		averageCPUTime = 0.0;
	} else {
		struct aubatch_jobNode* currentNode = aubatch_scheduler_finishedJobQueue.head;
		while (currentNode != NULL) {
			totalCPUTime += currentNode->job.time_actualExecution;
			currentNode = currentNode->next;
		}
		averageCPUTime = (double)totalCPUTime / (double)aubatch_scheduler_finishedJobQueue.size;
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Calculated average CPU time of %f seconds based on total CPU time of %u seconds and total seen job count of %u!", averageCPUTime, totalCPUTime, aubatch_scheduler_finishedJobQueue.size);
	}

	// Unlock and return
	aubatch_scheduler_unlockFinishedQueueMutex();
	return averageCPUTime;
}

double aubatch_scheduler_getCurrentAverageWaitTime() {

	// Define vars before locking
	uint32_t totalWaitTime = 0;
	double averageWaitTime = 0.0;

	// Lock mutex while we calculate
	aubatch_scheduler_lockFinishedQueueMutex();
	if (aubatch_scheduler_finishedJobQueue.size == 0) {
		averageWaitTime = 0.0;
	} else {
		struct aubatch_jobNode* currentNode = aubatch_scheduler_finishedJobQueue.head;
		while (currentNode != NULL) {
			totalWaitTime += currentNode->job.time_wait;
			currentNode = currentNode->next;
		}
		averageWaitTime = (double)totalWaitTime / (double)aubatch_scheduler_finishedJobQueue.size;
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Calculated average wait time of %f seconds based on total wait time of %u seconds and total seen job count of %u!", averageWaitTime, totalWaitTime, aubatch_scheduler_finishedJobQueue.size);
	}

	// Unlock and return
	aubatch_scheduler_unlockFinishedQueueMutex();
	return averageWaitTime;
}

double aubatch_scheduler_getCurrentThroughput() {

	// Lock
	aubatch_scheduler_lockQueueMutex();

	// Calculate
	double throughput = aubatch_scheduler_currentJobQueue.totalSeenJobs / difftime(time(NULL), aubatch_scheduler_currentJobQueue.queueBeginTime);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Calculated throughput of %f no./second based on total seen job count of %u and queue duration of %f seconds!", throughput, aubatch_scheduler_currentJobQueue.totalSeenJobs, difftime(time(NULL), aubatch_scheduler_currentJobQueue.queueBeginTime));

	// Unlock and return
	aubatch_scheduler_unlockQueueMutex();
	return throughput;
}

int aubatch_scheduler_insert(struct aubatch_job job) {

	// Log insertion attempt
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Request was made to insert job with ID %u into queue according to %s policy!", job.id, aubatch_scheduler_getSchedulingPolicyName());

	// Set job to waiting
	job.status = AUBATCH_JOBSTATUS_WAITING;

	// Make a new node for the job
	struct aubatch_jobNode* node = malloc(sizeof(struct aubatch_jobNode));
	if (node == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to allocate memory (malloc) for job node to contain job with ID %u!", job.id);
		return 1;
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Successfully allocated memory (malloc) for job node to contain job with ID %u!", job.id);
	}
	node->job = job;
	node->next = NULL;
	node->prev = NULL;

	// Lock the queue mutex
	aubatch_scheduler_lockQueueMutex();
	
	// Handle insertion based on policy
	switch (aubatch_scheduler_currentSchedulingPolicy) {

		// First Come First Serve
		case AUBATCH_SCHEDULINGPOLICY_FCFS: {

			// Simply insert job at end of queue
			aubatch_jobQueue_spliceJobNode(aubatch_scheduler_currentJobQueue.tail, NULL, node);
			break;

		}

		// Shortest Job First
		case AUBATCH_SCHEDULINGPOLICY_SJF: {

			// Traverse list to find first job with a longer execution time
			struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
			while ((currentNode != NULL) && (currentNode->job.time_requestedExecution <= node->job.time_requestedExecution)) {
				currentNode = currentNode->next;
			}

			// Insert before the found job with longer execution time
			if (currentNode == NULL) {
				aubatch_jobQueue_spliceJobNode(aubatch_scheduler_currentJobQueue.tail, NULL, node);
			} else {
				aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, node);
			}
			break;
		}

		// Priority Scheduling (Lower Value == Higher Priority)
		case AUBATCH_SCHEDULINGPOLICY_PRIORITY: {

			// Traverse list to find first job with a higher priority value (meaning lower priority)
			struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
			while ((currentNode != NULL) && (currentNode->job.priority <= node->job.priority)) {
				currentNode = currentNode->next;
			}

			// Insert before the found job with lower priority value
			if (currentNode == NULL) {
				aubatch_jobQueue_spliceJobNode(aubatch_scheduler_currentJobQueue.tail, NULL, node);
			} else {
				aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, node);
			}
			break;
		}

		// If its somehow not set
		case AUBATCH_SCHEDULINGPOLICY_NOTSET: {

			// Log, free memory, unlock mutex, return error
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Cannot insert job with ID %u into queue because the scheduling policy is not set!", job.id);
			free(node);
			aubatch_scheduler_unlockQueueMutex();
			return 1;
		}

	}

	// Update head/tail of queue as needed
	if (node->prev == NULL) {
		aubatch_scheduler_currentJobQueue.head = node;
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Updating queue head to job with ID %u since it is the first job in the queue!", node->job.id);
	}
	if (node->next == NULL) {
		aubatch_scheduler_currentJobQueue.tail = node;
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Updating queue tail to job with ID %u since it is the last job in the queue!", node->job.id);
	}

	// Log scheduling insertion info
	if (node->prev != NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Inserted job with ID %u (priority %u, exec time %u) after job with ID %u (priority %u, exec time %u)!", node->job.id, node->job.priority, node->job.time_requestedExecution, node->prev->job.id, node->prev->job.priority, node->prev->job.time_requestedExecution);
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Inserted job with ID %u (priority %u, exec time %u) with no nodes ahead of it!", node->job.id, node->job.priority, node->job.time_requestedExecution);
	}
	if (node->next != NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Inserted job with ID %u (priority %u, exec time %u) before job with ID %u (priority %u, exec time %u)!", node->job.id, node->job.priority, node->job.time_requestedExecution, node->next->job.id, node->next->job.priority, node->next->job.time_requestedExecution);
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "-> Inserted job with ID %u (priority %u, exec time %u) with no nodes behind it!", node->job.id, node->job.priority, node->job.time_requestedExecution);
	}

	// Set arrival time of job to now
	node->job.time_arrival = time(NULL);

	// Increment queue size, total seen jobs, and expected wait time
	aubatch_scheduler_currentJobQueue.size++;
	aubatch_scheduler_currentJobQueue.totalSeenJobs++;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime += job.time_requestedExecution;

	// Log within mutex since raw access to size
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u according to %d policy! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentSchedulingPolicy, aubatch_scheduler_currentJobQueue.size);

	// Send signal on CV that queue is no longer empty
	pthread_cond_signal(&queueNotEmptyCV);

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Return
	return 0;

}

struct aubatch_job aubatch_scheduler_popJobQueue() {

	// Lock the queue mutex
	aubatch_scheduler_lockQueueMutex();

	// Wait on cv for queue to not be empty
	while ((aubatch_scheduler_currentJobQueue.size == 0) || (aubatch_scheduler_currentJobQueue.head == NULL)) {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Waiting on CV for job queue to not be empty!");
		pthread_cond_wait(&queueNotEmptyCV, &queueMutex);
	}

	// Get node at front of queue
	struct aubatch_jobNode* node = aubatch_scheduler_currentJobQueue.head;
	if (node == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Cannot pop job from queue because the queue is empty!");
		aubatch_scheduler_unlockQueueMutex();
		return (struct aubatch_job){ .id = 0 };
	}

	// Get job from node
	struct aubatch_job job = node->job;

	// Record pop time for current job metrics
	aubatch_scheduler_currentJobMetrics.job = job;
	aubatch_scheduler_currentJobMetrics.time_poppedFromQueue = time(NULL);
	aubatch_scheduler_currentJobMetrics.isCurrentlyExecuting = true;

	// Move queue head up
	if (node->next == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Popped last job from the queue, the queue is now empty! Updating head and tail to NULL.");
		aubatch_scheduler_currentJobQueue.head = NULL;
		aubatch_scheduler_currentJobQueue.tail = NULL;
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Popped job with ID %u from the queue! Updating head to job with ID %u.", job.id, node->next->job.id);
		aubatch_scheduler_currentJobQueue.head = node->next;
		aubatch_scheduler_currentJobQueue.head->prev = NULL;
	}
	
	// Free memory for old head node
	free(node);

	// Update size and expected wait time for queue
	aubatch_scheduler_currentJobQueue.size--;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime -= job.time_requestedExecution;

	// Log within mutex
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Popped job with ID %u from the queue! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentJobQueue.size);

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Return
	return job;
}

int aubatch_scheduler_recordFinishedJob(struct aubatch_job job) {

	// Lock the finished job queue mutex
	aubatch_scheduler_lockFinishedQueueMutex();

	// If the job is same as one currently being executed, then clear current job metrics since it is now finished
	if ((aubatch_scheduler_currentJobMetrics.isCurrentlyExecuting) && (aubatch_scheduler_currentJobMetrics.job.id == job.id)) {
		aubatch_scheduler_currentJobMetrics.job = (struct aubatch_job){ .id = 0 };
		aubatch_scheduler_currentJobMetrics.time_poppedFromQueue = 0;
		aubatch_scheduler_currentJobMetrics.isCurrentlyExecuting = false;
	}

	// Make a new node for the finished job
	struct aubatch_jobNode* node = malloc(sizeof(struct aubatch_jobNode));
	if (node == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to allocate memory (malloc) for finished job node to contain job with ID %u!", job.id);
		return 1;
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Successfully allocated memory (malloc) for finished job node to contain job with ID %u!", job.id);
	}
	node->job = job;
	node->next = NULL;
	node->prev = NULL;

	// Insert finished job into finished job queue (just insert at end since order doesn't matter)
	aubatch_jobQueue_spliceJobNode(aubatch_scheduler_finishedJobQueue.tail, NULL, node);
	if (node->prev == NULL) {
		aubatch_scheduler_finishedJobQueue.head = node;
	}
	if (node->next == NULL) {
		aubatch_scheduler_finishedJobQueue.tail = node;
	}

	// Increment size and total seen jobs for finished job queue
	aubatch_scheduler_finishedJobQueue.size++;
	aubatch_scheduler_finishedJobQueue.totalSeenJobs++;

	// Log within mutex
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Recorded finished job with ID %u into finished job queue! There are now %u jobs in the finished job queue!", job.id, aubatch_scheduler_finishedJobQueue.size);

	// Send signal on CV that finished job queue was updated
	pthread_cond_signal(&finishedQueueUpdatedCV);

	// Unlock the finished job queue mutex
	aubatch_scheduler_unlockFinishedQueueMutex();

	// Return
	return 0;
}

void aubatch_scheduler_printJobQueue(enum aubatch_loggerLevel logLevel) {

	// Lock queue mutex
	aubatch_scheduler_lockQueueMutex();

	// Lock finished queue mutex
	aubatch_scheduler_lockFinishedQueueMutex();

	// Print total number of jobs in queue
	aubatch_log(logLevel, AUBATCH_MODULE_NAME, "Total number of jobs in the queue: %u\n", aubatch_scheduler_currentJobQueue.size);

	// Print scheduling policy
	aubatch_log(logLevel, AUBATCH_MODULE_NAME, "Scheduling Policy: %s\n", aubatch_scheduler_getSchedulingPolicyName());

	// Print header for job list
	aubatch_log(logLevel, AUBATCH_MODULE_NAME, "Name\tCPU_Time\tPri\tArrival_time\tProgress\n");

	// Iterate over finished job queue and print info for each job
	struct aubatch_jobNode* currentNode = aubatch_scheduler_finishedJobQueue.head;
	while (currentNode != NULL) {
		
		// Get job and print info
		struct aubatch_job job = currentNode->job;
		char arrivalTime[9];
		aubatch_convertTimeToHHMMSS(job.time_arrival, arrivalTime);
		aubatch_log(logLevel, AUBATCH_MODULE_NAME, "%s\t%u\t%u\t%s\t%s\n", job.name, job.time_requestedExecution, job.priority, arrivalTime, aubatch_jobs_getJobStatusName(job.status));
		
		// Move to next node
		currentNode = currentNode->next;
	}

	// Print currently executing job if there is one
	if ((aubatch_scheduler_currentJobMetrics.isCurrentlyExecuting) && (aubatch_scheduler_currentJobMetrics.job.id != 0)) {
		struct aubatch_job job = aubatch_scheduler_currentJobMetrics.job;
		char arrivalTime[9];
		aubatch_convertTimeToHHMMSS(job.time_arrival, arrivalTime);
		aubatch_log(logLevel, AUBATCH_MODULE_NAME, "%s\t%u\t%u\t%s\t%s\n", job.name, job.time_requestedExecution, job.priority, arrivalTime, aubatch_jobs_getJobStatusName(AUBATCH_JOBSTATUS_RUNNING));
	}

	// Iterate over waiting job queue and print info for each job
	currentNode = aubatch_scheduler_currentJobQueue.head;
	while (currentNode != NULL) {
		
		// Get job and print info
		struct aubatch_job job = currentNode->job;
		char arrivalTime[9];
		aubatch_convertTimeToHHMMSS(job.time_arrival, arrivalTime);
		aubatch_log(logLevel, AUBATCH_MODULE_NAME, "%s\t%u\t%u\t%s\t%s\n", job.name, job.time_requestedExecution, job.priority, arrivalTime, aubatch_jobs_getJobStatusName(job.status));
		
		// Move to next node
		currentNode = currentNode->next;
	}

	// Free finished job mutex
	aubatch_scheduler_unlockFinishedQueueMutex();

	// Free mutex
	aubatch_scheduler_unlockQueueMutex();

}

uint32_t aubatch_scheduler_waitForNJobsToFinish(uint32_t n) {

	// Lock finished mutex
	aubatch_scheduler_lockFinishedQueueMutex();

	// Wait while less than n have finished
	while (aubatch_scheduler_finishedJobQueue.size < n) {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Waiting on finished queueCV for %u jobs to finish! Currently %u have finished.", n, aubatch_scheduler_finishedJobQueue.size);
		pthread_cond_wait(&finishedQueueUpdatedCV, &finishedQueueMutex);
	}

	// Unlock
	aubatch_scheduler_unlockFinishedQueueMutex();

	// Return to indicate success
	return 0;

}