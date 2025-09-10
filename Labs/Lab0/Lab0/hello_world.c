// 2025F COMP-166 Lab 0 - Thomas Boland - C0556991
#include <stdio.h>

int main() {
	puts("Hello world!");

	// Also output:
	// 1. full name
	// 2. reason for taking this course
	// 3. 2-3 things hope to learn in this class
	const char* name = "Thomas Boland";
	const char* reason =
		"I am taking this course as part of the Engineering Transfer program with a goal of studying "
		"Mechanical Engineering.";
	const char* objective =
		"Since most of my programming experience is with higher-level languages, I hope to learn more "
		"details about memory management and use of pointers.";
	printf("My name is: %s\n", name);
	puts("Reason for taking this course:");
	puts(reason);
	puts("2-3 things I hope to learn in this class:");
	puts(objective);
}