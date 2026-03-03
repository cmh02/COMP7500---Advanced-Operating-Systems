#[[
	--------------------------------------------------

	# Information

	File: CMakeLists.txt
	Project: Project 3 - AUbatch
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	# Description

	This file is used to build the AUbatch project with cmake.

	--------------------------------------------------

	References:

	1. https://www.geeksforgeeks.org/linux-unix/how-to-use-cmake-for-linux/
	-> Basic creation of a CMakeLists.txt file for the project

	2. https://stackoverflow.com/questions/20804468/cmake-multiple-line-comments-block-comment
	-> How to make multiline comments (this one) in CMakeLists.txt

	--------------------------------------------------
]]

cmake_minimum_required(VERSION 3.10)

project(aubatch C)

add_executable(aubatch
    aubatch.c
    aubatch_utils.c
	aubatch_logger.c
	aubatch_config.c
)