/*
* units_math.h
* 2025F COMP-166 Lab 6 - Thomas Boland - C0556991
* 
* Contains structure for representing values with SI units and
* function prototypes for parsing and for arithmetic operations.
*/

#include <stdio.h>
#include <stdlib.h>

typedef struct SiUnitLinkedListNode {
	char* unit;
	int power;
	struct SiUnitLinkedListNode* next;
} SiUnitLinkedListNode;

typedef struct {
	SiUnitLinkedListNode* head;
	SiUnitLinkedListNode* tail;
} SiUnitLinkedList;

typedef struct {
	double value;
	SiUnitLinkedList units;
} SiValue;

bool siValueSameUnit(const SiValue a, const SiValue b);

int siValueAdd(const SiValue a, const SiValue b, SiValue* result);
int siValueSubtract(const SiValue a, const SiValue b, SiValue* result);
int siValueMultiply(const SiValue a, const SiValue b, SiValue* result);
int siValueDivide(const SiValue a, const SiValue b, SiValue* result);

int siValueParseFromStream(FILE* stream, SiValue* value);
int siValueOutputToStream(FILE* stream, const SiValue* value);