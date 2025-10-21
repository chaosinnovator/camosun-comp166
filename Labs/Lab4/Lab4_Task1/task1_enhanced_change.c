/*
* 2025F COMP-166 Lab 4 - Thomas Boland - C0556991
* 
* This program converts a given amount of money to the optimal
* quantity of each denomination, drawing from a limited inventory
* of cash.
* 
*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>

/**
* Accepts user input for an amount of money in format "$xxxx.xx" (must be positive). Forces user to retry if
* input is invalid. Lab instructions require this function to return the value directly, so this function takes
* a pointer a success flag instead of returning success.
* @param *success Sets to true if successful or false if unrecoverable error encountered.
* @return Input money value in cents.
*/
int getMoneyInput(bool* success) {
	char input_buf[128];
	long dollar_input_value;
	long cents_input_value;
	(*success) = false;
	while (!(*success)) {
		printf("Enter amount of cash in dollars and cents (like 10.23): $");

		// get up to 128 characters of input or up to newline
		// fgets returns NULL in the event reading input failed
		if (fgets(input_buf, 128, stdin) == NULL) {
			// Failed to read input
			return -1;
		}

		// convert dollar amount up to first '.' character
		char* first_unconverted_character = 0;
		errno = 0; // I'm not a fan of the errno pattern, but strtol uses it.
		dollar_input_value = strtol(input_buf, &first_unconverted_character, 10); // radix (aka base) = 10

		// check that dollar amount converted properly
		// possible cases:
		//   first_unconverted_character pointer is equal to input_buffer (pointer), so nothing was converted
		//   first_unconverted_character pointer is set, but it isn't '.' so something else was encountered that couldn't be converted

		if (first_unconverted_character == input_buf ||
		    (*first_unconverted_character && *first_unconverted_character != '.')) {
			puts("Unable to properly convert input. Please try again.");
			continue;
		}

		// dollars has to be less than int_max/100 since there needs to be room to store total number of cents in an int.
		if (errno == ERANGE || dollar_input_value > (INT_MAX / 100) || dollar_input_value < 0) {
			printf("Dollar amount must be a positive integer less than or equal to %d. Please try again.\n", INT_MAX / 100);
			continue;
		}

		// convert cents amount starting from first_unconverted_character + 1
		char* second_unconverted_character = 0;
		errno = 0;
		cents_input_value = strtol(first_unconverted_character + 1, &second_unconverted_character, 10); // base = 10

		// check that dollar amount converted properly
		// possible cases:
		//   second_unconverted_character pointer is equal to input_buffer (pointer), so nothing was converted
		//   second_unconverted_character pointer is set, but it isn't '\n' so something else was encountered that couldn't be converted

		if (second_unconverted_character == input_buf ||
			(*second_unconverted_character && *second_unconverted_character != '\n')) {
			puts("Unable to properly convert input. Please try again.");
			continue;
		}

		// cents has to be less than 100 since there needs to be room to store total number of cents in an int.
		if (errno == ERANGE || cents_input_value >= 100 || cents_input_value < 0) {
			printf("Cents amount must be a positive integer less than 100. Please try again.\n");
			continue;
		}

		(*success) = 1;
	}

	return (int)(dollar_input_value * 100 + cents_input_value);
}

/**
* Calculated the quantity of each denomination for the given amount of cents. Reduces the available
* inventory accordingly.
* @param amount Amount of cents to draw change for.
* @param *change Array to put calculated change quantities into. Must be same length as inventory.
* @param **inventory 2D Array of inventory of available denominations. Each item = {denom_value, qty}.
* @param n_denominations length of change and inventory arrays.
* @return Amount of cents that was unfulfilled if inventory was exhausted, or 0.
*/
int drawChangeFromInventory(int amount, int* change, int inventory[][2], int n_denominations) {
	// for each entry in inventory
	int denom_qty = 0;
	for (int d = 0; d < n_denominations; d++) {
		//   quantity needed to fit amount?
		denom_qty = amount / inventory[d][0];
		//   min out of quantity, available_quantity
		denom_qty = denom_qty < inventory[d][1] ? denom_qty : inventory[d][1];
		//   change[i] = quantity
		change[d] = denom_qty;
		//   inventory[i][1] -= quantity
		inventory[d][1] -= denom_qty;
		//   amount -= denom*quantity
		amount -= inventory[d][0] * denom_qty;
		//   don't use if amount == 0 break because change array needs to either be initialized or finish
		//   being reset to 0 for remaining denoms.
	}
	// return amount
	return amount;
}

void outputChange(const int amount, const int* change, const int inventory[][2], int n_denominations) {
	// Denomination		Inventory		Change
	// ---------------------------------------
	// ---------------------------------------
	//                  Total			Total
	printf("Denomination		Inventory		Change\n");
	printf("------------------------------------------\n");
	int inv_total = 0;
	int change_total = 0;
	for (int d = 0; d < n_denominations; d++) {
		printf("$%d.%02d\t\t%d\t\t%d\n", inventory[d][0] / 100, inventory[d][0] % 100, inventory[d][1], change[d]);
		inv_total += inventory[d][0] * inventory[d][1];
		change_total += inventory[d][0] * change[d];
	}
	printf("------------------------------------------\n");
	printf("\t\t$%d.%02d\t\t$%d.%02d\n", inv_total / 100, inv_total % 100, change_total / 100, change_total % 100);
}

#define N_DENOMINATIONS 11

int main() {
	// initialize inventory array and change array
	// inventory array MUST be sorted largest denomination to smallest.
	int inventory[N_DENOMINATIONS][2] = {
		{10000, 5}, // $100 bills
		{ 5000, 10}, // $50 bills
		{ 2000, 10}, // $20 bills
		{ 1000, 10}, // $10 bills
		{  500, 15}, // $5 bills
		{  200, 15}, // toonies
		{  100, 15}, // loonies
		{   25, 20}, // quarters
		{   10, 20}, // dimes
		{    5, 20}, // nickels
		{    1, 20}, // pennies
	};
	int change[N_DENOMINATIONS];

	// enter program loop
	int unfulfiled = 0;
	while (unfulfiled == 0) {
		// Get user input in format "$xxx.xx" and return integer number of cents.
		bool success;
		int input_cents = getMoneyInput(&success);
		if (!success) {
			fputs("Failed to get input.", stderr);
			return EXIT_FAILURE;
		}

		// calculate change, drawing from inventory (modifies this). returns 0 or unfulfilled amount of cents
		unfulfiled = drawChangeFromInventory(input_cents, change, inventory, N_DENOMINATIONS);

		// output table:
		outputChange(input_cents, change, inventory, N_DENOMINATIONS);
	}

	// Not enough change
	printf("Not enough change. Short by $%01d.%02d.", unfulfiled / 100, unfulfiled % 100);
	return EXIT_SUCCESS;
}