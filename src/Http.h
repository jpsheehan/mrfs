#ifndef HTTP_H_
#define HTTP_H_

#include <stdint.h>
#include <curl/curl.h>

#include "KeyValueList.h"

typedef enum {
	HTTP_METHOD_GET,
} http_method_t;

typedef CURL http_request_t;

typedef struct {
	char *data;
	size_t data_read;
} http_response_t;

/**
 * Creates a new request based upon a URL and method.
 *
 * @param 	req		A pointer to the request struct.
 * @param	url		A pointer to the character array containing the URL.
 * @param	method	The method for this request.
 * @param	res		A pointer to the response struct.
 * @returns	0 on success.
 *
 */
int HttpRequestCreate(http_request_t **req, const char *url, http_method_t method, http_response_t *res);

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
 *
 * @returns	0 on success.
 *
 */
int HttpRequestSend(http_request_t *req);

void HttpRequestAddBasicAuth(http_request_t* req, const char *username, const char *password);

#endif

