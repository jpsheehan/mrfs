#include ".env.h"
#include "Mrbs.h"
#include "Mrfs.h"
#include "utils.h"

int main() {
  directory_t d = {
      .version = 1,
  };
  nid_t nid = {.year = 2020,
               .month = 8,
               .day = 9,
               .hour = 10,
               .min = 30,
               .room = MRBS_ROOM_212A};
  mrbs_credentials_t credentials = {.username = USERNAME, .password = PASSWORD};

  INFO("Started main function");

  if (MrfsDirectoryWrite(&d, &credentials, &nid) == STATUS_ERR) {
    WARN("could not create directory");
  } else {
    INFO("created directory");
  }

  // clear the directory
  d = (directory_t){0};
  if (MrfsDirectoryRead(&d, &credentials, &nid) == STATUS_ERR) {
    ERR("could not read directory");
  } else {
    INFO("read directory successfully");
  }

  return 0;
}
