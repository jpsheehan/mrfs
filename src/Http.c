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

#define BUFFER_SIZE 2048

static char *CRLF = "\r\n";
static char *VERSION = "HTTP/1.1";
static char *method_str = "GET";

#define HTTP_USER_AGENT "mrfs/0.1"

// forward declarations
int HttpParseUrl(const char *url, http_request_t *req);
int HttpResponseParse(http_response_t *res, char *data, size_t data_len);

int HttpRequestCreate(http_request_t *req, const char *url,
                      http_method_t method) {
  if (req) {
    if (HttpParseUrl(url, req) < 0) {
      return STATUS_ERR;
    } else {
      req->headers = KeyValueListCreate("User-Agent", HTTP_USER_AGENT);
      if (req->headers == NULL) {
        return STATUS_ERR;
      }

      req->method = method;
    }
  }
  return STATUS_OK;
}

int HttpRequestDestroy(http_request_t *req) {
  KeyValueListDestroy(req->headers);
  free(req->host);
  free(req->path);
  return STATUS_OK;
}

int HttpParseUrl(const char *url, http_request_t *req) {
  regex_t r = {0};
  regmatch_t m[4] = {0};
  size_t len = 0;

  if ((regcomp(&r, "^http://([[:alnum:].]+)(:[[:digit:]]+)?(.*)$",
               REG_EXTENDED) != 0)) {
    ERR("could not compile regex");
    regfree(&r);
    return STATUS_ERR;
  }

  if ((regexec(&r, url, 4, m, 0) != 0)) {
    ERR("could not execute regex");
    regfree(&r);
    return STATUS_ERR;
  }

  // get host
  len = m[1].rm_eo - m[1].rm_so;
  req->host = calloc(len + 1, sizeof(char));
  strncpy(req->host, &url[m[1].rm_so], len);

  // get port
  if (m[2].rm_so != -1) {
    len = m[2].rm_eo - m[2].rm_so - 1;
    req->port = atoi(&url[m[2].rm_so + 1]);
  } else {
    req->port = 80;
  }

  // get path
  // if there is no / at the end of url, m[3]'s rm_so and rm_eo are equal
  if ((m[3].rm_so != -1) && (m[3].rm_so != m[3].rm_eo)) {
    len = m[3].rm_eo - m[3].rm_so;
    req->path = calloc(len + 1, sizeof(char));
    strncpy(req->path, &url[m[3].rm_so], len);
  } else {
    req->path = calloc(2, sizeof(char));
    req->path[0] = '/';
    req->path[1] = '\0';
  }

  // printf("Host = '%s', Port = %d, Path = '%s'\n", req->host, req->port,
  // req->path);
  regfree(&r);

  return STATUS_OK;
}

int HttpRequestSend(const http_request_t *req, http_response_t *res) {
  int sockfd, result;
  struct sockaddr_in serv_addr = {0};

  // TODO: move all the socket creation functions into their own module (for
  // portability)
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    ERR("could not create socket");
    return STATUS_ERR;
  }

  // convert host to ip
  struct hostent *he;
  struct in_addr **addr_list;
  char ip[40] = {0};
  he = gethostbyname(req->host);
  if (he == NULL) {
    perror("gethostbyname");
    ERR("could not get hostname");
    return STATUS_ERR;
  }
  addr_list = (struct in_addr **)he->h_addr_list;
  result = 0;
  for (int i = 0; addr_list[i] != NULL; ++i) {
    strncpy(ip, inet_ntoa(*addr_list[i]), 40);
    result = 1;
    break;
  }
  if (result == 0) {
    ERR("could not get IP address");
    return STATUS_ERR;
  }

  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(req->port);

  result = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (result < 0) {
    perror("ERROR connecting");
    ERR("could not connect to remote server");
    return STATUS_ERR;
  }

  // add the headers:
  KeyValueListSet(req->headers, "Host", req->host);
  KeyValueListSet(req->headers, "Accept", "text/html");
  KeyValueListSet(req->headers, "Connection", "close");

  char *buffer = NULL;
  size_t buffer_size = BUFFER_SIZE;

  buffer = calloc(BUFFER_SIZE, sizeof(char));

  if (buffer == NULL) {
    ERR("could not allocate space for buffer");
    return STATUS_ERR;
  }

  // send the first line of the HTTP header
  result = snprintf(buffer, BUFFER_SIZE, "%s %s %s%s", method_str, req->path,
                    VERSION, CRLF);
  ssize_t n = write(sockfd, buffer, strlen(buffer));
  n ^= n;

  // send the headers
  ll_t *l = (ll_t *)req->headers;
  kv_t *kv = NULL;
  do {
    kv = (kv_t *)l->p;
    result += snprintf(buffer, BUFFER_SIZE, "%s: %s%s", kv->k, kv->v, CRLF);
    // printf("TX: %s", buffer);
    n = write(sockfd, buffer, strlen(buffer));

    l = l->next;
  } while (l);

  // send the final double new line
  result += snprintf(buffer, BUFFER_SIZE, "%s", CRLF);
  n = write(sockfd, buffer, strlen(buffer));

  if (result < 0) {
    perror("ERROR writing to socket");
    ERR("error writing to socket");
    return STATUS_ERR;
  }

  // printf("SENT %d bytes\n", result);

  memset(buffer, 0, buffer_size);

  size_t offset = 0;
  while (true) {
    // printf("Asking for %ld bytes... ", buffer_size - offset);
    result = recv(sockfd, &buffer[offset], buffer_size - offset, 0);

    if (result < 0) {
      perror("Error reading from socket.");
      ERR("could not read from socket");
      return STATUS_ERR;
    }

    if (result == 0) {
      break;
    }

    // printf("Read %d bytes, buffer usage is %ld/%ld", result, offset + result,
    // buffer_size);
    if (offset + result >= buffer_size) {
      // realloc
      // printf(", realloc'ing to %ld...\n", buffer_size * 2);
      char *tmpBuffer = NULL;
      tmpBuffer = realloc(buffer, buffer_size * 2);
      if (tmpBuffer == NULL) {
        ERR("could not realloc buffer");
        free(buffer);
        return STATUS_ERR;
      }
      buffer = tmpBuffer;
      memset(&buffer[buffer_size], 0, buffer_size);
      buffer_size *= 2;
    } else {
      // printf("\n");
    }
    offset += result;
  }

  if (result < 0) {
    perror("ERROR reading from socket");
    ERR("could not read from socket");
    return STATUS_ERR;
  }

  result = HttpResponseParse(res, buffer, offset);
  // printf("RECEIVED %lu bytes:\n%s", res->headers_len + res->body_len,
  // res->headers);

  return result;
}

int HttpResponseDestroy(http_response_t *res) {
  // this also frees all the other data
  free(res->_data);
  return STATUS_OK;
}

/**
 * This will clobber the data!
 */
int HttpResponseParse(http_response_t *res, char *data, size_t data_len) {
  // store a reference to the data so we can free it later
  res->_data = data;

  // First line:
  // HTTP/1.0 200 OK\r\n

  char *eol = NULL;
  eol = strstr(data, "\r\n");
  if (eol == NULL) {
    ERR("could not find the end of first line");
    return STATUS_ERR;
  }
  *eol = '\0'; // clobber the carriage return

  char *start_code = NULL;
  start_code = strchr(data, ' ');
  if ((start_code == NULL) || (start_code > eol)) {
    ERR("could not split the HTTP status line");
    return STATUS_ERR;
  }
  start_code++; // skip past the whitespace to the number

  char *start_status = NULL;
  start_status = strchr(start_code, ' ');
  if ((start_status == NULL) || (start_status > eol)) {
    ERR("could not find the status code");
    return STATUS_ERR;
  }
  *start_status = '\0'; // clobber the whitespace so the code ends properly
  start_status++;       // skip past the whitespace

  res->status_code = atoi(start_code);
  res->status_str = start_status;

  eol += 2; // advance to the beginning of the headers
  res->headers = eol;

  // now find the end of the headers
  char *eoh = NULL;
  eoh = strstr(eol, "\r\n\r\n");
  if (eoh == NULL) {
    ERR("could not find the end of the header");
    return STATUS_ERR;
  }
  *eoh = '\0'; // clobber this so the headers string reads properly

  res->headers_len = eoh - data;

  eoh += 4; // advance to the body

  res->body_len = data_len - (eoh - data);

  res->body = eoh;

  return STATUS_OK;
}

void HttpRequestAddBasicAuth(http_request_t *req, const char *username,
                             const char *password) {
  // Authorization: Basic ...
  size_t plaintext_len = strlen(username) + strlen(password) + 2;
  char *plaintext = calloc(plaintext_len, sizeof(char));

  if (plaintext) {
    size_t base64_len = Base64EncodedLength(plaintext_len - 1) + 1;
    char *base64 = calloc(base64_len, sizeof(char));

    if (base64) {
      snprintf(plaintext, plaintext_len, "%s:%s", username, password);
      Base64Encode((uint8_t *)plaintext, strlen(plaintext), base64);

      size_t value_len = strlen(base64) + 7; // 7 = strlen("Basic \0")
      char *value = calloc(value_len, sizeof(char));

      if (value) {
        snprintf(value, value_len, "Basic %s", base64);

        KeyValueListSet(req->headers, "Authorization", value);

        free(value);
      } else {
        ERR("could not allocate space for value");
      }

      free(base64);
    } else {
      ERR("could not allocate space for base64");
    }

    free(plaintext);
  } else {
    ERR("could not allocate space for plaintext");
  }
}
