/*
	--------------------------------------------------

	# Information

	File: aubatch_cmdparser.h
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	This file provides the command parser module for AUbatch.

	--------------------------------------------------

	## References
	
	1. 

	--------------------------------------------------
*/

// Prevent multiple inclusions
#ifndef AUBATCH_CMDPARSER_H
#define AUBATCH_CMDPARSER_H

// Libraries

// Project Libraries
#include "aubatch.h"
#include "aubatch_utils.h"

/*
	# Command Parser Command Loop

	Enter the command loop for the command parser.

	## Arguments
	- None

	## Returns
	- void
*/
int aubatch_cmdparser_enterCommandLoop();


#endif // AUBATCH_CMDPARSER_H