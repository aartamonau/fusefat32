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

struct fat32_fh_allocator_t *
fat32_fh_allocator_create(void)
{
  struct fat32_fh_allocator_t *allocator =
    malloc(sizeof(struct fat32_fh_allocator_t));

  if (allocator == NULL) {
    return NULL;
  }

  allocator->last_fh = 0;

  return allocator;
}

void
fat32_fh_allocator_free(struct fat32_fh_allocator_t *allocator)
{
  free(allocator);
}

bool
fat32_fh_allocate(struct fat32_fh_allocator_t *allocator,
                  fat32_fh_t *fh)
{
  if (allocator->last_fh < UINT64_MAX) {
    *fh = ++allocator->last_fh;
    return true;
  } else {
    return false;
  }
}

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
