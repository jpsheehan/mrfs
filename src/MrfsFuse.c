#include <asm-generic/errno-base.h>
#include <fcntl.h>
#include <string.h>
#define FUSE_USE_VERSION 31
#define _FILE_OFFSET_BITS 64

#include <errno.h>
#include <fuse/fuse.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include ".env.h"
#include "Mrbs.h"
#include "Mrfs.h"
#include "utils.h"

#define unused __attribute__((unused))

directory_t dir = {0};
mrbs_credentials_t creds = {.username = USERNAME, .password = PASSWORD};
nid_t nid = {0};

static int _getattr(const char *path, struct stat *st) {

  st->st_uid = getuid();
  st->st_gid = getgid();
  st->st_atime = time(NULL);
  st->st_mtime = time(NULL);

  if (strcmp("/", path) == 0) {
    // handle the root directory case
    // st->st_blksize = 512;
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
  } else {
    for (int i = 0; i < dir.num_files; ++i) {
      if (strcmp(dir.file_refs[i].name, path + 1) == 0) {
        st->st_mode = S_IFREG | 0644;
        st->st_nlink = 1;
        st->st_size = MrfsFile_GetSize(&creds, dir.file_refs[i].id);
        return 0;
      }
    }
    // handle the case of an unknown file/dir
    return -ENOENT;
  }

  return 0;
}

// static void *_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
//   return NULL;
// }

static int _read(const char *path, char *buffer, size_t size, off_t offset,
                 struct fuse_file_info *fi unused) {
  for (int i = 0; i < dir.num_files; ++i) {
    if (strcmp(path + 1, dir.file_refs[i].name) == 0) {
      uint8_t *data = NULL;
      size_t total_size = 0;

      if (MrfsFile_Read(&creds, dir.file_refs[i].id, &data, &total_size) ==
          STATUS_ERR) {
        return 0;
      }

      if (total_size <= size) { // TODO: is my thinking correct?
        memcpy(buffer, &data[offset], total_size - offset);
        return total_size - offset;
      } else {
        memcpy(buffer, &data[offset], size);
        return size;
      }
    }
  }
  return 0;
}

static int _readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                    off_t offset unused, struct fuse_file_info *fi unused) {

  // make sure that we're only handling the root directory for this project
  if (strcmp("/", path) != 0) {
    return -ENOENT;
  }

  // we need these for some reason
  filler(buffer, ".", NULL, 0);
  filler(buffer, "..", NULL, 0);

  for (int i = 0; i < dir.num_files; ++i) {
    if (dir.file_refs[i].name) {
      filler(buffer, dir.file_refs[i].name, NULL, 0);
    }
  }

  return 0;
}

static int _open(const char *path unused, struct fuse_file_info *fi unused) {
  for (int i = 0; i < dir.num_files; ++i) {
    if (strcmp(path + 1, dir.file_refs[i].name) == 0) {
      fi->fh = (unsigned long)&dir.file_refs[i].id;
      return 0;
    }
  }

  if ((fi->flags & 3) != O_RDONLY) {
    return -EACCES;
  }

  return -ENOENT;
}

static int _create(const char *path unused, mode_t mode unused,
                   struct fuse_file_info *fi unused) {
  // TODO: Create an new and empty file
  // TODO: Add this file to the directory
  return 0;
}

static int _write(const char *path unused, const char *src unused, size_t size,
                  off_t offset unused, struct fuse_file_info *fi unused) {
  return size;
}

static const struct fuse_operations ops = {
    .getattr = _getattr,
    .readdir = _readdir,
    .read = _read,
    .write = _write,
    .open = _open,
    .create = _create,
};

static void print_usage(const char *exe);

int main(int argc, char *argv[]) {

  if (argc < 8) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char *mountpoint = argv[1];

  nid.year = atoi(argv[2]);
  nid.month = atoi(argv[3]);
  nid.day = atoi(argv[4]);
  nid.hour = atoi(argv[5]);
  nid.min = atoi(argv[6]);
  nid.room = MrbsReverseRoomLookup(atoi(argv[7]));

  if (nid.room == STATUS_ERR) {
    printf("Invalid room id :(\n");
    return EXIT_FAILURE;
  }

  printf("Attempting to mount the booking at %04d-%02d-%02d %02d:%02d in room "
         "%d to the directory \"%s\"...",
         nid.year, nid.month, nid.day, nid.hour, nid.min, atoi(argv[7]),
         mountpoint);

  if (MrfsDirectoryRead(&dir, &creds, &nid) == STATUS_OK) {
    printf(" Okay!\n");

    for (int i = 0; i < dir.num_files; ++i) {
      printf("- %s\n", dir.file_refs[i].name);
    }
  } else {
    printf(" Not found!\n");
    printf("Attempting to create new directory...");

    dir.num_files = 8;
    dir.file_refs[0].name = strdup("Makefile");
    dir.file_refs[0].id = (nid_t){.year = 1901,
                                  .month = 12,
                                  .day = 14,
                                  .hour = 10,
                                  .min = 00,
                                  .room = 215};

    dir.file_refs[1].name = strdup("Makefile1");
    dir.file_refs[1].id = dir.file_refs[0].id;
    dir.file_refs[2].name = strdup("Makefile2");
    dir.file_refs[2].id = dir.file_refs[0].id;
    dir.file_refs[3].name = strdup("Makefile3");
    dir.file_refs[3].id = dir.file_refs[0].id;
    dir.file_refs[4].name = strdup("Makefile4");
    dir.file_refs[4].id = dir.file_refs[0].id;
    dir.file_refs[5].name = strdup("Makefile5");
    dir.file_refs[5].id = dir.file_refs[0].id;
    dir.file_refs[6].name = strdup("Makefile6");
    dir.file_refs[6].id = dir.file_refs[0].id;
    dir.file_refs[7].name = strdup("Makefile7");
    dir.file_refs[7].id = dir.file_refs[0].id;

    if (MrfsDirectoryWrite(&dir, &creds, &nid) == STATUS_OK) {
      printf(" Okay!\n");
    } else {
      printf(" Not created!\n");
      printf("Check that there is not an existing booking at the location!\n");
      return EXIT_FAILURE;
    }
  }

  const int fuse_argc = 3;
  char *fuse_argv[4] = {(char *)&argv[0], (char *)mountpoint, "-f", NULL};
  (void)ops;
  (void)fuse_argc;
  (void)fuse_argv;

  return fuse_main(fuse_argc, fuse_argv, &ops, NULL);

  return 0;
}

void print_usage(const char *exe) {
  printf("Usage: %s mountpoint year month day hour minute room\n", exe);
}