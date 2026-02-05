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

	# Iterate over 1 -> 5 counters (n)
	for n in {1..5}; do

		# Get actual system value with pWordCount
		actual=$(../build/pwordcount -f ../test/"$test_file" -n $n | grep -o '[0-9]\+$')
		
		# Compare expected and actual values to pass/fail
		if [ "$expected" -eq "$actual" ]; then
			echo "[PASS] Test case [f = $test_file | n = $n] has passed! Expected: $expected, Got: $actual"
			testCasesPassed=$((testCasesPassed + 1))
		else
			echo "[FAIL] Test case [f = $test_file | n = $n] has failed! Expected: $expected, Got: $actual"
			testCasesFailed=$((testCasesFailed + 1))
		fi
		echo "--------------------------"
	
	done

done

# Print summary of test results
echo "Test Summary:"
echo "Total Test Cases: $((testCasesPassed + testCasesFailed))"
echo "Passed: $testCasesPassed"
echo "Failed: $testCasesFailed"
echo "--------------------------"