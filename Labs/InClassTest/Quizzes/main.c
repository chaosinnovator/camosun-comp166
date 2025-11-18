#include <stdlib.h>

int* changeNum(int* myNum);

int main() {
	int myNum = 6;

	changeNum(&myNum + 2);

	return EXIT_SUCCESS;
}

int* changeNum(int* myNum) {
	*myNum++;
	return myNum + 2;
}