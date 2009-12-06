/**
 * @file   fs_object.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Nov 10 00:52:24 2009
 *
 * @brief  File system objects' functionality implementation.
 *
 *
 */

#include <stdlib.h>
#include <string.h>

#define EXTERN_INLINE_DEFINITIONS
#include "fat32/fs_object.h"
#undef  EXTERN_INLINE_DEFINITIONS

#include "fat32/direntry.h"

struct fat32_fs_object_t *
fat32_fs_object_root_dir(const struct fat32_fs_t *fs)
{
  struct fat32_fs_object_t *fs_object;

  fs_object = malloc(sizeof(struct fat32_fs_object_t));
  if (fs_object == NULL) {
    return NULL;
  }

  fs_object->type     = FAT32_FS_OBJECT_ROOT_DIR;
  fs_object->name     = NULL;
  fs_object->direntry = NULL;
  fs_object->fs       = fs;

  return fs_object;
}

struct fat32_fs_object_t *
fat32_fs_object_direntry(const struct fat32_fs_t       *fs,
                         const struct fat32_direntry_t *direntry,
                         const char *name)
{
  struct fat32_fs_object_t *fs_object;

  fs_object = malloc(sizeof(struct fat32_fs_object_t));
  if (fs_object == NULL) {
    return NULL;
  }

  fs_object->type     = FAT32_FS_OBJECT_DIR;
  fs_object->name     = NULL;
  fs_object->direntry = NULL;
  fs_object->fs       = fs;

  fs_object->name     = strdup((char *) direntry->name);
  if (fs_object->name == NULL) {
    goto cleanup;
  }

  fs_object->direntry = malloc(sizeof(struct fat32_direntry_t));
  if (fs_object->direntry == NULL) {
    goto cleanup;
  }

  return fs_object;

cleanup:
  if (fs_object->name != NULL) {
    free(fs_object->name);
  }

  free(fs_object);

  return NULL;
}

void
fat32_fs_object_free(struct fat32_fs_object_t *fs_object)
{
  if (fs_object->name != NULL) {
    free(fs_object->name);
  }

  free(fs_object);
}

uint32_t
fat32_fs_object_first_cluster(const struct fat32_fs_object_t *fs_object)
{
  if (fs_object->type == FAT32_FS_OBJECT_ROOT_DIR) {
    return fs_object->fs->bpb->root_cluster;
  } else {
    const struct fat32_direntry_t *direntry = fs_object->direntry;

    uint32_t low     = direntry->first_cluster_lo;
    uint32_t high    = direntry->first_cluster_hi;
    uint32_t cluster = (high << 16) | low;

    return cluster;
  }
}
