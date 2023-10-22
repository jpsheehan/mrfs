#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <regex.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Base64.h"
#include "Http.h"
#include "KeyValueList.h"
#include "utils.h"

#include <curl/curl.h>

#define BUFFER_SIZE 2048

#define HTTP_USER_AGENT "mrfs/0.1"

// forward declarations
size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata);

int HttpRequestCreate(CURL **req, const char *url,
                      http_method_t method, http_response_t *res) {
  CURL* handle = curl_easy_init();

  if (handle) {
    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, res);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, &HttpWriteCallback);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, HTTP_USER_AGENT);
    // curl_easy_setopt(handle, CURLOPT_VERBOSE, 1L);
    if (method == HTTP_METHOD_GET) {
      curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    }
    *req = handle;
    return STATUS_OK;
  }

  return STATUS_ERR;
}

int HttpRequestDestroy(CURL* handle) {
  if (handle) {
    curl_easy_cleanup(handle);
  }
  return STATUS_OK;
}

int HttpRequestSend(CURL *req) {
  CURLcode status;

  status = curl_easy_perform(req);
  if (status == CURLE_OK) {
    return STATUS_OK;
  }

  return STATUS_ERR;
}

int HttpResponseDestroy(http_response_t *res) {
  // this also frees all the other data
  free(res->data);
  return STATUS_OK;
}

void HttpRequestAddBasicAuth(CURL *req, const char *username,
                             const char *password) {
  curl_easy_setopt(req, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
  curl_easy_setopt(req, CURLOPT_USERNAME, username);
  curl_easy_setopt(req, CURLOPT_PASSWORD, password);
}

size_t HttpWriteCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
  size_t realsize = size * nmemb;
  http_response_t *res = (http_response_t*)userdata;

  res->data = realloc(res->data, res->data_read + realsize + 1);
  memcpy(&(res->data[res->data_read]), ptr, realsize);
  res->data_read += realsize;
  res->data[res->data_read] = '\0';

  return realsize;
}
