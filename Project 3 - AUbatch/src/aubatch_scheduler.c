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

// Project Libraries
#include "aubatch_jobs.h"
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
} aubatch_scheduler_currentJobMetrics;
struct aubatch_currentJobMetrics aubatch_scheduler_currentJobMetrics = { .job = { .id = 0 }, .time_poppedFromQueue = 0 };

/*
	# Finished Job Queue

	This variable will reference the queue of all finished jobs.
	This is purely for metric tracking post-run.
*/
static struct aubatch_jobQueue aubatch_scheduler_finishedJobQueue;

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
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Locking the scheduler job queue mutex!");
	return pthread_mutex_lock(&queueMutex);
}

/*
	# Job Queue Mutex - Unlock

	This function will unlock the job queue mutex, with debug logging.
*/
int aubatch_scheduler_unlockQueueMutex() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", "Unlocking the scheduler job queue mutex!");
	return pthread_mutex_unlock(&queueMutex);
}

/*
	# Job Queue Condition Variable

	This CV will be used to signal when the job queue is no longer empty.
*/
static pthread_cond_t queueNotEmptyCV = PTHREAD_COND_INITIALIZER;

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
	aubatch_scheduler_currentJobQueue = (struct aubatch_jobQueue){ .head = NULL, .tail = NULL, .size = 0, .totalSeenJobs = 0, .totalExpectedWaitTime = 0 };

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
	return (totalExpectedWaitTime + (aubatch_scheduler_currentJobMetrics.job.time_requestedExecution - (time(NULL) - aubatch_scheduler_currentJobMetrics.time_poppedFromQueue)));
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

int aubatch_scheduler_insert(struct aubatch_job job) {

	// Log insertion attempt
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Request was made to insert job with ID %u into queue according to %s policy!", job.id, aubatch_scheduler_getSchedulingPolicyName());

	// Make a new node for the job
	struct aubatch_jobNode* node = malloc(sizeof(struct aubatch_jobNode));
	if (node == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to allocate memory (malloc) for job node to contain job with ID %u!", job.id);
		return 1;
	} else {
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Successfully allocated memory (malloc) for job node to contain job with ID %u!", job.id);
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

			// Simply insert job at end of queue, updating head/tail as needed
			aubatch_jobQueue_spliceJobNode(aubatch_scheduler_currentJobQueue.tail, NULL, node);
			if (node->prev == NULL) {
				aubatch_scheduler_currentJobQueue.head = node;
			}
			if (node->next == NULL) {
				aubatch_scheduler_currentJobQueue.tail = node;
			}
			break;

		}

		// Shortest Job First
		case AUBATCH_SCHEDULINGPOLICY_SJF: {

			// Traverse list to find first job with a longer execution time
			struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
			while ((currentNode != NULL) && (currentNode->job.time_requestedExecution <= node->job.time_requestedExecution)) {
				currentNode = currentNode->next;
			}

			// Insert before the found job with longer execution time, updating head/tail as needed
			aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, node);
			if (currentNode == aubatch_scheduler_currentJobQueue.head) {
				aubatch_scheduler_currentJobQueue.head = node;
			}
			else if (currentNode == aubatch_scheduler_currentJobQueue.tail) {
				aubatch_scheduler_currentJobQueue.tail = node;
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

			// Insert before the found job with lower priority value, updating head/tail as needed
			aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, node);
			if (currentNode == aubatch_scheduler_currentJobQueue.head) {
				aubatch_scheduler_currentJobQueue.head = node;
			}
			else if (currentNode == aubatch_scheduler_currentJobQueue.tail) {
				aubatch_scheduler_currentJobQueue.tail = node;
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

	// Set arrival time of job to now
	node->job.time_arrival = time(NULL);

	// Increment queue size, total seen jobs, and expected wait time
	aubatch_scheduler_currentJobQueue.size++;
	aubatch_scheduler_currentJobQueue.totalSeenJobs++;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime += job.time_requestedExecution;

	// Send signal on CV that queue is no longer empty
	pthread_cond_signal(&queueNotEmptyCV);

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u according to %d policy! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentSchedulingPolicy, aubatch_scheduler_currentJobQueue.size);
	return 0;

}

struct aubatch_jobNode* aubatch_scheduler_screenshotJobQueue() {

	// Lock the queue mutex
	aubatch_scheduler_lockQueueMutex();

	// Make initial pointer for start of screenshot list
	struct aubatch_jobNode* startNode = NULL;

	// Iterate over nodes in the current queue
	size_t screenshotSize = 0;
	struct aubatch_jobNode* lastAddedNode = NULL;
	struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
	while (currentNode != NULL) {

		// Get job
		struct aubatch_job job = currentNode->job;

		// Make new node for screenshot
		struct aubatch_jobNode* screenshotNode = malloc(sizeof(struct aubatch_jobNode));
		if (screenshotNode == NULL) {
			aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to allocate memory (malloc) for job node screenshot!");
			continue;
		}
		screenshotNode->job = job;

		// If this is the first node in the screenshot just set startNode to it
		if (screenshotSize == 0) {
			startNode = screenshotNode;
		}

		// Otherwise, splice screenshot node into screenshot list
		else {
			aubatch_jobQueue_spliceJobNode(lastAddedNode, NULL, screenshotNode);
		}

		// Update lastAddedNode, inc screenshot size, move on
		lastAddedNode = screenshotNode;
		screenshotSize++;
		currentNode = currentNode->next;
	}

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Return pointer to first node in screenshot
	return startNode;
}

struct aubatch_job aubatch_scheduler_popJobQueue() {

	// Lock the queue mutex
	aubatch_scheduler_lockQueueMutex();

	// Wait on cv for queue to not be empty
	while (aubatch_scheduler_currentJobQueue.size == 0) {
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

	// Move queue head up
	aubatch_scheduler_currentJobQueue.head = node->next;
	if (aubatch_scheduler_currentJobQueue.head != NULL) {
		aubatch_scheduler_currentJobQueue.head->prev = NULL;
	}
	
	// Free memory for old head node
	free(node);

	// Update size and expected wait time for queue
	aubatch_scheduler_currentJobQueue.size--;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime -= job.time_requestedExecution;

	// Unlock the queue mutex
	aubatch_scheduler_unlockQueueMutex();

	// Record pop time for current job metrics
	aubatch_scheduler_currentJobMetrics.job = job;
	time(&aubatch_scheduler_currentJobMetrics.time_poppedFromQueue);

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Popped job with ID %u from the queue! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentJobQueue.size);
	return job;
}

int aubatch_scheduler_recordFinishedJob(struct aubatch_job job) {

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

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Recorded finished job with ID %u into finished job queue! There are now %u jobs in the finished job queue!", job.id, aubatch_scheduler_finishedJobQueue.size);
	return 0;
}