#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "Http.h"

int main(int argc, char* argv[])
{
	int status;
	char *url = NULL;

	http_request_t *req = malloc(sizeof(http_request_t));
	http_response_t *res = malloc(sizeof(http_response_t));

	if (argc == 2) {
		url = argv[1];
	} else {
		url = "https://library.canterbury.ac.nz/webapps/mrbs/day.php";
	}

	status = HttpRequestCreate(req, url, HTTP_METHOD_GET);

	if (status == 0) {
		HttpRequestAddBasicAuth(req, "USERNAME", "PASSWORD");
		status = HttpRequestSend(req, res);

		if (status == 0) {

			printf("Code: %d\nStatus: %s\nHeaders:\n%s\nBody:\n%s\n", res->status_code, res->status_str, res->headers, res->body);
		} else {
			printf("NOT OK\n");
		}
	}

	HttpRequestDestroy(req);
	HttpResponseDestroy(res);

	free(res);
	free(req);

	return EXIT_SUCCESS;
}

