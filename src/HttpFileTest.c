#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "Http.h"

int main(int argc, char *argv[])
{
	http_request_t req = { 0 };
	http_response_t res = { 0 };
	int status;
	FILE *f = NULL;
	char *url, *filename;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s url filename\n", argv[0]);
		return EXIT_FAILURE;
	}

	url = argv[1];
	filename = argv[2];

	status = HttpRequestCreate(&req, url, HTTP_METHOD_GET);
	if (status != 0)
	{
		fprintf(stderr, "Could not create request\n");
		return EXIT_FAILURE;
	}

	status = HttpRequestSend(&req, &res);
	if (status != 0)
	{
		fprintf(stderr, "Could not get response\n");
		return EXIT_FAILURE;
	}
	
	f = fopen(filename, "wb");
	fwrite(res.body, sizeof(uint8_t), res.body_len, f);
	fclose(f);

	HttpRequestDestroy(&req);
	HttpResponseDestroy(&res);

	return EXIT_SUCCESS;
}

