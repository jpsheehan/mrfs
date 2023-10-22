#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "Base64.h"

// allows for O(1) time encoder lookups
static char *b64_lookup = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

size_t Base64EncodedLength(size_t n)
{
	size_t size = (size_t)ceilf(n / 3.0f) * 4;
	return size;
}

size_t Base64DecodedLength(const char *ascii)
{
	size_t n = strlen(ascii);
	if (n % 4 != 0) {
		return 0;
	}

	size_t partial = (n * 6) / 8;

	if (ascii[n - 1] == '=') {
		partial--;
		if (ascii[n - 2] == '=') {
			partial--;
		}
	}

	return partial;
}

int Base64Encode(const uint8_t *bytes, size_t n, char *ascii)
{
	size_t i;
	uint32_t tmp;

	// do al whole blocks
	for (i = 0; i < n / 3; i++) {
		tmp = (bytes[i * 3] << 24) | (bytes[i * 3 + 1] << 16) | (bytes[i * 3 + 2] << 8);
		ascii[i * 4 + 0] = b64_lookup[(tmp >> 26) & 0x3f];
		ascii[i * 4 + 1] = b64_lookup[(tmp >> 20) & 0x3f];
		ascii[i * 4 + 2] = b64_lookup[(tmp >> 14) & 0x3f];
		ascii[i * 4 + 3] = b64_lookup[(tmp >>  8) & 0x3f];
	}

	// do remainder and add padding
	switch (n % 3) {
	case 0:
		ascii[i * 4 + 0] = '\0'; // null terminator
		break;
	case 1:
		// printf("1 remainder\n");
		ascii[i * 4 + 0] = b64_lookup[(bytes[i * 3 + 0] >> 2) & 0x3f];
		ascii[i * 4 + 1] = b64_lookup[(bytes[i * 3 + 0] & 0x3) << 6];
		ascii[i * 4 + 2] = '=';
		ascii[i * 4 + 3] = '=';
		ascii[i * 4 + 4] = '\0'; // null terminator
		break;
	case 2:
		// printf("2 remainder\n");
		ascii[i * 4 + 0] = b64_lookup[(bytes[i * 3 + 0] >> 2) & 0x3f];
		ascii[i * 4 + 1] = b64_lookup[ ((bytes[i * 3 + 0] & 0x3) << 4) | ((bytes[i * 3 + 1] >> 4) & 0xf) ];
		ascii[i * 4 + 2] = b64_lookup[ ((bytes[i * 3 + 1] & 0xf) << 2) ];
		ascii[i * 4 + 3] = '=';
		ascii[i * 4 + 4] = '\0'; // null terminator
		break;
	}

	return 0;
}

#define b64_lu(c) (uint8_t)(strchr(b64_lookup, c) - b64_lookup)

int Base64Decode(const char *ascii, uint8_t *bytes)
{
	// TODO: Change this from a linear search, use a lookup based approach
	size_t i = 0;
	uint8_t offsets[4] = { 0 };

	for (i = 0; i * 4 < strlen(ascii); ++i) {
		offsets[0] = strchr(b64_lookup, ascii[i * 4 + 0]) - b64_lookup;
		offsets[1] = strchr(b64_lookup, ascii[i * 4 + 1]) - b64_lookup;
		offsets[2] = strchr(b64_lookup, ascii[i * 4 + 2]) - b64_lookup;
		offsets[3] = strchr(b64_lookup, ascii[i * 4 + 3]) - b64_lookup;

		// always construct the first byte in the same way
		bytes[i * 3 + 0] = ((offsets[0]) << 2) | ((offsets[1] >> 4) & 0x3);

		if (offsets[3] >= 64) { // check if the last character does not exist in the LUT (i.e. is a '=')
			if (offsets[2] < 64) { // check if the second-to-last character is not a '='
				// there is only one padding character, we have two output bytes
				bytes[i * 3 + 1] = ( (b64_lu(ascii[i * 4 + 1]) << 4) & 0xf0) | ( (b64_lu(ascii[i * 4 + 2]) >> 2) & 0x0f );
			}
		} else {
			// there are no padding characters, we have all three output bytes
			bytes[i * 3 + 1] = ( (b64_lu(ascii[i * 4 + 1]) << 4) & 0xf0) | ((b64_lu(ascii[i * 4 + 2]) >> 2) & 0x0f );
			bytes[i * 3 + 2] = ( (b64_lu(ascii[i * 4 + 2]) << 6) & 0xf3) | ((b64_lu(ascii[i * 4 + 3]) >> 0) & 0x3f );
		}
	}
	return 0;
}


int Base64EncodeFile(const char *in_filename, const char *out_filename)
{
	FILE *infile = fopen(in_filename, "rb");
	FILE *outfile = fopen(out_filename, "w");

	uint8_t byte_buffer[BASE64_BYTE_BUFFER_SIZE];
	char	char_buffer[BASE64_CHAR_BUFFER_SIZE];
	size_t	bytes_read;

	while (!feof(infile))
	{
		memset(byte_buffer, 0, BASE64_BYTE_BUFFER_SIZE);
		memset(char_buffer, '\0', BASE64_CHAR_BUFFER_SIZE);
		
		bytes_read = fread(byte_buffer, sizeof(uint8_t), BASE64_BYTE_BUFFER_SIZE, infile);

		Base64Encode(byte_buffer, bytes_read, char_buffer);

		fwrite(char_buffer, sizeof(char), strlen(char_buffer), outfile);
	}

	fclose(outfile);
	fclose(infile);

	return 0;
}


int Base64DecodeFile(const char *in_filename, const char *out_filename)
{
	FILE *infile = fopen(in_filename, "r");
	FILE *outfile = fopen(out_filename, "wb");

	uint8_t byte_buffer[BASE64_BYTE_BUFFER_SIZE];
	char	char_buffer[BASE64_CHAR_BUFFER_SIZE];
	size_t	chars_read;
	size_t	bytes_decoded;

	while (!feof(infile))
	{
		memset(byte_buffer, 0, BASE64_BYTE_BUFFER_SIZE);
		memset(char_buffer, '\0', BASE64_CHAR_BUFFER_SIZE);

		chars_read = fread(char_buffer, sizeof(char), BASE64_CHAR_BUFFER_SIZE, infile);
		
		if (chars_read < BASE64_CHAR_BUFFER_SIZE)
		{
			char_buffer[chars_read] = '\0';
			bytes_decoded = Base64DecodedLength(char_buffer);
		}
		else
		{
			bytes_decoded = BASE64_BYTE_BUFFER_SIZE;
		}

		Base64Decode(char_buffer, byte_buffer);

		fwrite(byte_buffer, sizeof(uint8_t), bytes_decoded, outfile);
	}

	fclose(outfile);
	fclose(infile);

	return 0;
}
