#include ".env.h"
#include "Mrbs.h"
#include "Mrfs.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_SIZE (1024 * 1024)

void test_linear_search() {
  mrbs_booking_t booking = {0};
  mrbs_credentials_t creds = {.username = USERNAME, .password = PASSWORD};
  booking.credentials = &creds;

  if (MrbsGetFreeBooking(&booking, NULL, MRBS_ROOM_NONE) == STATUS_OK) {
    printf("Found booking at %04d-%02d-%02d %02d:%02d in the room with the "
           " id %d\n",
           booking.time.year, booking.time.month, booking.time.day,
           booking.time.hour, booking.time.minute,
           MrbsReverseRoomLookup(booking.room));
  } else {
    printf("Could not find a free booking :(\n");
  }
}

int main(int argc, char *argv[]) {
  mrbs_credentials_t creds = {.username = USERNAME, .password = PASSWORD};
  mrbs_booking_t inode = {0};

  if (argc != 2) {
    printf("Usage: %s filename\n", argv[0]);
    return EXIT_FAILURE;
  }

  uint8_t *buffer = NULL;
  buffer = calloc(MAX_FILE_SIZE, sizeof(char));
  if (buffer == NULL) {
    printf("Whoops, could not allocate %d bytes of memory for buffer\n",
           MAX_FILE_SIZE);
    return EXIT_FAILURE;
  }

  char *filename = argv[1];

  FILE *f = NULL;

  f = fopen(filename, "rb");

  if (f) {
    size_t n = fread(buffer, sizeof(uint8_t), MAX_FILE_SIZE, f);

    if (!feof(f)) {
      printf("Whoops, maximum allowable file size is %d bytes\n",
             MAX_FILE_SIZE);
      free(buffer);
      fclose(f);
      return EXIT_FAILURE;
    }

    printf("File %s is %lu bytes in size\n", filename, n);

    if (MrfsFile_Create(&creds, buffer, n, &inode) == STATUS_ERR) {
      printf("Whoops, could not upload file\n");
    } else {
      printf("Cool! Uploaded the file!\n");
      MrbsPrintBookingDetails(&inode, stdout);
      printf("\n");
    }

    fclose(f);
  }

  free(buffer);
  return EXIT_SUCCESS;
}