/**
 * @file   diriter.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Nov 10 00:46:34 2009
 * 
 * @brief  Directory iterators implementation.
 * 
 * 
 */
#include <assert.h>
#include <stdlib.h>

#include "fat32/diriter.h"

struct fat32_diriter_t *
fat32_diriter_create(const struct fat32_fs_object_t *fs_object)
{
  assert( (fs_object->type == FAT32_FS_OBJECT_DIR) ||
	  (fs_object->type == FAT32_FS_OBJECT_ROOT_DIR) );

  struct fat32_diriter_t *diriter;

  diriter = malloc(sizeof(struct fat32_diriter_t));
  if (diriter == NULL) {
    return NULL;
  }

  diriter->fs      = fs_object->fs;
  diriter->cluster = fat32_fs_object_first_cluster(fs_object);
  diriter->offset  = 0;

  return diriter;
}
