#ifndef UTILS_H_
#define UTILS_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STATUS_OK 0
#define STATUS_ERR -1

#define LOG(level, message)                                                    \
  fprintf(stderr, "%s::%s:%d in %s: %s\n", level, __FILE__, __LINE__,          \
          __func__, message)
#define INFO(message) LOG("INFO", message)
#define ERR(message) LOG("ERR", message)
#define WARN(message) LOG("WARN", message)

#endif
