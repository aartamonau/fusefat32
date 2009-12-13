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
  fs_object->offset   = 0;

  return fs_object;
}

struct fat32_fs_object_t *
fat32_fs_object_direntry(const struct fat32_fs_t       *fs,
                         const struct fat32_direntry_t *direntry,
                         const char *name, off_t offset)
{
  struct fat32_fs_object_t *fs_object;

  fs_object = malloc(sizeof(struct fat32_fs_object_t));
  if (fs_object == NULL) {
    return NULL;
  }

  if (fat32_direntry_is_directory(direntry)) {
    fs_object->type   = FAT32_FS_OBJECT_DIR;
  } else {
    fs_object->type   = FAT32_FS_OBJECT_FILE;
  }

  fs_object->name     = NULL;
  fs_object->direntry = NULL;
  fs_object->fs       = fs;
  fs_object->offset   = offset;

  fs_object->name     = strdup(name);
  if (fs_object->name == NULL) {
    goto cleanup;
  }

  fs_object->direntry = malloc(sizeof(struct fat32_direntry_t));

  if (fs_object->direntry == NULL) {
    goto cleanup;
  }
  memcpy(fs_object->direntry, direntry, sizeof(struct fat32_direntry_t));

  return fs_object;

cleanup:
  fat32_fs_object_free(fs_object);

  return NULL;
}

void
fat32_fs_object_free(struct fat32_fs_object_t *fs_object)
{
  if (fs_object->name != NULL) {
    free(fs_object->name);
  }

  if (fs_object->direntry != NULL) {
    free(fs_object->direntry);
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

void *
fat32_fs_object_cloner(const void *fs_object)
{
  const struct fat32_fs_object_t *original =
    (struct fat32_fs_object_t *) fs_object;

  struct fat32_fs_object_t *result   = malloc(sizeof(struct fat32_fs_object_t));
  if (result == NULL) {
    return NULL;
  }

  result->name     = NULL;
  result->direntry = NULL;

  result->name = strdup(original->name);
  if (result->name == NULL) {
    goto fs_object_allocator_cleanup;
  }

  if (original->direntry != NULL) {
    result->direntry = malloc(sizeof(struct fat32_direntry_t));
    if (result->direntry == NULL) {
      goto fs_object_allocator_cleanup;
    }

    memcpy(result->direntry, original->direntry,
           sizeof(struct fat32_direntry_t));
  }

  return result;
fs_object_allocator_cleanup:
  if (result->name != NULL) {
    free(result);
  }

  if (result->direntry != NULL) {
    free(result->direntry);
  }

  return NULL;
}

enum fat32_error_t
fat32_fs_object_mark_free(const struct fat32_fs_object_t *fs_object)
{
  assert( !fat32_fs_object_is_root_directory(fs_object) );

  return fat32_direntry_mark_free(fs_object->fs->fd, fs_object->offset);
}
