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

// Project Libraries
#include "aubatch_scheduler.h"

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

int aubatch_scheduler_setSchedulingPolicy(enum aubatch_schedulingPolicy policy) {

	// Make sure the given policy is valid
	if (policy == AUBATCH_SCHEDULINGPOLICY_NOTSET) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Cannot set scheduling policy to NOTSET!");
		return 1;
	}

	// Set the current scheduling policy
	aubatch_scheduler_currentSchedulingPolicy = policy;

	// Make a new queue and sort all jobs according to new policy
	struct aubatch_jobQueue oldQueue = aubatch_scheduler_currentJobQueue;
	aubatch_scheduler_currentJobQueue = (struct aubatch_jobQueue){ .head = NULL, .tail = NULL, .size = 0, .totalSeenJobs = 0, .totalExpectedWaitTime = 0 };

	// Re-insert all jobs with new policy
	struct aubatch_jobNode* currentNode = oldQueue.head;
	while (currentNode != NULL) {
		aubatch_scheduler_insert(currentNode->job);
		currentNode = currentNode->next;
	}

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_INFO, AUBATCH_MODULE_NAME, "Set scheduling policy to %d.", policy);
	return 0;
}

enum aubatch_schedulingPolicy aubatch_scheduler_getSchedulingPolicy() {
	return aubatch_scheduler_currentSchedulingPolicy;
}

const char* aubatch_scheduler_getSchedulingPolicyName() {
	return aubatch_schedulingPolicyNames[aubatch_scheduler_currentSchedulingPolicy];
}

uint8_t aubatch_scheduler_getCurrentWaitTime() {
	return aubatch_scheduler_currentJobQueue.totalExpectedWaitTime;
}

uint8_t aubatch_scheduler_getCurrentQueueSize() {
	return aubatch_scheduler_currentJobQueue.size;
}

uint8_t aubatch_scheduler_getCurrentTotalSeenJobs() {
	return aubatch_scheduler_currentJobQueue.totalSeenJobs;
}

int aubatch_scheduler_insert(struct aubatch_job job) {

	// Make a new node for the job
	struct aubatch_jobNode node = { 
		.job = job, 
		.next = NULL, 
		.prev = NULL 
	};

	// If queue is empty, always insert at beginning
	if (aubatch_scheduler_currentJobQueue.size == 0) {
		aubatch_scheduler_currentJobQueue.head = &node;
		aubatch_scheduler_currentJobQueue.tail = &node;
	}
	
	// Handle insertion based on policy
	switch (aubatch_scheduler_currentSchedulingPolicy) {

		// First Come First Serve
		case AUBATCH_SCHEDULINGPOLICY_FCFS: {

			// Simply insert job at end of queue
			aubatch_jobQueue_spliceJobNode(aubatch_scheduler_currentJobQueue.tail, NULL, &node);
			aubatch_scheduler_currentJobQueue.tail = &node;

		}

		// Shortest Job First
		case AUBATCH_SCHEDULINGPOLICY_SJF: {

			// Traverse list to find first job with a longer execution time
			struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
			while ((currentNode != NULL) && (currentNode->job.execution_time <= node.job.execution_time)) {
				currentNode = currentNode->next;
			}

			// Insert before the found job with longer execution time, updating head/tail as needed
			aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, &node);
			if (currentNode == aubatch_scheduler_currentJobQueue.head) {
				aubatch_scheduler_currentJobQueue.head = &node;
			}
			else if (currentNode == aubatch_scheduler_currentJobQueue.tail) {
				aubatch_scheduler_currentJobQueue.tail = &node;
			}

		}

		// Priority Scheduling (Lower Value == Higher Priority)
		case AUBATCH_SCHEDULINGPOLICY_PRIORITY: {

			// Traverse list to find first job with a higher priority value (meaning lower priority)
			struct aubatch_jobNode* currentNode = aubatch_scheduler_currentJobQueue.head;
			while ((currentNode != NULL) && (currentNode->job.priority <= node.job.priority)) {
				currentNode = currentNode->next;
			}

			// Insert before the found job with lower priority value, updating head/tail as needed
			aubatch_jobQueue_spliceJobNode(currentNode->prev, currentNode, &node);
			if (currentNode == aubatch_scheduler_currentJobQueue.head) {
				aubatch_scheduler_currentJobQueue.head = &node;
			}
			else if (currentNode == aubatch_scheduler_currentJobQueue.tail) {
				aubatch_scheduler_currentJobQueue.tail = &node;
			}

		}

	}

	// Set arrival time of job to now
	node.job.arrival_time = time(NULL);

	// Increment queue size, total seen jobs, and expected wait time
	aubatch_scheduler_currentJobQueue.size++;
	aubatch_scheduler_currentJobQueue.totalSeenJobs++;
	aubatch_scheduler_currentJobQueue.totalExpectedWaitTime += job.execution_time;

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u according to %d policy! There are now %u jobs in the queue.", job.id, aubatch_scheduler_currentSchedulingPolicy, aubatch_scheduler_currentJobQueue.size);
	return 0;

}

struct aubatch_jobQueue aubatch_scheduler_getCurrentJobQueue() {

	// bruh
	return aubatch_scheduler_currentJobQueue;
}