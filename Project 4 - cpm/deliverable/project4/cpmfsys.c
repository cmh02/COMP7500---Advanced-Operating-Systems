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

	4. https://www.w3schools.com/c/ref_string_strcmp.php
	-> I used this resource for comparing strings.


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

// Segment size
#define BLOCK_SEGMENT_SIZE 128

// Pre-defined value for undefined integers (I just used the course code)
#define CPM_INT_UNDEFINED -7500 

/*
	# GLOBAL FREE LIST

	Global variable to represent which blocks are free (true) and which are used (false).
*/
bool global_freeList[NUM_BLOCKS];


/*
	# Directory Struct Creation

	This function will make a directory struct from the extent in dir.

	## Parameters

	- int index: the index of the extent in the directory (up to 32)
	- uint8_t *e: pointer to the start of disk block 0

	## Returns

	- pointer to newly created DirStructType

	## Caution

	Must free after done as this allocates memory!
*/
DirStructType *mkDirStruct(int index, uint8_t *e) {

	// Find correct extent from block 0
	uint8_t *ptr_extentEntryStart = e + (index * EXTENT_SIZE);

	// Allocate memory for new DirStructType
	DirStructType *ptr_newDirStructType = malloc(sizeof(DirStructType));

	// Copy fields into struct
	memcpy(&ptr_newDirStructType->status, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_STATUS, EXTENT_FORMAT_BYTELENGTH_STATUS);
	memcpy(&ptr_newDirStructType->name, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_NAME, EXTENT_FORMAT_BYTELENGTH_NAME);
	memcpy(&ptr_newDirStructType->extension, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_EXTENSION, EXTENT_FORMAT_BYTELENGTH_EXTENSION);
	memcpy(&ptr_newDirStructType->XL, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XL, EXTENT_FORMAT_BYTELENGTH_XL);
	memcpy(&ptr_newDirStructType->BC, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BC, EXTENT_FORMAT_BYTELENGTH_BC);
	memcpy(&ptr_newDirStructType->XH, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XH, EXTENT_FORMAT_BYTELENGTH_XH);
	memcpy(&ptr_newDirStructType->RC, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_RC, EXTENT_FORMAT_BYTELENGTH_RC);
	memcpy(&ptr_newDirStructType->blocks, ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BLOCKS, EXTENT_FORMAT_BYTELENGTH_BLOCKS);

	// Return pointer to new DirStructType
	return ptr_newDirStructType;

}

/*
	# Write Directory Struct

	This function will write a struct back at the specified extent.

	## Parameters

	- DirStructType *d: pointer to target struct to write
	- uint8_t index: the index of the extent in the directory (up to 32)
	- uint8_t *e: pointer to the start of disk block 0
*/
void writeDirStruct(DirStructType *d, uint8_t index, uint8_t *e) {

	// Find correct extent from block 0
	uint8_t *ptr_extentEntryStart = e + (index * EXTENT_SIZE);

	// Copy fields from struct into block 0
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_STATUS, &d->status, EXTENT_FORMAT_BYTELENGTH_STATUS);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_NAME, &d->name, EXTENT_FORMAT_BYTELENGTH_NAME);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_EXTENSION, &d->extension, EXTENT_FORMAT_BYTELENGTH_EXTENSION);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XL, &d->XL, EXTENT_FORMAT_BYTELENGTH_XL);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BC, &d->BC, EXTENT_FORMAT_BYTELENGTH_BC);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_XH, &d->XH, EXTENT_FORMAT_BYTELENGTH_XH);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_RC, &d->RC, EXTENT_FORMAT_BYTELENGTH_RC);
	memcpy(ptr_extentEntryStart + EXTENT_FORMAT_BYTESTART_BLOCKS, &d->blocks, EXTENT_FORMAT_BYTELENGTH_BLOCKS);

}

/*
	# Make Free List

	This function will initialize the free block list. It will then
	read through all extents currenttly in disk and mark any blocks
	that are in use.
*/
void makeFreeList() {

	// First block never free since it holds directory
	global_freeList[0] = false;

	// Set all blocks to free
	for (int i = 1; i < NUM_BLOCKS; i++) {
		global_freeList[i] = true;
	}

	// Load first block from disk as directory into buffer
	uint8_t dirBuffer[BLOCK_SIZE];
	blockRead(dirBuffer, 0);

	// Go through extents and mark blocks that are used
	for (int extentIndex = 0; extentIndex < (BLOCK_SIZE / EXTENT_SIZE); extentIndex++) {

		// Get addr of current extent
		uint8_t *ptr_extentInDirectory = dirBuffer + (extentIndex * EXTENT_SIZE);

		// Check status byte (byte 0) to to see if unused (0xe5 per assignment) or used (0-15 for user)
		uint8_t statusByteValue = ptr_extentInDirectory[EXTENT_FORMAT_BYTESTART_STATUS];
		if (statusByteValue == 0xe5) {
			continue;
		}

		// For used extent we need to check blocks
		for (int blockIndex = 0; blockIndex < BLOCKS_PER_EXTENT; blockIndex++) {

			// Get block number from extent
			uint8_t blockByteValue = ptr_extentInDirectory[EXTENT_FORMAT_BYTESTART_BLOCKS + blockIndex];

			// If block number is set, then mark block as used
			if (blockByteValue != 0) {
				global_freeList[blockByteValue] = false;
			}
		}
	}
}

/*
	# Print Free List

	This function will print the free block list for debugging.
*/
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
	# CPM Directory

	This function will print the directory, including name, extension, and size.
*/
void cpmDir() {

	// Load first block from disk as directory into buffer
	uint8_t dirBuffer[BLOCK_SIZE];
	blockRead(dirBuffer, 0);

	// Go through extents and mark blocks that are used
	for (int extentIndex = 0; extentIndex < (BLOCK_SIZE / EXTENT_SIZE); extentIndex++) {

		// Make a directory struct for extent
		DirStructType *ptr_dirStruct = mkDirStruct(extentIndex, dirBuffer);

		// Check status byte to see if unused (0xe5) or used (0-15 for user)
		if (ptr_dirStruct->status != 0xe5) {

			// Get number of blocks in use by extent
			int numberBlocksUsed = 0;
			for (int blockIndex = 0; blockIndex < BLOCKS_PER_EXTENT; blockIndex++) {

				// Get block number from extent
				uint8_t blockByteValue = ptr_dirStruct->blocks[blockIndex];

				// If block number is set, then mark block as used
				if (blockByteValue != 0) {
					numberBlocksUsed++;
				}
			}

			// For all n-1 blocks, we count full file size
			int totalFileSize = 0;
			totalFileSize += (BLOCK_SIZE * (numberBlocksUsed - 1));

			// For last block we have to check BC and RC
			totalFileSize += (BLOCK_SEGMENT_SIZE * ptr_dirStruct->RC);
			totalFileSize += ptr_dirStruct->BC;

			// Print `name.ext size`
			printf("%s.%s %d\n", ptr_dirStruct->name, ptr_dirStruct->extension, totalFileSize);
		}

		// Free mem for dir struct
		free(ptr_dirStruct);
	}
}

/*
	# Check Legal Character

	This is a helper utility for checking a specific character. This will allow for
	checking whether a character is either alphanumeric, a separator, or a terminator.

	## Parameters

	- char c: character to check

	## Returns

	- true for legal character, false for illegal character

	## Note

	This function is NOT required by the assignment. However, under the idea
	of each function being a "master" of one thing, this makes the flow much clearer.
*/
bool checkLegalCharacter(char c) {

	// Check for valid
	if (isalnum(c) || c == '.' || c == '\0') {
		return true;
	}
	return false;

}

/*
	# Name Information Structure

	This is a helper structure to hold the values of indices for each segment of the name.

	## Fields

	- bool flag_invalidCharactersFound: whether any invalid characters were found
	- bool flag_invalidFormatFound: whether the format was invalid (too long, etc.)
	- bool flag_nameSegmentFound: whether a name segment was found
	- bool flag_separatorFound: whether a separator was found
	- bool flag_extensionSegmentFound: whether an extension segment was found
	- bool flag_terminatorFound: whether a terminator was found
	- int index_name_start: the index of the start of the name segment
	- int index_name_end: the index of the end of the name segment
	- int index_separator: the index of the separator between name and extension
	- int index_extension_start: the index of the start of the extension segment
	- int index_extension_end: the index of the end of the extension segment
	- int index_terminator: the index of the terminator at the end of the

	## Note

	This structure is NOT required by this assignment. However, it makes the
	flow of transfering data and keeping functions isolated much easier.
*/
typedef struct nameInformationStruct {
	bool flag_invalidCharactersFound;
	bool flag_invalidFormatFound;
	bool flag_nameSegmentFound;
	bool flag_separatorFound;
	bool flag_extensionSegmentFound;
	bool flag_terminatorFound;
	int index_name_start;
	int index_name_end;
	int index_separator;
	int index_extension_start;
	int index_extension_end;
	int index_terminator;
} nameInformationStructType;

/*
	# Initialize Name Information Structure

	This function will initialize a name info structure to all default values.

	## Parameters

	- nameInformationStructType *nameInfo: pointer to struct to initialize
*/
void initializeNameInformationStruct(nameInformationStructType *nameInfo) {
	nameInfo->flag_invalidCharactersFound = false;
	nameInfo->flag_invalidFormatFound = false;
	nameInfo->flag_nameSegmentFound = false;
	nameInfo->flag_separatorFound = false;
	nameInfo->flag_extensionSegmentFound = false;
	nameInfo->flag_terminatorFound = false;
	nameInfo->index_name_start = CPM_INT_UNDEFINED;
	nameInfo->index_name_end = CPM_INT_UNDEFINED;
	nameInfo->index_separator = CPM_INT_UNDEFINED;
	nameInfo->index_extension_start = CPM_INT_UNDEFINED;
	nameInfo->index_extension_end = CPM_INT_UNDEFINED;
	nameInfo->index_terminator = CPM_INT_UNDEFINED;
}

/*
	# Get Name Segment Indices

	This function will get the indices for each segment of a given name.

	## Parameters

	- char *name: pointer to string containing name and extension

	## Returns

	- nameInformationStructType struct containing the indices for each segment of the name

	## Note

	This function is NOT required by this assignment. However, it makes proper flow
	and reuse of this logic much easier.
*/
nameInformationStructType getNameSegmentIndices(char *name) {

	// Make new struct to hold name segment indices
	nameInformationStructType nameInfo;
	initializeNameInformationStruct(&nameInfo);

	// Check up to max 13 bytes (8 for name, 1 for separator, 3 for extension, 1 for terminator)
	for (int i = 0; i < (EXTENT_FORMAT_BYTELENGTH_NAME + 1 + EXTENT_FORMAT_BYTELENGTH_EXTENSION + 1); i++) {

		// Check for legal character
		if (!checkLegalCharacter(name[i])) {
			nameInfo.flag_invalidCharactersFound = true;
			break;
		}

		// Name segment discovery
		if (!nameInfo.flag_nameSegmentFound) {

			// Check for start of name segment
			if (nameInfo.index_name_start == CPM_INT_UNDEFINED) {

				nameInfo.index_name_start = i;
				continue;

			}

			// Check for terminator, indicating only a name segment without separator / extension
			if (name[i] == '\0') {

				nameInfo.flag_nameSegmentFound = true;
				nameInfo.flag_terminatorFound = true;
				nameInfo.index_name_end = i - 1;
				nameInfo.index_terminator = i;
				break;

			}

			// Check for separator, indicating we have a file extension segment to find too
			if (name[i] == '.') {

				nameInfo.flag_nameSegmentFound = true;
				nameInfo.flag_separatorFound = true;
				nameInfo.index_name_end = i - 1;
				nameInfo.index_separator = i;
				continue;

			}

			// If we are still looking for name segment but have maxxed bytes (i = 8, byte = 9), this is illegal
			if (i == EXTENT_FORMAT_BYTELENGTH_NAME) {

				nameInfo.flag_invalidFormatFound = true;
				break;

			}

		}

		// Extension segment discovery
		if (nameInfo.flag_nameSegmentFound && nameInfo.flag_separatorFound && !nameInfo.flag_extensionSegmentFound && !nameInfo.flag_terminatorFound) {

			// Check for terminator, indicating we have found the end of the extension segment
			if (name[i] == '\0') {

				// Check whether we actually had extension or not
				if (nameInfo.index_extension_start != CPM_INT_UNDEFINED) {
					nameInfo.flag_extensionSegmentFound = true;
					nameInfo.index_extension_end = i - 1;
				}

				nameInfo.flag_terminatorFound = true;
				nameInfo.index_terminator = i;
				break;

			}

			// Check for start of extension segment
			if (nameInfo.index_extension_start == CPM_INT_UNDEFINED) {

				nameInfo.index_extension_start = i;
				continue;

			}

			// If we are still looking for extension segment but have maxxed bytes (i = 12, byte = 13), this is illegal
			if (i == (EXTENT_FORMAT_BYTELENGTH_NAME + 1 + EXTENT_FORMAT_BYTELENGTH_EXTENSION)) {

				nameInfo.flag_invalidFormatFound = true;
				break;

			}
		}
	}
	return nameInfo;
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

	// Try to get name segment indices
	nameInformationStructType nameInfo = getNameSegmentIndices(name);

	// Check if any illegal characters were found
	if (nameInfo.flag_invalidCharactersFound) {
		return false;
	}

	// Check if any illegal format was found
	if (nameInfo.flag_invalidFormatFound) {
		return false;
	}

	// Otherwise, name should be legal, so return true
	return true;

}

/*
	# Find Extent With Name

	This function will find the extent number for a given file name if valid.

	## Parameters

	- char *name: pointer to string containing full file name
	- uint8_t *block0: pointer to block 0 of disk (dir)

	## Returns

	- (-1) for illegal name or name not found
	- (0-31) for extent number where file found in dir

*/
int findExtentWithName(char *name, uint8_t *block0) {

	// Check for illegal name
	if (!checkLegalName(name)) {
		return -1;
	}

	// Get segment information for name
	char input_segment_name[EXTENT_FORMAT_BYTELENGTH_NAME + 1];
	char input_segment_extension[EXTENT_FORMAT_BYTELENGTH_EXTENSION + 1];
	nameInformationStructType nameInfo = getNameSegmentIndices(name);

	// Copy over name segment into buffer based on indices
	if (nameInfo.flag_nameSegmentFound) {
		int currentIndex = nameInfo.index_name_start;
		for (int i = 0; i < EXTENT_FORMAT_BYTELENGTH_NAME; i++) {
			if (currentIndex > nameInfo.index_name_end) {
				input_segment_name[i] = ' ';
			} else {
				input_segment_name[i] = name[currentIndex];
				currentIndex++;
			}
		}
		input_segment_name[EXTENT_FORMAT_BYTELENGTH_NAME] = '\0';
	}

	// Copy over extension segment into buffer based on indices
	if (nameInfo.flag_extensionSegmentFound) {
		int currentIndex = nameInfo.index_extension_start;
		for (int i = 0; i < EXTENT_FORMAT_BYTELENGTH_EXTENSION; i++) {
			if (currentIndex > nameInfo.index_extension_end) {
				input_segment_extension[i] = ' ';
			} else {
				input_segment_extension[i] = name[currentIndex];
				currentIndex++;
			}
		}
		input_segment_extension[EXTENT_FORMAT_BYTELENGTH_EXTENSION] = '\0';
	}

	// Go through extents and mark blocks that are used
	int matchedExtentIndex = CPM_INT_UNDEFINED;
	for (int extentIndex = 0; extentIndex < (BLOCK_SIZE / EXTENT_SIZE); extentIndex++) {

		// Make a directory struct for extent
		DirStructType *ptr_dirStruct = mkDirStruct(extentIndex, block0);

		// Check status byte to see if unused (0xe5) or used (0-15 for user)
		if (ptr_dirStruct->status != 0xe5) {

			// Make string for just the name segment
			char extent_segment_name[EXTENT_FORMAT_BYTELENGTH_NAME + 1];
			char extent_segment_extension[EXTENT_FORMAT_BYTELENGTH_EXTENSION + 1];

			// Add in name from extent
			for (int i = 0; i < EXTENT_FORMAT_BYTELENGTH_NAME; i++) {
				char c = ptr_dirStruct->name[i];
				if (c != ' ' && c != '\0') {
					extent_segment_name[i] = c;
				} 
				else {
					extent_segment_name[i] = ' ';
				}
			}
			extent_segment_name[EXTENT_FORMAT_BYTELENGTH_NAME] = '\0';

			// Add in extension from extent
			for (int i = 0; i < EXTENT_FORMAT_BYTELENGTH_EXTENSION; i++) {
				char c = ptr_dirStruct->extension[i];
				if (c != ' ' && c != '\0') {
					extent_segment_extension[i] = c;
				}
				else {
					extent_segment_extension[i] = ' ';
				}
			}
			extent_segment_extension[EXTENT_FORMAT_BYTELENGTH_EXTENSION] = '\0';

			// Check if name and extension segments match input name
			if (strcmp(input_segment_name, extent_segment_name) == 0 && strcmp(input_segment_extension, extent_segment_extension) == 0) {
				matchedExtentIndex = extentIndex;
			}
		}

		// Free mem for dir struct
		free(ptr_dirStruct);

		// If we found match, stop looking
		if (matchedExtentIndex != CPM_INT_UNDEFINED) {
			break;
		}
	}

	// Return val based on if we found it or not
	if (matchedExtentIndex != CPM_INT_UNDEFINED) {
		return matchedExtentIndex;
	} else {
		return -1;
	}
}

/*
	# CPM Delete

	This will delete the fiel and free disk blocks in the free list

	## Parameters

	- char *name: file name to delete

	## Returns

	- (0) if the file was found and deleted
	- (-1) if the name was illegal or file was not found
*/
int cpmDelete(char *name) {

	// Check for illegal name
	if (!checkLegalName(name)) {
		return -1;
	}

	// Load first block from disk as directory into buffer
	uint8_t dirBuffer[BLOCK_SIZE];
	blockRead(dirBuffer, 0);

	// Find extent with the name (or return for not found)
	int extentIndex = findExtentWithName(name, dirBuffer);
	if (extentIndex == -1) {
		return -1;
	}

	// Clear all of the blocks for this extent in free list
	DirStructType *ptr_dirStruct = mkDirStruct(extentIndex, dirBuffer);
	for (int blockIndex = 0; blockIndex < BLOCKS_PER_EXTENT; blockIndex++) {

		// Get block number from extent
		uint8_t blockByteValue = ptr_dirStruct->blocks[blockIndex];

		// If block number is set, then mark block as free
		if (blockByteValue != 0) {
			global_freeList[blockByteValue] = true;
		}
	}
	free(ptr_dirStruct);

	// Write an empty struct to the directory with only status
	DirStructType *emptyDirStruct = malloc(sizeof(DirStructType));
	emptyDirStruct->status = 0xe5;
	writeDirStruct(emptyDirStruct, extentIndex, dirBuffer);
	free(emptyDirStruct);

	// Update directory (block 0)
	blockWrite(dirBuffer, 0);

	// Return success
	return 0;

}

/*
	# CPM Rename

	This function will modify an extent to rename a given file.

	## Parameters

	- char *oldName: the current name of the file to find and rename
	- char *newName: the new name for the file

	## Returns

	- (0) if the file was found and renamed successfully
	- (-1) if either name are illegal or the old file name could not be found
*/
int cpmRename(char *oldName, char *newName) {

	// Check for illegal names
	if (!checkLegalName(oldName) || !checkLegalName(newName)) {
		return -1;
	}

	// Load first block from disk as directory into buffer
	uint8_t dirBuffer[BLOCK_SIZE];
	blockRead(dirBuffer, 0);

	// Find extent with the name (or return for not found)
	int extentIndex = findExtentWithName(oldName, dirBuffer);
	if (extentIndex == -1) {
		return -1;
	}

	// Get segment information for new name
	nameInformationStructType newNameInfo = getNameSegmentIndices(newName);

	// Calculate sizes of name and extension
	int new_nameSegmentSize = newNameInfo.index_name_end - newNameInfo.index_name_start + 1;
	int new_extensionSegmentSize = newNameInfo.index_extension_end - newNameInfo.index_extension_start + 1;

	// Get struct for the found extent
	DirStructType *ptr_dirStruct = mkDirStruct(extentIndex, dirBuffer);
	
	// Clear name + extension in extent
	memset(ptr_dirStruct->name, ' ', EXTENT_FORMAT_BYTELENGTH_NAME);
	memset(ptr_dirStruct->extension, ' ', EXTENT_FORMAT_BYTELENGTH_EXTENSION);

	// Copy over name + extension to extent
	if (newNameInfo.flag_nameSegmentFound) {
		memcpy(&ptr_dirStruct->name, &newName[newNameInfo.index_name_start], new_nameSegmentSize);
	}
	if (newNameInfo.flag_extensionSegmentFound) {
		memcpy(&ptr_dirStruct->extension, &newName[newNameInfo.index_extension_start], new_extensionSegmentSize);
	}

	// Write updated struct back to directory buffer
	writeDirStruct(ptr_dirStruct, extentIndex, dirBuffer);

	// Free memory for dir struct
	free(ptr_dirStruct);

	// Update directory (block 0)
	blockWrite(dirBuffer, 0);

	// Return success
	return 0;

}