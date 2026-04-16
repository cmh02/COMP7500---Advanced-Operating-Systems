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