/**
 * @file   files.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 22:55:10 2009
 *
 * @brief  Implements utility functions declared in utils/files.h.
 *
 *
 */

#include <stdbool.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>

#include "utils/files.h"

int
xopen(const char *path, int oflag, ...)
{
  // do we need to take yet another parameter specifying permissions
  bool    has_mode = oflag & O_CREAT;

  // initialized to suppress "possibly uninitialized" warning
  // this value is never used
  mode_t  mode = 0;
  va_list arglist;
  int     ret;

  va_start(arglist, oflag);

  if (has_mode) {
    mode = va_arg(arglist, mode_t);
  }

  va_end(arglist);

  while (true) {
    if (has_mode) {
      ret = open(path, oflag, mode);
    } else {
      ret = open(path, oflag);
    }

    if (ret < 0 && errno == EINTR) {
      continue;
    } else {
      return ret;
    }
  }

}

int
xclose(int fd)
{
  int ret;

  while (true) {
    ret = close(fd);
    if (ret < 0 && errno == EINTR) {
      continue;
    } else {
      return ret;
    }
  }
}

ssize_t
xread(int fd, void *buf, size_t count)
{
  ssize_t nread = 0;
  ssize_t ret;

  while (nread < count) {
    ret = read(fd, buf, count);

    if (ret > 0) {
      nread += ret;
    } else if (ret < 0) {
      if (errno == EINTR) {
        continue;
      } else {
        return ret;
      }
    } else {
      /* ret is zero */
      break;
    }
  }

  return nread;
}

ssize_t
xwrite(int fd, const void *buf, size_t count)
{
  ssize_t nwritten = 0;
  ssize_t ret;

  while (nwritten < count) {
    ret = write(fd, buf, count);

    if (ret >= 0) {
      nwritten += ret;
    } else {
      if (errno == EINTR) {
        continue;
      } else {
        return ret;
      }
    }
  }

  return nwritten;
}
