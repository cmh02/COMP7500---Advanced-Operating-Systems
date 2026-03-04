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

// Pre-Defined Messages
static const char *AUBATCH_MESSAGE_LOCKMUTEX = "Locking the scheduler job queue mutex!";
static const char *AUBATCH_MESSAGE_UNLOCKMUTEX = "Unlocking the scheduler job queue mutex!";

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
	# Job Queue Mutex

	This mutex will be used to lock the job queue.
*/
static pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

int aubatch_scheduler_setSchedulingPolicy(enum aubatch_schedulingPolicy policy) {

	// Make sure the given policy is valid
	if (policy == AUBATCH_SCHEDULINGPOLICY_NOTSET) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Cannot set scheduling policy to NOTSET!");
		return 1;
	}

	// Lock the queue mutex
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);

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
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);

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
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);
	return aubatch_scheduler_currentJobQueue.totalExpectedWaitTime;
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);
}

uint8_t aubatch_scheduler_getCurrentQueueSize() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);
	return aubatch_scheduler_currentJobQueue.size;
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);
}

uint8_t aubatch_scheduler_getCurrentTotalSeenJobs() {
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);
	return aubatch_scheduler_currentJobQueue.totalSeenJobs;
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);
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
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);
	
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
			while ((currentNode != NULL) && (currentNode->job.execution_time <= node->job.execution_time)) {
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
			pthread_mutex_unlock(&queueMutex);
			aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);
			return 1;

		}

	}

	// Set arrival time of job to now
	node->job.arrival_time = time(NULL);

	// Increment queue size, total seen jobs, and expected wait time
	aubatch_scheduler_currentJobQueue.size++;
	aubatch_scheduler_currentJobQueue.totalSeenJobs++;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime += job.execution_time;

	// Unlock the queue mutex
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u according to %d policy! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentSchedulingPolicy, aubatch_scheduler_currentJobQueue.size);
	return 0;

}

struct aubatch_jobNode* aubatch_scheduler_screenshotJobQueue() {

	// Lock the queue mutex
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_LOCKMUTEX);
	pthread_mutex_lock(&queueMutex);

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
	pthread_mutex_unlock(&queueMutex);
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "%s", AUBATCH_MESSAGE_UNLOCKMUTEX);

	// Return pointer to first node in screenshot
	return startNode;
}