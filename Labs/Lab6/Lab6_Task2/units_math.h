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

bool siValueSameUnit(SiValue a, SiValue b);

int siValueAdd(SiValue a, SiValue b, SiValue* result);
int siValueSubtract(SiValue a, SiValue b, SiValue* result);
SiValue siValueMultiply(SiValue a, SiValue b);
SiValue siValueDivide(SiValue a, SiValue b);

int siValueParseFromStream(FILE* stream, SiValue* value);
int siValueOutputToStream(FILE* stream, SiValue* value);