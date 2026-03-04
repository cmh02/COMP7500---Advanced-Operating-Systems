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
#include <time.h>
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
	# Job Status Names

	This will provide string representations for the job statuses.
	The index of each string corresponds to the value of the enum.
*/
static const char* aubatch_jobStatusNames[] = {
	"NEW",
	"READY",
	"RUNNING",
	"COMPLETED",
	"FAILED"
};

/* 
	# Get Job Status Name
*/
const char* aubatch_jobs_getJobStatusName(enum aubatch_jobStatus status);


/*
	# Job Data Structure

	This struct will represent a job in the system

	## Fields
	- id: the unique identifier for the job
	- execution_time: the execution time for the job
	- priority: the priority for the job

*/
struct aubatch_job {
	char name;
	uint32_t id;
	uint32_t execution_time;
	uint32_t priority;
	time_t creation_time;
	time_t arrival_time;
	time_t start_time;
	time_t completion_time;
	enum aubatch_jobStatus status;
};

/* 
	# Job Queue Node Data Structure

	This struct will provide a node for the job queue double-linked-list.

	## Fields
	- job: the job contained in this node
	- next: pointer to the next node in the queue
	- prev: pointer to the previous node in the queue
*/
struct aubatch_jobNode {
	struct aubatch_job job;
	struct aubatch_jobNode* next;
	struct aubatch_jobNode* prev;
};

/* 
	# Job Queue Data Structure

	This struct will provide a queue for the jobs in the system.
	This is implemented as a double-linked-list with some easy access and modifiers.

	## Fields
	- head: pointer to the first job in the queue
	- tail: pointer to the last job in the queue
	- size: the number of jobs currently in the queue
*/
struct aubatch_jobQueue {
	struct aubatch_jobNode* head;
	struct aubatch_jobNode* tail;
	uint32_t size;
	uint32_t totalSeenJobs;
	uint32_t totalExpectedWaitTime;
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
	- name: the name of the job
	- executionTime: the execution time for the job
	- priority: the priority for the job

	## Returns
	- struct aubatch_job: the new job struct with the given information and a generated ID
*/
struct aubatch_job aubatch_jobs_createNewJob(char name, int executionTime, int priority);

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

/*
	# Job Queue - Get Node At Index

	This function will return a pointer to the node at the specified index in the queue.

	## Parameters
	- struct aubatch_jobQueue* queue: pointer to the job queue to get the node from
	- uint32_t index: the index to get the node at

	## Returns
	- struct aubatch_jobNode*: pointer to the node at the specified index, or NULL if the index is out of bounds
*/
struct aubatch_jobNode* aubatch_jobQueue_getNodeAtIndex(struct aubatch_jobQueue* queue, uint32_t index);

/*
	# Job Queue - Splice Job Node

	This function will splice a job node into a linked list between two existing nodes.
	Note that this does not modify the queue itself, only the nodes.

	## Parameters
	- struct aubatch_jobNode* node1: pointer to the first node to splice between
	- struct aubatch_jobNode* node2: pointer to the second node to splice between
	- struct aubatch_jobNode* newNode: pointer to the new node
*/
int aubatch_jobQueue_spliceJobNode(struct aubatch_jobNode* node1, struct aubatch_jobNode* node2, struct aubatch_jobNode* newNode);