#ifndef HTTP_H_
#define HTTP_H_

#include <stdint.h>

#include "KeyValueList.h"

typedef enum {
	HTTP_METHOD_GET,
} http_method_t;

typedef struct {
	char *host;
	char *path;
	uint16_t port;
	http_method_t method;
	kvl_t* headers;
} http_request_t;

typedef struct {
	char *_data;
	char *headers;
	char *body;
	size_t body_len;
	size_t headers_len;
	int status_code;
	char* status_str;
} http_response_t;

/**
 * Creates a new request based upon a URL and method.
 *
 * @param 	req		A pointer to the request struct.
 * @param	url		A pointer to the character array containing the URL.
 * @param	method	The method for this request.
 * @returns	0 on success.
 *
 */
int HttpRequestCreate(http_request_t *req, const char *url, http_method_t method);

/**
 * Frees the memory associated with the request.
 * If the request struct was dynamically allocated it must be freed outside of this function.
 *
 * @returns 0 on success.
 *
 */
int HttpRequestDestroy(http_request_t *req);

/**
 * Frees the memory associated with the response.
 * If the response struct was dynamically allocated it must be freed outside of this function.
 *
 * @returns	0 on success.
 *
 */
int HttpResponseDestroy(http_response_t *res);

/**
 * Sends an HTTP request to the server. The response struct is populated.
 *
 * @param	req		A pointer to the request struct.
 * @param	res		A pointer to the response struct.
 *
 * @returns	0 on success.
 *
 */
int HttpRequestSend(const http_request_t *req, http_response_t *res);

void HttpRequestAddBasicAuth(http_request_t* req, const char *username, const char *password);

#endif

