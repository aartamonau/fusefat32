/**
 * @file   fh.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Mon Dec  7 21:00:34 2009
 *
 * @brief  File handles' related functionality. The implementation.
 *
 *
 */

#include <stdlib.h>
#include <limits.h>

#include "fat32/fh.h"

unsigned int
fat32_fh_hash(const void *fh)
{
  const fat32_fh_t *value = fh;

  return *value / UINT_MAX;
}

bool
fat32_fh_equal(const void *fh_a, const void *fh_b)
{
  const fat32_fh_t *a = fh_a;
  const fat32_fh_t *b = fh_b;

  return *a == *b;
}

void *
fat32_fh_cloner(const void *fh)
{
  void *result = malloc(sizeof(fat32_fh_t));

  if (result == NULL) {
    return result;
  }

  *((fat32_fh_t *) result) = *((fat32_fh_t *) fh);

  return result;
}
