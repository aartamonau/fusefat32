/**
 * @file   files.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 22:55:10 2009
 * 
 * @brief  Implements utility functions declared in
 * @link utils/files.h @endlink.
 * 
 * 
 */
#include <errno.h>

#include "utils/files.h"

int
xclose(int fd)
{
  int res;

  while (true) {
    res = close(fd);
    if (res < 0 && errno == EINTR) {
      continue;
    } else {
      return res;
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

    if (ret > 0) {
      nwritten += ret;
    } else if (ret < 0) {
      if (errno == EINTR) {
        continue;
      } else {
        return ret;
      }
    } else {
      break;
    }
  }

  return nwritten;
}
