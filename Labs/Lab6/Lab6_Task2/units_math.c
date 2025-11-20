/*
* units_math.c
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
*
* Contains function implementations for parsing and for arithmetic operations.
*/

#include <stdbool.h>
#include <string.h>
#include "units_math.h"

// 0 for success, non-zero for failure
int _siValueAppendUnit(SiValue* value, const char* unit, int power) {
	SiUnitLinkedListNode* newNode = (SiUnitLinkedListNode*)malloc(sizeof(SiUnitLinkedListNode));
	if (newNode == NULL) {
		return -1;
	}
	
	//copy unit string
	newNode->unit = (char*)malloc(strlen(unit) + 1);
	if (newNode->unit == NULL) {
		free(newNode);
		return -1;
	}

	strcpy_s(newNode->unit, strlen(unit) + 1, unit);

	newNode->power = power;
	newNode->next = NULL;
	if (value->units.head == NULL) {
		value->units.head = newNode;
		value->units.tail = newNode;
	}
	else {
		value->units.tail->next = newNode;
		value->units.tail = newNode;
	}
}

void _siValueClearUnits(SiValue* value) {
	SiUnitLinkedListNode* current = value->units.head;
	while (current != NULL) {
		SiUnitLinkedListNode* toFree = current;
		current = current->next;
		if (toFree->unit != NULL) {
			free(toFree->unit);
		}
		free(toFree);
	}
	value->units.head = NULL;
	value->units.tail = NULL;
}

bool siValueSameUnit(SiValue a, SiValue b) {
	// Assumes no duplicate units, and units are same case.
	// NOTE does not handle case of same units specified in different orders
	//   possible solutions: -sort both lists of units first
	//                       -for each unitA, check all unitBs. we can assume there are no duplicate errors here.
	// This does handle the case where both values have no units.
	
	// for every unit in linked list a:
	//   check that a.unit and b.unit match and that a.power == b.power
	//   if a or b ends before the other one, then mismatch in number of units.
	SiUnitLinkedListNode* unitA = a.units.head;
	SiUnitLinkedListNode* unitB = b.units.head;
	while (unitA != NULL) {
		if (unitB == NULL) { // more unitAs than unitBs
			return false;
		}

		// unit mismatch
		if (strcmp(unitA->unit, unitB->unit) != 0) {
			return false;
		}

		// power mismatch
		if (unitA->power != unitA->power) {
			return false;
		}

		unitA = unitA->next;
		unitB = unitB->next;
	}
	if (unitB != NULL) { // more unitBs than unitAs
		return false;
	}
	return true;
}

int siValueAdd(SiValue a, SiValue b, SiValue* result) {
	if (!siValueSameUnit(a, b)) {
		return -1;
	}

	result->value = a.value + b.value;
	
	// copy units from a (or b, since they are the same) into result
	result->units.head = NULL;
	result->units.tail = NULL;
	SiUnitLinkedListNode* unitA = a.units.head;
	while (unitA != NULL) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			// TODO: free previously allocated units in result and retunr failure
			_siValueClearUnits(result);
			return -1;
		}
	}

	return 0;
}

// TODO this is the same thing as a + (-b). could refactor to avoid code duplication.
int siValueSubtract(SiValue a, SiValue b, SiValue* result) {
	if (!siValueSameUnit(a, b)) {
		return -1;
	}

	result->value = a.value - b.value;

	// copy units from a (or b, since they are the same) into result
	result->units.head = NULL;
	result->units.tail = NULL;
	SiUnitLinkedListNode* unitA = a.units.head;
	while (unitA != NULL) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			// TODO: free previously allocated units in result and retunr failure
			_siValueClearUnits(result);
			return -1;
		}
	}

	return 0;
}

SiValue siValueMultiply(SiValue a, SiValue b);
SiValue siValueDivide(SiValue a, SiValue b);

int siValueParseFromStream(FILE* stream, SiValue* value) {
	// NOTE: need to handle combining multiple instances of same unit. for example, m s m -> m^2 s
}
int siValueOutputToStream(FILE* stream, SiValue* value);