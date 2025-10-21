/*
* 2025F COMP-166 Lab 3 - Thomas Boland - C0556991
* 
* This program calculates the compound interest given an initial investment over DISPLAY_YEARS years.
* 
*/

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#define COMPOUND_PER_YEAR 12
#define INTEREST_RATE 0.025
#define DISPLAY_YEARS 5
#define CENTS_PER_DOLLAR 100

int main() {
    double realRate = 1 + INTEREST_RATE / COMPOUND_PER_YEAR;
    long balance[DISPLAY_YEARS + 1];

    printf("Annual interest rate: %.2f%%\n", INTEREST_RATE * 100);
    printf("Interest compounded %d times yearly\n", COMPOUND_PER_YEAR);

    // runs forever
    char input_buf[128];
    while (true) {
        // user keeps entering value until valid value is entered.
        while (true) {
            printf("Initial Investment: $");

            // get up to 128 characters of input or up to newline
            // fgets returns NULL in the event reading input failed
            if (fgets(input_buf, 128, stdin) == NULL) {
                // Failed to read input
                return -1;
            }

            char* first_unconverted_character;
            errno = 0; // I'm not a fan of the errno pattern, but strtol uses it.
            balance[0] = strtol(input_buf, &first_unconverted_character, 10); // radix (aka base) = 10

            // possible cases:
            //   first_unconverted_character pointer is equal to input_buffer (pointer), so nothing was converted
            //   first_unconverted_character pointer is set, but it isn't \n so something else was encountered that couldn't be converted
            //   errno is set to ERANGE if number is not in range for long
            //   number is not in range to cast to int
            //   number is negative

            if (first_unconverted_character == input_buf ||
                (*first_unconverted_character && *first_unconverted_character != '\n')) {
                puts("Unable to properly convert input. Please try again.");
                continue;
            }

            if (errno == ERANGE || balance[0] < 0) {
                printf("Input must be a positive integer less than or equal to %d. Please try again.\n", LONG_MAX);
                continue;
            }

            // success
            balance[0] *= CENTS_PER_DOLLAR;
            break;
        }
        
        // both for loops had same iteration scheme and the second loop did not depend
        //   on subsequent iterations, so I combined the for loops.
        // Simplification opportunity: no need for a array of balance each year since the
        //  array is never used except for printing in order of calculation, so long balance
        //  could be calculated cumulatively/in-place.
        for (int year = 1; year <= DISPLAY_YEARS; year++) {
            balance[year] = balance[year - 1];
            for (int period = 0; period < COMPOUND_PER_YEAR; period++) {
                balance[year] = lround(balance[year] * realRate);
            }

            printf(
                "year %d:  $%ld.%02ld\n",
                year,
                balance[year] / CENTS_PER_DOLLAR,
                balance[year] % CENTS_PER_DOLLAR
            );
        }
    }

    return 0;
}