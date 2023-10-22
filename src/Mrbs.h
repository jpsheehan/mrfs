#ifndef MRBS_H_
#define MRBS_H_

#include <stdbool.h>
#include <stdio.h>

// I manually checked the first 80 area ids
typedef enum {
  MRBS_AREA_EPS_TRAINING = 1, // hidden
  MRBS_AREA_LEVEL_9 = 2,
  MRBS_AREA_LEVEL_11 = 3,
  MRBS_AREA_EPS_101C = 4,        // hidden
  MRBS_AREA_LAPTOP_TROLLEY = 12, // hidden
  MRBS_AREA_AV = 15,
  MRBS_AREA_TEST = 16,           // hidden
  MRBS_AREA_REDMAYNE = 18,       // hidden
  MRBS_AREA_LEVEL_2_HIDDEN = 19, // hidden
  MRBS_AREA_AV_2 = 20,           // hidden
  MRBS_AREA_EQUIPMENT = 21,      // hidden
  MRBS_AREA_LEVEL_3 = 22,        // hidden
  MRBS_AREA_ALPHA = 23,          // hidden
  MRBS_AREA_TUTORS_1 = 28,       // hidden
  MRBS_AREA_MACMILLAN = 27,
  MRBS_AREA_TUTORS_2 = 29, // hidden
  MRBS_AREA_EPS = 32,
  MRBS_AREA_CRYPT = 33,         // hidden
  MRBS_AREA_COMMITTEE = 34,     // hidden
  MRBS_AREA_CAVE = 36,          // hidden
  MRBS_AREA_HELPDESK = 37,      // hidden
  MRBS_AREA_MANAGERS = 50,      // hidden
  MRBS_AREA_LEVEL_3_302 = 52,   // hidden
  MRBS_AREA_EDUCATION_LIB = 53, // hidden
  MRBS_AREA_LEVEL_2_212 = 54,
  MRBS_AREA_CENTRAL_L2 = 56,    // hidden
  MRBS_AREA_LEVEL_2_214_2 = 58, // hidden
  MRBS_AREA_EQUIPMENT_2 = 59,   // hidden
  MRBS_AREA_LEVEL_8 = 60,
  MRBS_AREA_LEVEL_6 = 61, // hidden
  MRBS_AREA_LEVEL_2_214 = 62,
} mrbs_area_t;

typedef enum {
  MRBS_ROOM_NONE = -1,
  MRBS_ROOM_MIN = 0,
  MRBS_ROOM_212A = 0,
  MRBS_ROOM_212B = 1,
  MRBS_ROOM_212C = 2,
  MRBS_ROOM_212D = 3,
  MRBS_ROOM_212E = 4,
  MRBS_ROOM_212F = 5,
  MRBS_ROOM_212G = 6,
  MRBS_ROOM_212H = 7,
  MRBS_ROOM_MAX
} mrbs_room_t;

typedef enum {
  MRBS_DUR_30_MINS = 30,
  MRBS_DUR_1_HOUR = 60,
  MRBS_DUR_90_MINS = 90,
  MRBS_DUR_2_HOURS = 120,
} mrbs_dur_t;

typedef struct {
  char *username;
  char *password;
} mrbs_credentials_t;

typedef struct {
  int year, month, day;
  int hour, minute, second;
} mrbs_time_t;

typedef struct {
  char *name;
  char *description;
  const mrbs_credentials_t *credentials;
  mrbs_area_t area;
  mrbs_room_t room;
  mrbs_dur_t duration;
  mrbs_time_t time;
  int _id;
} mrbs_booking_t;

typedef enum {
  MRBS_URL_NEW,
  MRBS_URL_SHOW,
  MRBS_URL_SHOW_DAY,
  MRBS_URL_SHOW_WEEK,
  MRBS_URL_DELETE,
} mrbs_url_t;

#define MRBS_EARLY_YEAR 1901
#define MRBS_EARLY_MONTH 12
#define MRBS_EARLY_DAY 14
#define MRBS_EARLY_HOUR 10
#define MRBS_EARLY_MINUTE 0

int MrbsBookingCreate(mrbs_booking_t *booking,
                      const mrbs_credentials_t *credentials, const char *name,
                      const char *description, int year, int month, int day,
                      int hour, int minute, mrbs_dur_t duration,
                      mrbs_area_t area, mrbs_room_t room);

void MrbsBookingDestroy(mrbs_booking_t *booking);

// generates a url to hold the booking information
// this url is dynamically allocated and must be freed by the caller
char *_MrbsBookingGetUrl(mrbs_booking_t *booking, mrbs_url_t type);

int MrbsBookingSend(mrbs_booking_t *booking);

int MrbsBookingDelete(mrbs_booking_t *booking);

int MrbsBookingGet(mrbs_booking_t *booking,
                   const mrbs_credentials_t *credentials, int year, int month,
                   int day, int hour, int minute, mrbs_room_t room);

bool MrbsBookingExists(const mrbs_time_t *time, mrbs_room_t room);

bool MrbsBookingIsBookable(mrbs_booking_t *booking);

int MrbsGetFreeBooking(mrbs_booking_t *booking, const mrbs_time_t *startTime,
                       mrbs_room_t startRoom);

// returnst the mrbs_room_t enum from an actual room id
mrbs_room_t MrbsReverseRoomLookup(int room_id);

/**
 * Reserves a booking for future use.
 * @param booking The booking to reserve. The name and description will be
 * changed for the actual booking.
 * @returns STATUS_OK on success, STATUS_ERR on failure.
 */
int MrbsBookingReserve(mrbs_booking_t *booking);

int MrbsRoomLookup(mrbs_room_t room);

void MrbsPrintBookingDetails(mrbs_booking_t *booking, FILE *f);

#endif
