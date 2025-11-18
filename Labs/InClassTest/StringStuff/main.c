#include <stdio.h>
#include <stdlib.h>

void strToUpper(char* str);

int main(int argc, char* argv[]) {
	if (argc != 2) {
		puts("Incorrect number of args.");
		return EXIT_FAILURE;
	}

	strToUpper(argv[1]);

	printf("Uppercase string:\n%s\n", argv[1]);

	return EXIT_SUCCESS;
}

/**
* @brief Converts any lowercase characters [a-z] in provided string [A-Z]
* up to null terminator.
* @param str null-terminated string/char array
*/
void strToUpper(char* str) {
	// converts all characters to uppercase. Assume C-string
	// don't need to know str length, just continue until encounter null
	int i = 0;
	while (str[i] != '\0') {
		// if char is not in lowercase range, ignore.
		if (str[i] < 'a' || str[i] > 'z') {
			i++;
			continue;
		}

		// subtract 32 ('a' is 97 and 'A' is 65)
		str[i] -= 32;
		i++;
	}
}