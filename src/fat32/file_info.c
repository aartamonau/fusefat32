/**
 * @file   file_info.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Dec 12 12:09:56 2009
 *
 * @brief  Information about open files.
 *
 * @todo: move to fusefat32 directory
 *
 *
 */
#include <stdlib.h>

#include "fat32/file_info.h"

void *
fat32_file_info_cloner(const void *file_info)
{
  struct fat32_file_info_t *result = malloc(sizeof(struct fat32_file_info_t));
  if (result == NULL) {
    return NULL;
  }
  result->refs    = 1;
  result->deleted = false;

  return result;
}
