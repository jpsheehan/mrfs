#ifndef MRFS_H_
#define MRFS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "Mrbs.h"

#define MRFS_VERSION 1

#define MRFS_STATUS_NONE 0x00
#define MRFS_STATUS_CREATION 0x01 // marked for creation
#define MRFS_STATUS_DELETION 0x02 // marked for deletion

#define MRFS_BYTES_PER_CHUNK 1350 // 1800
#define MRFS_CHARS_PER_CHUNK 1800 // 2400

typedef uint32_t chunk_num_t;
typedef uint32_t checksum_t;

typedef struct {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  int16_t room;
} nid_t;

typedef struct {
  nid_t file_id;
  uint8_t version;
  chunk_num_t num;
  checksum_t checksum;
  char data[MRFS_BYTES_PER_CHUNK];
} chunk_t;

typedef struct {
  nid_t id;
  chunk_num_t num;
  checksum_t checksum;
} chunk_ref_t;

typedef struct {
  uint8_t version;
  uint8_t status;
  chunk_num_t num_chunks;
  chunk_ref_t *chunk_refs;
} file_t;

typedef struct {
  nid_t id;
  char *name;
} file_ref_t;

typedef struct {
  uint8_t version;
  uint16_t num_files;
  file_ref_t file_refs[8];
} directory_t;

int MrfsFile_Create(mrbs_credentials_t *credentials, uint8_t *src, size_t size,
                    mrbs_booking_t *out_inode);

int MrfsDirectoryRead(directory_t *d, mrbs_credentials_t *credentials,
                      nid_t *nid);

int MrfsDirectoryWrite(directory_t *d, mrbs_credentials_t *credentials,
                       nid_t *nid);

int MrfsFile_GetSize(mrbs_credentials_t *credentials, nid_t nid);

int MrfsFile_Read(mrbs_credentials_t *credentials, nid_t nid, uint8_t **data,
                  size_t *size);

#endif
