/*
	File: simple.c
	Project: Project1
	Author: Chris Hinkson [cmh0201@auburn.edu]
	Class: COMP7500 - Advanced Operating Systems
*/

// Package Imports
#include <stdio.h>
#include <math.h>

// Main Execution
int main(void) {

	// Variable Initialization
	double coolNumbers[10] = {1, 7, 9, 21, 23, 67, 100, 2002, 2026, 4096};
	double sumOfSquareRoots = 0.0;
	int coolNumbersLength = sizeof(coolNumbers) / sizeof(coolNumbers[0]);
	
	// Loop through numbers, get the square root of each, then add it to sum
	for (int i=0; i < coolNumbersLength; i++) {
		sumOfSquareRoots += sqrt(coolNumbers[i]);
	}

	// Calculate average and print
	double averageOfSquareRoots = sumOfSquareRoots / coolNumbersLength;
	printf("The average of the square roots of my cool numbers array is: %.2f!\n", averageOfSquareRoots);
	// Main return
	return 0;
}
