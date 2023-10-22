#include <regex.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Http.h"
#include "Mrbs.h"
#include "utils.h"

#define MRBS_URL_BUFFER_SIZE 2800
#define MRBS_URL_ROOT "https://library.canterbury.ac.nz/webapps/mrbs/"

const int ROOM_LOOKUP[] = {
    215, // ROOM 212A
    216, // ROOM 212B
    217, // ROOM 212C
    225, // ROOM 212D
    226, // ROOM 212E
    218, // ROOM 212F
    219, // ROOM 212G
    220, // ROOM 212H
};

char *_MrbsParamEncode(const char *param);

int _MrbsBookingGetId(mrbs_booking_t *booking);

// http://library.canterbury.ac.nz/webapps/mrbs/edit_entry_handler.php?name=NAME&description=DESCRIPTION&day=6&month=8&year=2020&hour=08&minute=00&duration=1&dur_units=hours&type=I&page=1&area=54&returl=&room_id=215&create_by=jps111&rep_id=0&edit_type=
int MrbsBookingCreate(mrbs_booking_t *booking,
                      const mrbs_credentials_t *credentials, const char *name,
                      const char *description, int year, int month, int day,
                      int hour, int minute, mrbs_dur_t duration,
                      mrbs_area_t area, mrbs_room_t room) {

  booking->credentials = credentials;
  booking->name = (char *)name;
  booking->description = (char *)description;
  booking->time.year = year;
  booking->time.month = month;
  booking->time.day = day;
  booking->time.hour = hour;
  booking->time.minute = minute;
  booking->duration = duration;
  booking->area = area;
  booking->room = room;
  booking->_id = -1;

  return 0;
}

int MrbsBookingSend(mrbs_booking_t *booking) {
  http_request_t req = {0};
  http_response_t res = {0};
  char *url = NULL;
  int status;

  url = _MrbsBookingGetUrl(booking, MRBS_URL_NEW);
  if (HttpRequestCreate(&req, url, HTTP_METHOD_GET) != 0) {
    free(url);
    ERR("Could not create the request.");
    return STATUS_ERR;
  }

  // printf("URL = %s\n", url);

  HttpRequestAddBasicAuth(&req, booking->credentials->username,
                          booking->credentials->password);

  if (HttpRequestSend(&req, &res) != 0) {
    free(url);
    HttpRequestDestroy(&req);
    ERR("Could not send the request");
    return STATUS_ERR;
  }

  switch (res.status_code) {
  case 302:
    status = STATUS_OK;
    break;
  case 200:
    ERR("Received failed status code (200)");
    FILE *f = fopen("dump.html", "w");
    fprintf(f, "%s", res.body);
    fclose(f);
    status = STATUS_ERR;
    break;
  default:
    ERR("Received unexpected status code :(");
    printf("Status: %d, %s\n", res.status_code, res.status_str);
    status = STATUS_ERR;
    break;
  }

  HttpResponseDestroy(&res);
  HttpRequestDestroy(&req);
  free(url);

  return status;
}

int MrbsBookingDelete(mrbs_booking_t *booking) {
  http_request_t req = {0};
  http_response_t res = {0};
  char *url;
  int result = -1;

  _MrbsBookingGetId(booking);

  if (booking->_id) {
    url = _MrbsBookingGetUrl(booking, MRBS_URL_DELETE);
    if (url == NULL) {
      return -1;
    }

    result = HttpRequestCreate(&req, url, HTTP_METHOD_GET);
    if (result != 0) {
      free(url);
      return -1;
    }

    HttpRequestAddBasicAuth(&req, booking->credentials->username,
                            booking->credentials->password);
    result = HttpRequestSend(&req, &res);
    if (result != 0) {
      free(url);
      HttpRequestDestroy(&req);
      return -1;
    }

    if (res.status_code == 302) {
      result = 0;
      booking->_id = 0;
    } else {
      result = -1;
    }

    free(url);
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
  }

  return result;
}

bool MrbsBookingExists(const mrbs_time_t *time, mrbs_room_t room) {
  mrbs_booking_t booking = {.time = *time, .room = room};
  if (_MrbsBookingGetId(&booking) == -1) {
    return false;
  } else {
    return true;
  }
}

// https://en.wikipedia.org/wiki/Query_string
char *_MrbsParamEncode(const char *param) {
  char *enc = NULL;
  char hexbuf[4] = {0};
  char c = '\0';
  size_t enc_len = 128, offset = 0, param_i = 0;

  enc = calloc(enc_len, sizeof(char));
  if (enc) {

    while ((c = param[param_i]) != '\0') {

      // make sure there are at least four characters free on the enc string
      if (enc_len - offset < 4) {
        enc = realloc(enc, enc_len * 2);
        memset(&enc[enc_len], 0, enc_len);
        enc_len *= 2;
      }

      if (c == ' ') {
        strncpy(&enc[offset], "%20", 4);
        offset += 3;
      } else if (c == '~') {
        strncpy(&enc[offset], "%7E", 4);
        offset += 3;
      } else if (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) ||
                 ((c >= '0') && (c <= '9')) || (c == '~') || (c == '-') ||
                 (c == '.') || (c == '_')) {
        enc[offset] = c;
        offset += 1;
      } else if (c == '+') {
        strncpy(&enc[offset], "%2B", 4);
        offset += 3;
      } else {
        snprintf(hexbuf, 4, "%%%02X", c);
        strncpy(&enc[offset], hexbuf, 4);
        offset += 3;
      }

      param_i++;
    }
  }

  return enc;
}

char *_MrbsBookingGetUrl(mrbs_booking_t *booking, mrbs_url_t type) {
  char *url = NULL;
  char *safe_name = NULL, *safe_description = NULL;
  int num_chars_printed = 0;

  url = calloc(MRBS_URL_BUFFER_SIZE, sizeof(char));
  if (url) {

    // assume the credentials name doesn't contain any special escapable
    // characters

    switch (type) {
    case MRBS_URL_NEW:
      safe_name = _MrbsParamEncode(booking->name);
      if (safe_name == NULL) {
        free(url);
        return NULL;
      }

      safe_description = _MrbsParamEncode(booking->description);
      if (safe_description == NULL) {
        free(safe_name);
        free(url);
        return NULL;
      }
      num_chars_printed =
          snprintf(url, MRBS_URL_BUFFER_SIZE,
                   MRBS_URL_ROOT "edit_entry_handler.php?"
                                 "name=%s&"
                                 "description=%s&"
                                 "day=%02d&"
                                 "month=%02d&"
                                 "year=%d&"
                                 "hour=%02d&"
                                 "minute=%02d&"
                                 "duration=%02d&"
                                 "dur_units=minutes&"
                                 "type=I&"
                                 "page=1&"
                                 "returl=&"
                                 "room_id=%d&"
                                 "create_by=%s&"
                                 "rep_id=0&"
                                 "edit_type=",
                   safe_name, safe_description, booking->time.day,
                   booking->time.month, booking->time.year, booking->time.hour,
                   booking->time.minute, booking->duration,
                   ROOM_LOOKUP[booking->room], booking->credentials->username);
      // always free the name and description
      free(safe_name);
      free(safe_description);
      break;
    case MRBS_URL_SHOW_DAY:
      num_chars_printed = snprintf(url, MRBS_URL_BUFFER_SIZE,
                                   MRBS_URL_ROOT "day.php?"
                                                 "day=%02d&"
                                                 "month=%02d&"
                                                 "year=%d&"
                                                 "area=%d&"
                                                 "page=1",
                                   booking->time.day, booking->time.month,
                                   booking->time.year, booking->area);
      break;
    case MRBS_URL_SHOW_WEEK:
      num_chars_printed =
          snprintf(url, MRBS_URL_BUFFER_SIZE,
                   MRBS_URL_ROOT "week.php?"
                                 "page=1&"
                                 "year=%d&"
                                 "month=%02d&"
                                 "day=%02d&"
                                 "room=%d",
                   booking->time.year, booking->time.month, booking->time.day,
                   ROOM_LOOKUP[booking->room]);
      break;
    case MRBS_URL_SHOW:
      // TODO: IMPLEMENT! NEED ID
      num_chars_printed = snprintf(
          url, MRBS_URL_BUFFER_SIZE,
          MRBS_URL_ROOT
          "view_entry.php?"
          "id=%d", // TODO: Add other parameters? We don't really need them
          booking->_id);
      break;
    case MRBS_URL_DELETE:
      num_chars_printed = snprintf(url, MRBS_URL_BUFFER_SIZE,
                                   MRBS_URL_ROOT "del_entry.php?"
                                                 "page=1&"
                                                 "area=%d&"
                                                 "id=%d&"
                                                 "series=0",
                                   booking->area, booking->_id);
      break;
    default:
      fprintf(stderr, "SHOULDN'T GET HERE\n");
      break;
    }

    // if something went wrong, free the url string
    if (num_chars_printed < 0) {
      free(url);
      url = NULL;
    }
  }

  return url;
}

// returns the id of the booking, making a request to the server if required.
int _MrbsBookingGetId(mrbs_booking_t *booking) {
  http_request_t req = {0};
  http_response_t res = {0};
  char *url = NULL;
  int result = 0;

  if (booking->_id >= 1) {
    return booking->_id;
  }

  // make a request to the server
  url = _MrbsBookingGetUrl(booking, MRBS_URL_SHOW_WEEK);
  if (url == NULL) {
    free(url);
    return -1;
  }
  // INFO("here");
  printf("URL: %s\n", url);

  result = HttpRequestCreate(&req, url, HTTP_METHOD_GET);
  if (result != 0) {
    HttpRequestDestroy(&req);
    free(url);
    return -1;
  }

  result = HttpRequestSend(&req, &res);
  if (result != 0) {
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
    free(url);
    fprintf(stderr, "Error sending :(\n");
    return -1;
  }

  // we don't need the request or the url anymore
  HttpRequestDestroy(&req);
  free(url);
  url = NULL;

  char needle[64];
  snprintf(needle, 64, "%02d:%02d</a></td><td", booking->time.hour,
           booking->time.minute);

  char *row_start = NULL;

  printf("BODY HAS %d chars: %s\n\n\n\n", strlen(res.body), res.body);

  row_start = strstr(res.body, needle);
  if (row_start == NULL) {
    fprintf(stderr, "Could not find row start\n");
    HttpResponseDestroy(&res);
    return -1;
  }

  char *row_end = strstr(row_start, "</tr>");

  if (row_end == NULL) {
    fprintf(stderr, "Could not find row end\n");
    HttpResponseDestroy(&res);
    return -1;
  }

  *row_end = '\0';

  snprintf(needle, 64, "view_entry.php\\?page=1&id=([[:digit:]]+)&day=%02d",
           booking->time.day);
  regex_t r = {0};
  regmatch_t m[2] = {0};

  if (regcomp(&r, needle, REG_EXTENDED) != 0) {
    perror("Regex could not compile");
    regfree(&r);
    HttpResponseDestroy(&res);
    return -1;
  }

  if (regexec(&r, row_start, 2, m, 0) != 0) {
    // printf("ROW: %s\n", row_start);
    // printf("Could not find string '%s'\n", needle);
    perror("Regex didn't match");
    regfree(&r);
    HttpResponseDestroy(&res);
    return -1;
  }

  booking->_id = atoi(&row_start[m[1].rm_so]);

  // printf("Found the booking id: %d!\n", booking->_id);

  regfree(&r);
  HttpResponseDestroy(&res);

  return booking->_id;
}

char *_MrbsGetName(char *body) {
  char *sof = NULL;
  char *eof = NULL;

  sof = strstr(body, "<H2>");
  if (sof == NULL) {
    fprintf(stderr, "_MrbsGetName: could not find start tag.\n");
    return NULL; // could not find the start tag
  }

  sof += 4; // advance past the start tag

  eof = strstr(sof, "</H2>");
  if (eof == NULL) {
    fprintf(stderr, "_MrbsGetName: could not find end tag.\n");
    return NULL; // could not find the end tag
  }

  size_t len = eof - sof;
  char *name = calloc(len + 1, sizeof(char));

  memcpy(name, sof, len);

  return name;
}

char *_MrbsGetDescription(char *body) {
  char *sof = NULL;
  char *eof = NULL;
  char *desc = NULL;
  size_t len = 0;

  sof = strstr(body, "</H2>");
  if (sof == NULL) {
    fprintf(stderr, "_MrbsGetDescription: could not find end tag of H2.\n");
    return NULL;
  }

  sof = strstr(sof, "<td>");
  if (sof == NULL) {
    fprintf(stderr,
            "_MrbsGetDescription: could not find start of first TD tag.\n");
    return NULL;
  }

  sof = strstr(sof + 1, "<td>");
  if (sof == NULL) {
    fprintf(stderr, "_MrbsGetDescription: could not find the start of the "
                    "second TD tag.\n");
    return NULL;
  }

  sof += 4; // advance the start of the tag to the start of text

  eof = strstr(sof, "</td>");
  if (eof == NULL) {
    fprintf(stderr, "_MrbsGetDescription: could not find the end tag.\n");
    return NULL;
  }

  len = eof - sof;
  desc = calloc(len + 1, sizeof(char));

  memcpy(desc, sof, len);

  return desc;
}

// populates a booking from the backend
int MrbsBookingGet(mrbs_booking_t *booking,
                   const mrbs_credentials_t *credentials, int year, int month,
                   int day, int hour, int minute, mrbs_room_t room) {
  if (booking->_id <= 0) {
    booking->time.year = year;
    booking->time.month = month;
    booking->time.day = day;
    booking->time.hour = hour;
    booking->time.minute = minute;
    booking->room = room;
    booking->credentials = credentials;

    // printf("Added extra info.\n");
    if (_MrbsBookingGetId(booking) == -1) {
      fprintf(
          stderr,
          "Could not get the id for the booking, maybe it doesn't exist?\n");
      return -1;
    }
  }

  // we know the id is good.
  char *url = _MrbsBookingGetUrl(booking, MRBS_URL_SHOW);

  http_response_t res = {0};
  http_request_t req = {0};

  if (HttpRequestCreate(&req, url, HTTP_METHOD_GET) != 0) {
    HttpRequestDestroy(&req);
    fprintf(stderr, "Could not create http request.\n");
    return -1;
  }
  // printf("URL: %s\n", url);

  HttpRequestAddBasicAuth(&req, credentials->username, credentials->password);

  if (HttpRequestSend(&req, &res) != 0) {
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
    free(url);
    fprintf(stderr, "Could not send http request.\n");
    return -1;
  }

  // don't need the URL anymore
  free(url);
  url = NULL;

  // Check that the ID was valid
  if (strstr(res.body, "Invalid entry id.") != NULL) {
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
    fprintf(stderr, "Invalid entry id. id = %d\n", booking->_id);
    return -1;
  }

  // we have a response. populate the name field of the booking
  booking->name = _MrbsGetName(res.body);

  if (booking->name == NULL) {
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
    fprintf(stderr, "Could not get the name of booking.\n");
    return -1;
  }

  booking->description = _MrbsGetDescription(res.body);
  if (booking->description == NULL) {
    HttpRequestDestroy(&req);
    HttpResponseDestroy(&res);
    fprintf(stderr, "Could not get the description of booking.\n");
    return -1;
  }

  HttpRequestDestroy(&req);
  HttpResponseDestroy(&res);
  return 0;
}

void MrbsBookingDestroy(mrbs_booking_t *booking) {
  if (booking->name != NULL) {
    free(booking->name);
  }

  if (booking->description != NULL) {
    free(booking->description);
  }
}

bool MrbsBookingIsBookable(mrbs_booking_t *booking) {
  // to determine if a specific time is bookable:
  // if (the booking can be created) then delete it and return true else return
  // false
  if (MrbsBookingSend(booking) == 0) {
    MrbsBookingDelete(booking);
    return true;
  } else {
    return false;
  }
}

/**
 * @returns The first possible booking time.
 */
mrbs_time_t MrbsGetEarlyTime(void) {
  return (mrbs_time_t){.year = MRBS_EARLY_YEAR,
                       .month = MRBS_EARLY_MONTH,
                       .day = MRBS_EARLY_DAY,
                       .hour = MRBS_EARLY_HOUR,
                       .minute = MRBS_EARLY_MINUTE};
}

// returns the next available booking.
// slow linear search :(
int MrbsGetFreeBooking(mrbs_booking_t *booking, const mrbs_time_t *startTime,
                       mrbs_room_t startRoom) {
  // set the default time to start searching
  mrbs_time_t time = MrbsGetEarlyTime();

  mrbs_room_t room = MRBS_ROOM_212A;

  mrbs_booking_t tmpBooking = {0};
  tmpBooking.description = "";
  tmpBooking.name = "MRBS-DELETE-ME";

  if (startTime != NULL) {
    time = *startTime;
  }

  if (startRoom != MRBS_ROOM_NONE) {
    room = startRoom;
  }

  while (time.year < 2020) {

    // printf(
    //     "Seeing if booking at %04d-%02d-%02d %02d:%02d in the room with the
    //     id "
    //     "%d is free... ",
    //     time.year, time.month, time.day, time.hour, time.minute,
    //     ROOM_LOOKUP[room]);

    tmpBooking.credentials = booking->credentials;
    tmpBooking.room = room;
    tmpBooking.time = time;
    if (MrbsBookingIsBookable(&tmpBooking)) {
      // we have found a free room and time!
      // printf("Yes!\n");
      booking->time = time;
      booking->room = room;
      return STATUS_OK;
    } else {
      // printf("Nope!\n");
      // we have not found a free room and time :(
      // scan over all rooms firs
      ++room;

      if (room == MRBS_ROOM_MAX) {
        room = MRBS_ROOM_MIN;
      }

      if ((room == startRoom) ||
          ((startRoom == MRBS_ROOM_NONE) && (room == MRBS_ROOM_MIN))) {

        // increment the minute
        time.minute += 30;

        if (time.minute == 60) {

          // increment the hour
          time.minute = 0;
          time.hour++;

          if (time.hour > 20) { // 8PM

            // increment the day
            time.hour = 8; // 8 AM
            time.day++;

            if (time.day == 31) {
              // increment the month
              time.day = 1;
              time.month++;

              if (time.month == 13) {

                // increment the year
                time.month = 1;
                time.year++;
              }
            }
          }
        }
      }
    }
  }

  return STATUS_ERR;
}

int MrbsRoomLookup(mrbs_room_t room) { return ROOM_LOOKUP[(int)room]; }

mrbs_room_t MrbsReverseRoomLookup(int room_id) {
  int i;
  for (i = MRBS_ROOM_NONE + 1; i < MRBS_ROOM_MAX; ++i) {
    if (ROOM_LOOKUP[i] == room_id) {
      break;
    }
  }
  if (i == MRBS_ROOM_MAX) {
    return MRBS_ROOM_NONE;
  }
  return (mrbs_room_t)i;
}

int MrbsBookingReserve(mrbs_booking_t *booking) {
  mrbs_booking_t reserve = {0};
  reserve.name = "MRFS-RESERVED";
  reserve.description = "";
  reserve.credentials = booking->credentials;
  reserve.time = booking->time;
  reserve.room = booking->room;
  return MrbsBookingSend(&reserve);
}

void MrbsPrintBookingDetails(mrbs_booking_t *booking, FILE *f) {
  if (booking->_id < 1) {
    _MrbsBookingGetId(booking);
  }
  fprintf(f, "#%d at %04d-%02d-%02d %02d:%02d in %d", booking->_id,
          booking->time.year, booking->time.month, booking->time.day,
          booking->time.hour, booking->time.minute, ROOM_LOOKUP[booking->room]);
}