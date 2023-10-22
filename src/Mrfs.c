#include <math.h>
#include <stdio.h>
#include <string.h>

#include "Base64.h"
#include "Mrbs.h"
#include "Mrfs.h"
#include "utils.h"

#define MRFS_NAME_LEN 128
#define MRFS_DESC_LEN 2400

void print_nid(const nid_t *nid);

int MrfsParseNid(const char *str, nid_t *nid);

int MrfsDirectoryRead(directory_t *d, mrbs_credentials_t *credentials,
                      nid_t *nid) {
  if (d == NULL) {
    ERR("d cannot be null");
    return STATUS_ERR;
  }

  if (credentials == NULL) {
    ERR("credentials cannot be null");
    return STATUS_ERR;
  }

  if (nid == NULL) {
    ERR("nid cannot be null");
    return STATUS_ERR;
  }

  mrbs_booking_t booking = {0};

  if (MrbsBookingGet(&booking, credentials, nid->year, nid->month, nid->day,
                     nid->hour, nid->min, nid->room) == STATUS_ERR) {
    ERR("cannot get booking");
    MrbsBookingDestroy(&booking);
    return STATUS_ERR;
  }

  // attempt to extract the node id from the name
  nid_t actual_nid = {0};
  if (sscanf(booking.name, "MRFSD-%hu-%hhu-%hhu-%hhu-%hhu-%hd",
             &actual_nid.year, &actual_nid.month, &actual_nid.day,
             &actual_nid.hour, &actual_nid.min, &actual_nid.room) != 6) {
    ERR("could not get 6 values from booking name");
    MrbsBookingDestroy(&booking);
    return STATUS_ERR;
  }

  // print_nid(nid);
  // print_nid(&actual_nid);

  // check that the node ids are all good
  if ((actual_nid.year != nid->year) || (actual_nid.month != nid->month) ||
      (actual_nid.day != nid->day) || (actual_nid.hour != nid->hour) ||
      (actual_nid.min != nid->min) || (actual_nid.room != nid->room)) {
    ERR("actual nid differs from the specified one");
    MrbsBookingDestroy(&booking);
    return STATUS_ERR;
  }

  // Read data out of booking here
  char *message = calloc(MRFS_BYTES_PER_CHUNK, sizeof(char));

  // TODO: Check for errors?
  Base64Decode(booking.description, (uint8_t *)message);

  char *sol = NULL, *eol = NULL;

  // the first line contains the number of files
  sol = message;
  eol = strchr(sol, '\n');
  if (eol == NULL) {
    eol = sol + strlen(sol);
  } else {
    *eol = '\0';
  }

  d->num_files = atoi(sol);

  int i = 0;
  while (true) {
    sol = eol + 1;
    if (*sol == '\0') {
      break;
    }
    eol = strchr(sol, '\n');
    if (eol == NULL) {
      eol = sol + strlen(sol);
    }
    *eol = '\0';

    // extract info from the line
    // the line will be in the form
    // booking id COMMA filename
    char *sep = strchr(sol, ',');
    if (sep == NULL) {
      continue;
    }
    *sep = '\0';

    file_ref_t *fref = &d->file_refs[i];

    if (fref->name) {
      free(fref->name);
    }
    fref->name = strdup(sep + 1);

    if (MrfsParseNid(sol, &fref->id) == STATUS_ERR) {
      ERR("Couldn't parse the nid");
    }

    ++i;
  }

  if (i != d->num_files) {
    ERR("Mismatched values!");
  }

  free(message);
  MrbsBookingDestroy(&booking);

  return STATUS_OK;
}

int MrfsDirectoryWrite(directory_t *d, mrbs_credentials_t *credentials,
                       nid_t *nid) {
  char *name = NULL, *description = NULL;
  mrbs_booking_t booking = {0};

  if (d == NULL) {
    ERR("d cannot be null");
    return STATUS_ERR;
  }

  if (credentials == NULL) {
    ERR("credentials cannot be null");
    return STATUS_ERR;
  }

  if (nid == NULL) {
    ERR("nid cannot be null");
    return STATUS_ERR;
  }

  name = calloc(MRFS_NAME_LEN, sizeof(char));
  description = calloc(MRFS_DESC_LEN, sizeof(char));

  snprintf(name, MRFS_NAME_LEN, "MRFSD-%04d-%02d-%02d-%02d-%02d-%04d",
           nid->year, nid->month, nid->day, nid->hour, nid->min, nid->room);

  // actually populate the message with info about the files

  char *message = calloc(MRFS_BYTES_PER_CHUNK, sizeof(char));
  char *offset = message;
  snprintf(offset, MRFS_BYTES_PER_CHUNK, "%d\n", d->num_files);
  offset = message + strlen(message);
  for (int i = 0; i < d->num_files; ++i) {
    file_ref_t *fref = &d->file_refs[i];
    snprintf(offset, MRFS_BYTES_PER_CHUNK - (offset - message),
             "MRFSI-%04d-%02d-%02d-%02d-%02d-%04d,%s\n", fref->id.year,
             fref->id.month, fref->id.day, fref->id.hour, fref->id.min,
             fref->id.room, fref->name);
    offset += 26 + 1 + strlen(fref->name) + 1;
  }

  // encode the description
  Base64Encode((uint8_t *)message, strlen(message), description);
  free(message);
  message = NULL;

  if (MrbsBookingCreate(&booking, credentials, name, description, nid->year,
                        nid->month, nid->day, nid->hour, nid->min,
                        MRBS_DUR_30_MINS, 0, nid->room) == STATUS_ERR) {
    MrbsBookingDestroy(&booking);
    ERR("could not create booking");
    return STATUS_ERR;
  }

  if (MrbsBookingSend(&booking) == STATUS_ERR) {
    MrbsBookingDestroy(&booking);
    ERR("could not send booking");
    return STATUS_ERR;
  }

  MrbsBookingDestroy(&booking);

  return STATUS_OK;
}

// int MrfsCreateFile(file_t *outfile, FILE *infile) {
//   uint8_t byte_buffer[MRFS_BYTES_PER_CHUNK];
//   uint16_t bytes_read = 0;
//   char char_buffer[MRFS_CHARS_PER_CHUNK];

//   outfile->version = MRFS_VERSION;

//   while (!feof(infile)) {
//     if (ferror(infile)) {
//       return STATUS_ERR;
//     }

//     memset(byte_buffer, 0, MRFS_BYTES_PER_CHUNK);
//     memset(char_buffer, '\0', MRFS_CHARS_PER_CHUNK);

//     bytes_read =
//         fread(byte_buffer, sizeof(uint8_t), MRFS_BYTES_PER_CHUNK, infile);

//     Base64Encode(byte_buffer, bytes_read, char_buffer);

//     printf("%s\n", char_buffer);
//   }

//   return 0;
// }

// simply returns the file size of a booking
int MrfsFile_GetSize(mrbs_credentials_t *credentials, nid_t nid) {
  mrbs_booking_t booking = {0};

  if (MrbsBookingGet(&booking, credentials, nid.year, nid.month, nid.day,
                     nid.hour, nid.min,
                     MrbsReverseRoomLookup(nid.room)) != STATUS_OK) {
    MrbsBookingDestroy(&booking);
    return 0;
  }

  char *message = calloc(MRFS_BYTES_PER_CHUNK, sizeof(uint8_t));
  if (message == NULL) {
    MrbsBookingDestroy(&booking);
    return 0;
  }

  Base64Decode(booking.description, (uint8_t *)message);
  // TODO check for errors

  int num_bookings, filesize;
  if (sscanf(message, "%d\n%d\n", &num_bookings, &filesize) != 2) {
    MrbsBookingDestroy(&booking);
    return 0;
  }

  (void)num_bookings;
  MrbsBookingDestroy(&booking);
  return filesize;
}

int MrfsFile_Read(mrbs_credentials_t *credentials, nid_t nid, uint8_t **data,
                  size_t *size) {
  mrbs_booking_t inode = {0};
  int num_bookings;

  if (MrbsBookingGet(&inode, credentials, nid.year, nid.month, nid.day,
                     nid.hour, nid.min,
                     MrbsReverseRoomLookup(nid.room)) != STATUS_OK) {
    MrbsBookingDestroy(&inode);
    return STATUS_ERR;
  }

  char *message = calloc(MRFS_BYTES_PER_CHUNK + 1, sizeof(uint8_t));
  if (message == NULL) {
    ERR("Could not allocate memory for message.");
    MrbsBookingDestroy(&inode);
    return STATUS_ERR;
  }
  Base64Decode(inode.description, (uint8_t *)message);

  // printf("Description: %s\n", message);

  char *sol = NULL, *eol = NULL;

  sol = message;
  eol = strchr(sol, '\n');
  if (eol == NULL) {
    ERR("Could not find the first line delimiter.");
    free(message);
    MrbsBookingDestroy(&inode);
    return STATUS_ERR;
  }
  *eol = '\0';
  num_bookings = atoi(sol);

  sol = eol + 1;
  eol = strchr(sol, '\n');
  if (eol == NULL) {
    eol = sol + strlen(sol);
    if (eol == sol) {
      ERR("Could not find the second line delimiter.");
      MrbsBookingDestroy(&inode);
      free(message);
      return STATUS_ERR;
    }
  } else {
    *eol = '\0';
  }
  *size = atoi(sol);
  *data = calloc(*size, sizeof(uint8_t));
  if (*data == NULL) {
    ERR("Could not allocate memory");
    MrbsBookingDestroy(&inode);
    free(message);
    return STATUS_ERR;
  }
  size_t data_offset = 0;
  // printf("File is %lu bytes\n", *size);

  for (int i = 0; i < num_bookings; ++i) {
    mrbs_booking_t dbooking = {0};
    nid_t nid = {0};

    sol = eol + 1;
    if (*sol == '\0') {
      break;
    }
    eol = strchr(sol, '\n');
    if (eol == NULL) {
      eol = sol + strlen(sol);
    } else {
      *eol = '\0';
    }

    if (MrfsParseNid(sol, &nid) == STATUS_ERR) {
      ERR("Could not parse nid.");
      MrbsBookingDestroy(&inode);
      free(message);
      return STATUS_ERR;
    }

    if (MrbsBookingGet(&dbooking, credentials, nid.year, nid.month, nid.day,
                       nid.hour, nid.min,
                       MrbsReverseRoomLookup(nid.room)) == STATUS_ERR) {
      ERR("Could not get booking.");
      MrbsBookingDestroy(&inode);
      free(message);
      return STATUS_ERR;
    }

    unsigned n = Base64DecodedLength(dbooking.description);
    // printf("Decoding %u bytes\n", n);
    Base64Decode(dbooking.description, &(*data)[data_offset]);
    data_offset += n;

    MrbsBookingDestroy(&dbooking);
  }

  free(message);
  MrbsBookingDestroy(&inode);
  return STATUS_OK;
}

int MrfsFile_Create(mrbs_credentials_t *credentials, uint8_t *src, size_t size,
                    mrbs_booking_t *out_inode) {
  size_t i;

  // Step 1:
  // Determine how many data chunks are required for the file.
  size_t number_of_data_bookings =
      (size_t)ceil((float)size / (float)MRFS_BYTES_PER_CHUNK);
  // size_t number_of_inode_bookings = 1; // TODO: determine if we need more
  // inodes size_t number_of_bookings =
  //     number_of_data_bookings + number_of_inode_bookings;

  printf("number of data bookings = ceil(%lu / %d) = %lu\n", size,
         MRFS_BYTES_PER_CHUNK, number_of_data_bookings);

  mrbs_booking_t *data_bookings = NULL;
  data_bookings = calloc(number_of_data_bookings, sizeof(mrbs_booking_t));

  if (data_bookings == NULL) {
    ERR("Could not allocate space for bookings.");
    return STATUS_ERR;
  }

  // Step 2:
  // Reserve the number of bookings to hold the data and the inode.
  out_inode->credentials = credentials;
  out_inode->duration = MRBS_DUR_30_MINS;
  if (MrbsGetFreeBooking(out_inode, NULL, MRBS_ROOM_NONE) == STATUS_ERR) {
    ERR("Could not find an available booking for the inode.");
    free(data_bookings);
    return STATUS_ERR;
  }
  if (MrbsBookingReserve(out_inode) == STATUS_ERR) {
    ERR("Could not reserve the inode booking.");
    free(data_bookings);
    return STATUS_ERR;
  }

  printf("Reserved booking ");
  MrbsPrintBookingDetails(out_inode, stdout);
  printf(" for inode.\n");

  mrbs_time_t latest_time = out_inode->time;
  mrbs_room_t latest_room = out_inode->room;
  for (i = 0; i < number_of_data_bookings; ++i) {
    data_bookings[i].credentials = credentials;
    data_bookings[i].duration = MRBS_DUR_30_MINS;

    if (MrbsGetFreeBooking(&data_bookings[i], &latest_time, latest_room) ==
        STATUS_ERR) {
      ERR("Could not find an available booking for the data node.");
      free(data_bookings);
      return STATUS_ERR;
    }
    if (MrbsBookingReserve(&data_bookings[i]) == STATUS_ERR) {
      ERR("Could not reserve the data booking.");
      free(data_bookings);
      return STATUS_ERR;
    }

    printf("Reserved booking ");
    MrbsPrintBookingDetails(&data_bookings[i], stdout);
    printf(" for data.\n");

    latest_time = data_bookings[i].time;
    latest_room = data_bookings[i].room;
  }

  // Step 3:
  // Create the inode booking with references to the data bookings
  char name[MRFS_NAME_LEN] = {0};
  char description[MRFS_BYTES_PER_CHUNK] = {0};
  char descriptionB64[MRFS_CHARS_PER_CHUNK + 1] = {
      0}; // add one for the null terminator

  snprintf((char *)name, MRFS_NAME_LEN, "MRFSI-%04d-%02d-%02d-%02d-%02d-%04d",
           out_inode->time.year, out_inode->time.month, out_inode->time.day,
           out_inode->time.hour, out_inode->time.minute,
           MrbsRoomLookup(out_inode->room));

  int chars_written = 0;

  // the first line contains the number of data bookings
  // the seconds line contains the total file size in bytes
  chars_written += snprintf((char *)&description[chars_written],
                            MRFS_BYTES_PER_CHUNK - chars_written - 1,
                            "%lu\n%lu\n", number_of_data_bookings, size);

  for (i = 0; i < number_of_data_bookings; ++i) {
    chars_written += snprintf(
        (char *)&description[chars_written],
        MRFS_BYTES_PER_CHUNK - chars_written - 1,
        "MRFSF-%04d-%02d-%02d-%02d-%02d-%04d\n", data_bookings[i].time.year,
        data_bookings[i].time.month, data_bookings[i].time.day,
        data_bookings[i].time.hour, data_bookings[i].time.minute,
        MrbsRoomLookup(data_bookings[i].room));
  }

  if (Base64Encode((uint8_t *)description, strlen(description),
                   descriptionB64) != STATUS_OK) {
    free(data_bookings);
    ERR("Could not base64 encode the inode.");
    return STATUS_ERR;
  }

  out_inode->name = (char *)name;
  out_inode->description = descriptionB64;

  // delete the reserved booking
  if (MrbsBookingDelete(out_inode) == STATUS_ERR) {
    ERR("Could not delete the reserved inode booking\n");
    free(data_bookings);
    return STATUS_ERR;
  }

  // send the new booking
  if (MrbsBookingSend(out_inode) == STATUS_ERR) {
    ERR("Could not create the booking inode\n");
    free(data_bookings);
    return STATUS_ERR;
  }

  printf("Created booking ");
  MrbsPrintBookingDetails(out_inode, stdout);
  printf(" for inode.\n");

  // Create the data bookings:
  for (i = 0; i < number_of_data_bookings; ++i) {
    memset(descriptionB64, '\0', (MRFS_CHARS_PER_CHUNK + 1) * sizeof(char));
    memset(name, '\0', MRFS_NAME_LEN * sizeof(char));

    // set the name
    snprintf((char *)name, MRFS_NAME_LEN, "MRFSF-%04d-%02d-%02d-%02d-%02d-%04d",
             data_bookings[i].time.year, data_bookings[i].time.month,
             data_bookings[i].time.day, data_bookings[i].time.hour,
             data_bookings[i].time.minute,
             MrbsRoomLookup(data_bookings[i].room));
    data_bookings[i].name = (char *)name;

    // set the description
    size_t offset = i * MRFS_BYTES_PER_CHUNK;
    size_t len = size > (1 + i) * MRFS_BYTES_PER_CHUNK
                     ? MRFS_BYTES_PER_CHUNK
                     : (size - i * MRFS_BYTES_PER_CHUNK);
    printf("Chunk %lu/%lu has length %lu at offset %lu\n", i + 1,
           number_of_data_bookings, len, offset);
    if (Base64Encode(&src[offset], len, descriptionB64) != STATUS_OK) {
      free(data_bookings);
      ERR("Could not base64 encode the file data.");
      return STATUS_ERR;
    }
    data_bookings[i].description = descriptionB64;

    // printf("Chunk data is %s\n", descriptionB64);

    // delete the reserved booking
    if (MrbsBookingDelete(&data_bookings[i]) != STATUS_OK) {
      free(data_bookings);
      ERR("Could not delete the reserved data booking");
      return STATUS_ERR;
    }

    // send the new data booking
    if (MrbsBookingSend(&data_bookings[i]) != STATUS_OK) {
      free(data_bookings);
      ERR("Could not send the reserved data booking");
      return STATUS_ERR;
    }
  }

  free(data_bookings);
  return STATUS_OK;
}

// uint8_t *MrfsFileRead(mrbs_booking_t *booking, size_t *n) {
//   uint8_t *data = NULL;

//   // Mrbs

//   return data;
// }

// void MrfsFile_Delete(mrbs_booking_t *booking) {

// Step 1:
// Read the booking

// Step 2:
// Decode the booking

// Step 3:
// Delete each data booking

// Step 4:
// Delete the inode booking
// }

void print_nid(const nid_t *nid) {
  printf("NID@%p: {\n\t.year = %d,\n\t.month = %d,\n\t.day = %d,\n\t.hour = "
         "%d,\n\t.min = %d,\n\t.room = %d\n}\n",
         (void *)nid, nid->year, nid->month, nid->day, nid->hour, nid->min,
         nid->room);
}

int MrfsParseNid(const char *str, nid_t *nid) {
  char type = '\0';
  if (sscanf(str, "MRFS%c-%hu-%hhu-%hhu-%hhu-%hhu-%hd", &type, &nid->year,
             &nid->month, &nid->day, &nid->hour, &nid->min, &nid->room) != 7) {
    return STATUS_ERR;
  } else {
    return STATUS_OK;
  }
}