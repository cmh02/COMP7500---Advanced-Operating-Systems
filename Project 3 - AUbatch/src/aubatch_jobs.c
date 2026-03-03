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

	// Initialize a new job
	struct aubatch_job newJob;

	// Get new ID and verify success
	newJob.id = aubatch_jobs_generateNextJobID();
	if (newJob.id == 0) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to generate a Job ID when creating new job! Will attempt to make job with ID 0.");
		return newJob;
	}

	// Copy over execution time & priority, set status to NEW
	newJob.execution_time = executionTime;
	newJob.priority = priority;
	newJob.status = AUBATCH_JOBSTATUS_NEW;

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Created new job with ID %u, execution time %u, and priority %u.", newJob.id, newJob.execution_time, newJob.priority);
	return newJob;
}

uint32_t aubatch_jobs_nextJobID;
uint32_t aubatch_jobs_generateNextJobID() {

	// Start ID's at 1 so we can reserve 0 for error cases / maxing out ID's
	if (aubatch_jobs_nextJobID == 0) {
		aubatch_jobs_nextJobID = 1;
		return aubatch_jobs_nextJobID;
	}

	// Check that we haven't maxed out ID's yet
	if (aubatch_jobs_nextJobID == UINT32_MAX) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "The maximum Job ID has been reached - cannot generate new Job ID's!");
		return 0;
	}

	// Return next ID and increment for next job
	return aubatch_jobs_nextJobID++;
}

struct aubatch_job aubatch_jobs_runJob(struct aubatch_job job) {

	// Log job run and update status
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Running job with ID %u, execution time %u, and priority %u.", job.id, job.execution_time, job.priority);
	job.status = AUBATCH_JOBSTATUS_RUNNING;

	// Mock the job's execution for the specified time
	sleep(job.execution_time);

	// Log job completion and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Completed job with ID %u.", job.id);
	job.status = AUBATCH_JOBSTATUS_COMPLETED;
	return job;

}

int aubatch_jobQueue_dequeueJob(struct aubatch_jobQueue* queue) {

	// If the queue is empty, log and return failure
	if (queue->size == 0) {
		aubatch_log(AUBATCH_LOGLEVEL_WARNING, AUBATCH_MODULE_NAME, "Attempted to dequeue job from empty queue!");
		return 1;
	}

	// Get job from head of queue and update queue
	struct aubatch_job job = queue->head->job;
	queue->head = queue->head->next;

	// Update size, log, and return
	queue->size--;
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Dequeued job with ID %u from job queue.", job.id);
	return 0;
}

struct aubatch_jobNode* aubatch_jobQueue_getNodeAtIndex(struct aubatch_jobQueue* queue, uint32_t index) {

	// If the queue is empty or index is out of bounds, return NULL
	if (queue->size == 0 || index >= queue->size) {
		return NULL;
	}

	// If in first half of queue, traverse from head
	struct aubatch_jobNode* currentNode;
	if (index < queue->size / 2) {
		
		// Traversing from head
		currentNode = queue->head;
		for (uint32_t i = 0; i < index; i++) {
			currentNode = currentNode->next;
		}

	// If in second half of queue, traverse from tail
	} else {

		// Traversing from tail
		currentNode = queue->tail;
		for (uint32_t i = queue->size - 1; i > index; i--) {
			currentNode = currentNode->prev;
		}

	}
	return currentNode;
}

int aubatch_jobQueue_insertJobAtIndex(struct aubatch_jobQueue* queue, struct aubatch_job job, uint32_t index) {

	// If the queue is empty, just enqueue the job
	if (queue->size == 0) {
		return aubatch_jobQueue_enqueueJob(queue, job);
	}

	// Create new node for job
	struct aubatch_jobNode node;
	node.job = job;
	node.next = NULL;
	node.prev = NULL;

	// Handle inserting into empty queue
	if (queue->size == 0) {
		aubatch_jobQueue_spliceJobNode(NULL, NULL, &node);
		queue->head = &node;
		queue->tail = &node;
		queue->size++;

		// Log and return
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u in empty job queue.", node.job.id);
		return 0;
	}

	// Handle inserting at beginning of non-empty queue
	if (index == 0) {
		aubatch_jobQueue_spliceJobNode(NULL, queue->head, &node);
		queue->head = &node;
		queue->size++;

		// Log and return
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u at beginning of job queue.", node.job.id);
		return 0;
	}

	// Handle inserting at end of non-empty queue
	if (index >= queue->size) {
		aubatch_jobQueue_spliceJobNode(queue->tail, NULL, &node);
		queue->tail = &node;
		queue->size++;

		// Log and return
		aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u at end of job queue.", node.job.id);
		return 0;
	}

	// Get the node currently at index
	struct aubatch_jobNode* currentNodeAtIndex = aubatch_jobQueue_getNodeAtIndex(queue, index);
	if (currentNodeAtIndex == NULL) {
		aubatch_log(AUBATCH_LOGLEVEL_ERROR, AUBATCH_MODULE_NAME, "Failed to get node at index %u when inserting job with ID %u into job queue!", index, node.job.id);
		return 1;
	}

	// Insert the new node inplace by splicing it
	aubatch_jobQueue_spliceJobNode(currentNodeAtIndex->prev, currentNodeAtIndex, &node);
	queue->size++;

	// Log and return
	aubatch_log(AUBATCH_LOGLEVEL_DEBUG, AUBATCH_MODULE_NAME, "Inserted job with ID %u at index %u of job queue.", node.job.id, index);
	return 0;
}

int aubatch_jobQueue_spliceJobNode(struct aubatch_jobNode* node1, struct aubatch_jobNode* node2, struct aubatch_jobNode* newNode) {

	// Update new node to point to node1 (prev) and node2 (next)
	newNode->prev = node1;
	newNode->next = node2;

	// Update node1 and node2 to point to new node
	node1->next = newNode;
	node2->prev = newNode;

	// Return success
	return 0;
}