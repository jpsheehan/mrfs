#ifndef B64_H_
#define B64_H_

#include <stdint.h>
#include <stddef.h>

#define BASE64_BYTE_BUFFER_SIZE 6000
#define BASE64_CHAR_BUFFER_SIZE 8000
/**
 * Returns the length of the encoded base64 string (excluding '\0' terminator)
 */
size_t Base64EncodedLength(size_t n);

/**
 * Returns the length of the decoded base64 data.
 */
size_t Base64DecodedLength(const char *ascii);

/**
 * Encodes the data into base64.
 *
 * Returns 0 upon success.
 */
int Base64Encode(const uint8_t *bytes, size_t n, char *ascii);

/**
 * Decodes the base64 string into an array of bytes.
 *
 * Returns 0 upon success.
 */
int Base64Decode(const char *ascii, uint8_t *bytes);

int Base64EncodeFile(const char *in_filename, const char *out_filename);

int Base64DecodeFile(const char *in_filename, const char *out_filename);

#endif

