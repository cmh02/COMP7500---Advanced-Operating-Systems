#[[
	--------------------------------------------------

	# Information

	File: CMakeLists.txt
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file is used to build the pWordCount project with cmake.

	--------------------------------------------------

	References:

	1. https://www.geeksforgeeks.org/linux-unix/how-to-use-cmake-for-linux/
	-> Basic creation of a CMakeLists.txt file for the project

	2. https://stackoverflow.com/questions/20804468/cmake-multiple-line-comments-block-comment
	-> How to make multiline comments (this one) in CMakeLists.txt

	--------------------------------------------------
]]

cmake_minimum_required(VERSION 3.10)

project(pwordcount C)

add_executable(pwordcount
    pwordcount.c
    pwc_utils.c
	pwc_reader.c
	pwc_counter.c
	pwc_counterManager.c
	pwc_logger.c
)