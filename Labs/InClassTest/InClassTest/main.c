#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "stats.h"

#define ASSIGNMENT_WEIGHT 0.4
#define EXAM_WEIGHT 0.6

struct Student {
    char* name;
    int id;
    float assignmentGrade;
    float examGrade;
    float finalGrade;
};

int getStudentInput(struct Student* student);
void calculateFinalGrade(struct Student* student);
void displayStudentInfo(const struct Student student);


int main() {
    struct Student s;
    if (getStudentInput(&s)) {
        fputs("Error getting input.", stderr);
        return EXIT_FAILURE;
    }

    calculateFinalGrade(&s);

    displayStudentInfo(s);

    return EXIT_SUCCESS;
}

int getStudentInput(struct Student* s) {
    s->name = "Thomas Boland";
    s->id = 556991;
    s->assignmentGrade = 0.8;
    s->examGrade = 0.6;

    return 0;
}

void calculateFinalGrade(struct Student* s) {
    s->finalGrade = ASSIGNMENT_WEIGHT * s->assignmentGrade + EXAM_WEIGHT * s->examGrade;
}

void displayStudentInfo(const struct Student s) {
    printf("Name: %-20s | ID: %08d | Final Grade: %3.1f%%\n", s.name, s.id, s.finalGrade * 100);
}

void sortDoubles(double* arr, int n_elements) {
    // selection sort
    double swap;
    int min_j;
    // keep track of sorted portion of array
    for (int i = 0; i < n_elements; i++) {
        // find lowest value in unsorted portion
        min_j = i;
        for (int j = i; j < n_elements; j++) {
            if (arr[j] < arr[min_j]) {
                min_j = j;
            }
        }

        // swap lowest value with first unsorted value
        swap = arr[i];
        arr[i] = arr[min_j];
        arr[min_j] = swap;
    }
}

void maxPerShelfOverTime(int months, int floors, int rows, int columns, int* inventory, int* maximums) {
    for (int m = 0; m < months; m++) {
        for (int f = 0; f < floors; f++) {
            for (int r = 0; r < rows; r++) {
                for (int c = 0; c < columns; c++) {
                    // inventory[m][f][r][c] --> inventory[m * floors * rows * columns + f * rows * columns + r * columns * c]
                    // maximum[f][r][c] --> maximums[f * rows * columns + r * columns + c]

                    if (inventory[m * floors * rows * columns + f * rows * columns + r * columns * c] > maximums[f * rows * columns + r * columns + c]) {
                        maximums[f * rows * columns + r * columns + c] = inventory[m * floors * rows * columns + f * rows * columns + r * columns * c];
                    }
                }
            }
        }
    }
}