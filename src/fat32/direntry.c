/**
 * @file   direntry.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Dec  5 17:00:52 2009
 *
 * @brief  Directory entries related functionality.
 *
 *
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <sys/types.h>
#include <unistd.h>

#include "utils/files.h"

#define EXTERN_INLINE_DEFINITIONS
#include "fat32/direntry.h"

/// a marker of free direntry
static const uint8_t EMPTY = 0xe5;

/// a marker of last direntry
static const uint8_t LAST  = 0x00;

bool
fat32_direntry_is_free(const struct fat32_direntry_t *direntry)
{
  const uint8_t *name = direntry->name;

  return (name[0] == EMPTY || name[0] == LAST);
}

bool
fat32_direntry_is_last(const struct fat32_direntry_t *direntry)
{
  const uint8_t *name = direntry->name;

  return (name[0] == LAST);
}

/// ASCII-code of space
const char SPACE = 0x20;

char *
fat32_direntry_short_name(const struct fat32_direntry_t *direntry)
{
  const uint8_t *p = direntry->name + (FAT32_DIRENTRY_NAME_SIZE - 1);
  size_t base_name_size = 0;
  size_t ext_size       = 0;

  for (int i = 0; i < FAT32_DIRENTRY_EXTENSION_SIZE; i++, p--) {
    if (*p != SPACE) {
      ext_size += FAT32_DIRENTRY_EXTENSION_SIZE - i;
      break;
    }
  }

  p = direntry->name + FAT32_DIRENTRY_BASE_NAME_SIZE - 1;

  for (int i = 0; i < FAT32_DIRENTRY_BASE_NAME_SIZE; i++, p--) {
    if (*p != SPACE) {
      base_name_size += FAT32_DIRENTRY_BASE_NAME_SIZE - i;
      break;
    }
  }

  /* + 1 is for trailing zero */
  size_t name_size = base_name_size + ext_size + 1;
  if (ext_size != 0) {
    /* + 1 is for point */
    name_size += 1;
  }

  char *result = malloc(name_size);
  if (result == NULL) {
    return NULL;
  }
  result[name_size - 1] = '\0';

  memcpy(result, direntry->name, base_name_size);

  if (ext_size != 0) {
    result[base_name_size] = '.';
    memcpy(result + base_name_size + 1,
           direntry->name + FAT32_DIRENTRY_BASE_NAME_SIZE,
           ext_size);
  }

  return result;
}

enum fat32_error_t
fat32_direntry_mark_free(int fd, off_t offset)
{
  const off_t inner_offset = offsetof(struct fat32_direntry_t, name[0]);
  offset += inner_offset;

  if (lseek(fd, offset, SEEK_SET) == (off_t) -1) {
    return FE_ERRNO;
  }

  ssize_t nwritten = xwrite(fd, &EMPTY, sizeof(uint8_t));
  if (nwritten == -1) {
    /* As sizeof(typeof(EMPTY)) is 1 we can assume that nothing is written when
     * error occurs */
    return FE_ERRNO;
  }

  return FE_OK;
}

bool
fat32_direntry_is_dot(const struct fat32_direntry_t *direntry)
{
  bool dot    = direntry->name[0] == '.';
  bool dotdot = dot && direntry->name[1] == '.';

  return dot || dotdot;
}
