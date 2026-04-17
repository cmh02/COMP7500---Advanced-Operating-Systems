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

	3. https://www.w3schools.com/c/ref_ctype_isalnum.php
	-> I used this resource for checking alphanumeric characters in the name.


	--------------------------------------------------
*/

/*
LIBRARIES
*/

#include <ctype.h>
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

// Display settings
#define NUM_BLOCKS_PER_DISPLAY_ROW 16

// CPM specified extent formatting
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

// Pre-defined value for undefined integers (I just used the course code)
#define CPM_INT_UNDEFINED -7500 

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
DirStructType LOAD/WRITE
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

/*
NAME CHECKING
*/


/*
	# Check Legal Character

	This is a helper utility for checking a specific character. This will allow for
	checking whether a character is either alphanumeric, a separator, or a terminator.

	## Parameters

	- char c: character to check

	## Returns

	- true for legal character, false for illegal character
*/
bool checkLegalCharacter(char c) {

	// Check for valid
	if (isalnum(c) || c == '.' || c == '\0') {
		return true;
	}
	return false;

}

/*
	# Check Legal Name

	This function will be used to check a file name for certain rules
	imposed by CPM. Primarily, file names must:
	- Follow 8.3 format (8 for name, 3 for extention)
	- Not be blank
	- Not be null
	- Not contain illegal characters (non-alpha-numeric)

	## Parameters

	- char *name: pointer to string containing name and extention

	## Returns

	-- true for legal name, false for illegal name

	## Valid Examples

	Byte:	0	1	2	3	4	5	6	7	8	9	10	11	12
	Ex:		f	i	l	e	n	a	m	e	.	e	x	t	\0
	Ex: 	f	i	l	e	n	a	m	e	.	e	x	\0
	Ex: 	f	i	l	e	n	a	m	e	.	\0
	Ex: 	f	i	l	e	n	a	m	e	\0
	Ex:		f	i	l	e	.	e	x	t	\0
	Ex: 	f	i	l	e	.	\0
	Ex: 	f	i	l	e	\0	

*/
bool checkLegalName(char *name) {

	// Early check for null pointer
	if (name == NULL) {
		return false;
	}

	// Early check for blank name
	if (name[0] == '\0') {
		return false;
	}

	// Check up to max 13 bytes (8 for name, 1 for separator, 3 for extension, 1 for terminator)
	bool flag_nameSegmentFound = false;
	bool flag_separatorFound = false;
	bool flag_extensionSegmentFound = false;
	bool flag_terminatorFound = false;
	int index_name_start = CPM_INT_UNDEFINED;
	int index_name_end = CPM_INT_UNDEFINED;
	int index_separator = CPM_INT_UNDEFINED;
	int index_extension_start = CPM_INT_UNDEFINED;
	int index_extension_end = CPM_INT_UNDEFINED;
	int index_terminator = CPM_INT_UNDEFINED;
	for (int i = 0; i < (EXTENT_FORMAT_BYTELENGTH_NAME + 1 + EXTENT_FORMAT_BYTELENGTH_EXTENSION + 1); i++) {

		// Check for legal character
		if (!checkLegalCharacter(name[i])) {
			return false;
		}

		// Name segment discovery
		if (!flag_nameSegmentFound) {

			// Check for start of name segment
			if (index_name_start == CPM_INT_UNDEFINED) {

				index_name_start = i;
				continue;

			}

			// Check for terminator, indicating only a name segment without separator / extension
			if (name[i] == '\0') {

				flag_nameSegmentFound = true;
				flag_terminatorFound = true;
				index_name_end = i - 1;
				index_terminator = i;
				break;

			}

			// Check for separator, indicating we have a file extension segment to find too
			if (name[i] == '.') {

				flag_nameSegmentFound = true;
				flag_separatorFound = true;
				index_name_end = i - 1;
				index_separator = i;
				continue;

			}

			// If we are still looking for name segment but have maxxed bytes (i = 8, byte = 9), this is illegal
			if (i == EXTENT_FORMAT_BYTELENGTH_NAME) {

				return false;

			}

		}

		// Extension segment discovery
		if (flag_nameSegmentFound && flag_separatorFound && !flag_extensionSegmentFound && !flag_terminatorFound) {

			// Check for terminator, indicating we have found the end of the extension segment
			if (name[i] == '\0') {

				// Check whether we actually had extension or not
				if (index_extension_start != CPM_INT_UNDEFINED) {
					flag_extensionSegmentFound = true;
					index_extension_end = i - 1;
				}

				flag_terminatorFound = true;
				index_terminator = i;
				break;

			}

			// Check for start of extension segment
			if (index_extension_start == CPM_INT_UNDEFINED) {

				index_extension_start = i;
				continue;

			}

			// If we are still looking for extension segment but have maxxed bytes (i = 12, byte = 13), this is illegal
			if (i == (EXTENT_FORMAT_BYTELENGTH_NAME + 1 + EXTENT_FORMAT_BYTELENGTH_EXTENSION)) {

				return false;

			}

		}

	}
	return true;

}