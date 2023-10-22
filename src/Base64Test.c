#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Base64.h"

static char STRING1[] = "Man is distinguished, not only by his reason, but by this singular passion from other animals, " \
                        "which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable " \
                        "generation of knowledge, exceeds the short vehemence of any carnal pleasure.";

int main(int argc, char* argv[])
{
	bool ok = true;
	char* string1_enc = NULL;
	size_t string1_enc_len = 0;
	char *string1_dec =  NULL;
	char *input_string = NULL;

	if (argc == 2) {
		input_string = argv[1];
	} else {
		input_string = STRING1;
	}

	string1_enc_len = Base64EncodedLength(strlen(input_string));
	string1_dec = calloc(strlen(input_string) + 1, sizeof(char));

	printf("Length of input string: %lu, length of output string: %lu\n", strlen(input_string), string1_enc_len);

	string1_enc = calloc(string1_enc_len + 1, sizeof(char));

	if (string1_enc) {

		Base64Encode((uint8_t*)input_string, strlen(input_string), string1_enc);
		Base64Decode(string1_enc, (uint8_t*)string1_dec);

		printf("Original String:\n\"%s\"\n\n", input_string);
		printf("Base64 encoded string:\n\"%s\"\n\n", string1_enc);
		printf("Base64 decoded string:\n\"%s\"\n\n", string1_dec);

		if (strcmp(input_string, string1_dec) == 0) {
			printf("Input string equals output string.\n");
			ok = true;
		} else {
			printf("Input string DOES NOT equal output string.\n");
			printf("strlen(input) = %lu, strlen(output) = %lu\n", strlen(input_string), strlen(string1_dec));
			ok = false;
		}
		free(string1_enc);
		free(string1_dec);
	} else {
		printf("Could not allocate space for string1_enc\n");
	}

	return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
