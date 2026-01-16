/*
	File: simple.c
	Project: Project1
	Author: Chris Hinkson [cmh0201@auburn.edu]
	Class: COMP7500 - Advanced Operating Systems

	References:

	1. https://www.geeksforgeeks.org/c/c-arrays/
	->  I used the GeeksForGeeks article on C arrays to refamiliarize myself with 
		how to define arrays in C and iterate over them.

	2. https://www.geeksforgeeks.org/c/sqrt-function-in-c/
	-> I used the GeeksForGeeks article on the square root function to refamiliarize
		myself with the proper syntax (function name / signature).

	3. https://www.geeksforgeeks.org/c/printf-in-c/
	-> I used the GeeksForGeeks article on the printf function to refamiliarize myself
		with the proper syntax for printing information and formatting numbers.
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
