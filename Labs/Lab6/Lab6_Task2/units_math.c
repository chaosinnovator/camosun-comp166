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
	newNode->unit = strdup(unit);
	if (newNode->unit == NULL) {
		free(newNode);
		return -1;
	}

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

void _siValueRemoveZeroPowerUnits(SiValue* value) {
	SiUnitLinkedListNode* previous = NULL;
	for (SiUnitLinkedListNode* current = value->units.head; current != NULL; current = current->next) {
		if (current->power != 0) {
			previous = current;
			continue;
		}

		// remove current
		SiUnitLinkedListNode* to_free = current;
		if (previous == NULL) {
			// removing head
			value->units.head = current->next;
		}
		else {
			// removing middle node
			previous->next = current->next;
		}
		// check tail
		if (value->units.tail == current) {
			value->units.tail = current->next;
		}
		free(to_free);
	}
}

bool siValueSameUnit(const SiValue a, const SiValue b) {
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

int siValueAdd(const SiValue a, const SiValue b, SiValue* result) {
	if (!siValueSameUnit(a, b)) {
		return -1;
	}

	result->value = a.value + b.value;
	result->units.head = NULL;
	result->units.tail = NULL;
	
	// copy units from a (or b, since they are the same) into result
	for (SiUnitLinkedListNode* unitA = a.units.head; unitA != NULL; unitA = unitA->next) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			// TODO: free previously allocated units in result and retunr failure
			_siValueClearUnits(result);
			return -1;
		}
	}

	return 0;
}

// TODO this is the same thing as a + (-b). could refactor to avoid code duplication.
int siValueSubtract(const SiValue a, const SiValue b, SiValue* result) {
	if (!siValueSameUnit(a, b)) {
		return -1;
	}

	result->value = a.value - b.value;
	result->units.head = NULL;
	result->units.tail = NULL;

	// copy units from a (or b, since they are the same) into result
	for (SiUnitLinkedListNode* unitA = a.units.head; unitA != NULL; unitA = unitA->next) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			// TODO: free previously allocated units in result and retunr failure
			_siValueClearUnits(result);
			return -1;
		}
	}

	return 0;
}

int siValueMultiply(const SiValue a, const SiValue b, SiValue* result) {
	result->value = a.value * b.value;
	result->units.head = NULL;
	result->units.tail = NULL;

	// combine units from a and b into result
	// for each unit in a: add to result
	for (SiUnitLinkedListNode* unitA = a.units.head; unitA != NULL; unitA = unitA->next) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			_siValueClearUnits(result);
			return -1;
		}
	}
	// for each unit in b: if exists in result, add powers, else add new unit to result
	for (SiUnitLinkedListNode* unitB = b.units.head; unitB != NULL; unitB = unitB->next) {
		bool unit_found = false;
		for (SiUnitLinkedListNode* unitR = result->units.head; unitR != NULL; unitR = unitR->next) {
			if (strcmp(unitB->unit, unitR->unit) != 0) {
				continue;
			}

			// unit exists in result, add powers
			unitR->power += unitB->power;
			// TODO if power is now zero, remove unit from list
			unit_found = true;
			break;
		}

		if (unit_found) {
			continue;
		}

		// unit from b not found in result, add new unit
		if (_siValueAppendUnit(result, unitB->unit, unitB->power) != 0) {
			_siValueClearUnits(result);
			return -1;
		}
	}

	_siValueRemoveZeroPowerUnits(result);
	return 0;
}

int siValueDivide(const SiValue a, const SiValue b, SiValue* result) {
	if (b.value == 0.0) { // prevent division by zero
		return -1;
	}

	result->value = a.value / b.value;
	result->units.head = NULL;
	result->units.tail = NULL;

	// combine units from a and b into result
	// for each unit in a: add to result
	for (SiUnitLinkedListNode* unitA = a.units.head; unitA != NULL; unitA = unitA->next) {
		if (_siValueAppendUnit(result, unitA->unit, unitA->power) != 0) {
			_siValueClearUnits(result);
			return -1;
		}
	}
	// for each unit in b: if exists in result, subtract powers, else add new unit with negative power to result
	for (SiUnitLinkedListNode* unitB = b.units.head; unitB != NULL; unitB = unitB->next) {
		bool unit_found = false;
		for (SiUnitLinkedListNode* unitR = result->units.head; unitR != NULL; unitR = unitR->next) {
			if (strcmp(unitB->unit, unitR->unit) != 0) {
				continue;
			}

			// unit exists in result, add powers
			unitR->power -= unitB->power;
			unit_found = true;
			break;
		}

		if (unit_found) {
			continue;
		}

		// unit from b not found in result, add new unit
		if (_siValueAppendUnit(result, unitB->unit, unitB->power) != 0) {
			_siValueClearUnits(result);
			return -1;
		}
	}

	_siValueRemoveZeroPowerUnits(result);
	return 0;
}

int siValueParseFromStream(FILE* stream, SiValue* value) {
	// NOTE: need to handle combining multiple instances of same unit. for example, m^2 s m -> m^3 s
	// NOTE: remove units with power 0 after parsing.
	_siValueRemoveZeroPowerUnits(value);
}
int siValueOutputToStream(FILE* stream, const SiValue* value);