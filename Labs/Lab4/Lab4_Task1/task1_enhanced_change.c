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

/**
* Accepts user input for an amount of money in format "$xxxx.xx" (must be positive). Forces user to retry if
* input is invalid
* @param *success Sets to true if successful or false if unrecoverable error encountered.
* @return Input money value in cents.
*/
int getMoneyInput(bool* success) {

}

/**
* Calculated the quantity of each denomination for the given amount of cents. Reduces the available
* inventory accordingly.
* @param amount Amount of cents to draw change for
* @param *change Array to put calculated change amounts into. Must be same length as inventory.
* @param **inventory 2D Array of inventory of available denominations. Each item = {denom_value, qty}.
* @return Amount of cents that was unfulfilled if inventory was exhausted, or 0.
*/
int drawChangeFromInventory(const int amount, int* change, int** inventory) {

}

void outputChange(const int amount, const int* change, const int** inventory) {

}

#define N_DENOMINATIONS 10

int main() {
	// initialize inventory array and change array
	// inventory array MUST be sorted largest denomination to smallest.
	int inventory[N_DENOMINATIONS][2] = {
		{10000, 0}, // $100 bills
		{ 5000, 0}, // $50 bills
		{ 2000, 0}, // $20 bills
		{ 1000, 0}, // $10 bills
		{  500, 0}, // $5 bills
		{  200, 0}, // toonies
		{  100, 0}, // loonies
		{   25, 0}, // quarters
		{   10, 0}, // dimes
		{    1, 0}, // pennies
	};
	int change[N_DENOMINATIONS];
	int unfulfiled = 0;
	while (unfulfiled == 0) {
		// Get user input in format "$xxx.xx" and return integer number of cents.
		// calculate change, drawing from inventory (modifies this). returns 0 or unfulfilled amount of cents
		// output table:
		// Denomination		Inventory		Change
		// ---------------------------------------
		//                  Total			Total
		// [Not enough change. Short by $%d.%02d.]
	}
	return EXIT_SUCCESS;
}