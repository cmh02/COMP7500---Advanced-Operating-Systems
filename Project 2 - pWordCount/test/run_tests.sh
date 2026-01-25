#!/bin/bash

: '
	--------------------------------------------------

	# Information

	File: run_tests.sh
	Project: Project 2 - pWordCount
	Author: Chris Hinkson @cmh02

	--------------------------------------------------

	## Description

	Main test script to run all test cases for pWordCount.

	To run this, simply run: ./run_test.sh from the test directory.

	--------------------------------------------------

	## References
	
	1. https://www.geeksforgeeks.org/linux-unix/wc-command-linux-examples/
	-> I used this GeeksForGeeks article to learn about using wc to count words in bash.

	2. https://www.w3schools.com/bash/bash_variables.php
	-> I used this w3schools article to learn about making bash variables.

	--------------------------------------------------
'

# Make two counters for pass/fail
testCasesPassed=0
testCasesFailed=0

# Iterate over all test files in the testfiles directory
for test_file in testfiles/*.txt; do
	
	# Begin test
	echo "Running test with file: $test_file"

	# Get expected value using wc
	expected=$(wc -w < "$test_file")

	# Get actual system value with pWordCount
	actual=$(../build/pwordcount ../test/"$test_file" 1 | grep -o '[0-9]\+$')
	
	# Compare expected and actual values to pass/fail
	if [ "$expected" -eq "$actual" ]; then
		echo "[PASS] Test case $test_file has passed! Expected: $expected, Got: $actual"
		testCasesPassed=$((testCasesPassed + 1))
	else
		echo "[FAIL] Test case $test_file has failed! Expected: $expected, Got: $actual"
		testCasesFailed=$((testCasesFailed + 1))
	fi
	echo "--------------------------"

done

# Print summary of test results
echo "Test Summary:"
echo "Total Test Cases: $((testCasesPassed + testCasesFailed))"
echo "Passed: $testCasesPassed"
echo "Failed: $testCasesFailed"
echo "--------------------------"