/*
	--------------------------------------------------

	# Information

	File: cpmfsys.c
	Project: Project 4 - CPM
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides required imeplementation for the CMP FS.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/c/global-variables-in-c/
	-> I used this resource to refamiliarize myself with global variables.

	2. https://stackoverflow.com/questions/3649026/how-can-i-display-hexadecimal-numbers-in-c
	-> I used this resource to refamiliarize myself with hex formatting in C.


	--------------------------------------------------
*/

/*
LIBRARIES
*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

/*
PROJECT LIBRARIES 
*/

#include "cpmfsys.h"

/*
MACROS
*/

// BLOCK_SIZE defined in diskSimulator.h as 1024
// NUM_BLOCKS defined in diskSimulator.h as 256
#define NUM_BLOCKS_PER_DISPLAY_ROW 16

#define EXTENT_SIZE 32
#define EXTENT_FORMAT_BYTESTART_STATUS 0
#define EXTENT_FORMAT_BYTESTART_NAME 1
#define EXTENT_FORMAT_BYTESTART_EXTENSION 9
#define EXTENT_FORMAT_BYTESTART_XL 12
#define EXTENT_FORMAT_BYTESTART_BC 13
#define EXTENT_FORMAT_BYTESTART_XH 14
#define EXTENT_FORMAT_BYTESTART_RC 15
#define EXTENT_FORMAT_BYTESTART_BLOCKS 16
#define EXTENT_FORMAT_BYTELENGTH_STATUS 1
#define EXTENT_FORMAT_BYTELENGTH_NAME 8
#define EXTENT_FORMAT_BYTELENGTH_EXTENSION 3
#define EXTENT_FORMAT_BYTELENGTH_XL 1
#define EXTENT_FORMAT_BYTELENGTH_BC 1
#define EXTENT_FORMAT_BYTELENGTH_XH 1
#define EXTENT_FORMAT_BYTELENGTH_RC 1
#define EXTENT_FORMAT_BYTELENGTH_BLOCKS 16

/*
GLOBAL FREE LIST
*/

// Free block list (true = free, false = used)
bool global_freeList[NUM_BLOCKS];

// Initialization
void makeFreeList() {

	// First block never free since it holds directory
	global_freeList[0] = false;

	// Set all blocks to free
	for (int i = 1; i < NUM_BLOCKS; i++) {
		global_freeList[i] = true;
	}

}

// Display free list
void printFreeList() {

	// Split up block list into rows of 16
	for (int firstBlockNumberInRow = 0; firstBlockNumberInRow < NUM_BLOCKS; firstBlockNumberInRow += NUM_BLOCKS_PER_DISPLAY_ROW) {

		// Print 2-digit hex address of first block in row
		printf("%02x ", firstBlockNumberInRow);

		// Loop through blocks in row
		for (int blockIndexInRow = 0; blockIndexInRow < NUM_BLOCKS_PER_DISPLAY_ROW; blockIndexInRow++) {

			// Show used blocks as '*', free as 'a'
			int blockNumber = firstBlockNumberInRow + blockIndexInRow;
			if (global_freeList[blockNumber]) {
				printf("a");
			} else {
				printf("*");
			}
		}
		printf("\n");
	}
}

/*
DirStructType
*/

DirStructType *mkDirStruct(int index, uint8_t *e) {

	// Find correct extent from block 0
	uint8_t *ptr_extentEntryStart = e + (index * EXTENT_SIZE);

	// Allocate memory for new DirStructType
	DirStructType *ptr_newDirStructType = malloc(sizeof(DirStructType));

	// Copy fields into struct
	memcpy(ptr_newDirStructType->status, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_STATUS, EXTENT_FORMAT_BYTELENGTH_STATUS);
	memcpy(ptr_newDirStructType->name, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_NAME, EXTENT_FORMAT_BYTELENGTH_NAME);
	memcpy(ptr_newDirStructType->extension, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_EXTENSION, EXTENT_FORMAT_BYTELENGTH_EXTENSION);
	memcpy(ptr_newDirStructType->XL, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XL, EXTENT_FORMAT_BYTELENGTH_XL);
	memcpy(ptr_newDirStructType->BC, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BC, EXTENT_FORMAT_BYTELENGTH_BC);
	memcpy(ptr_newDirStructType->XH, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XH, EXTENT_FORMAT_BYTELENGTH_XH);
	memcpy(ptr_newDirStructType->RC, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_RC, EXTENT_FORMAT_BYTELENGTH_RC);
	memcpy(ptr_newDirStructType->blocks, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BLOCKS, EXTENT_FORMAT_BYTELENGTH_BLOCKS);

	// Return pointer to new DirStructType
	return ptr_newDirStructType;

}

void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e) {

	// Find correct extent from block 0
	uint8_t *ptr_extentEntryStart = e + (index * EXTENT_SIZE);

	// Copy fields from struct into block 0
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_STATUS, d->status, EXTENT_FORMAT_BYTELENGTH_STATUS);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_NAME, d->name, EXTENT_FORMAT_BYTELENGTH_NAME);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_EXTENSION, d->extension, EXTENT_FORMAT_BYTELENGTH_EXTENSION);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XL, d->XL, EXTENT_FORMAT_BYTELENGTH_XL);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BC, d->BC, EXTENT_FORMAT_BYTELENGTH_BC);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XH, d->XH, EXTENT_FORMAT_BYTELENGTH_XH);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_RC, d->RC, EXTENT_FORMAT_BYTELENGTH_RC);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BLOCKS, d->blocks, EXTENT_FORMAT_BYTELENGTH_BLOCKS);

}