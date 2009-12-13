/**
 * @file   fs_object.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Nov 10 00:52:24 2009
 *
 * @brief  File system objects' functionality implementation.
 *
 * @todo Split truncate function into several.
 */

#include <stdlib.h>
#include <string.h>

#define EXTERN_INLINE_DEFINITIONS
#include "fat32/fs_object.h"
#undef  EXTERN_INLINE_DEFINITIONS

#include "fat32/diriter.h"

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

enum fat32_error_t
fat32_fs_object_is_empty_directory(const struct fat32_fs_object_t *fs_object,
                                   bool *result)
{
  assert( fat32_fs_object_is_directory(fs_object) );

  struct fat32_diriter_t *diriter = fat32_diriter_create(fs_object, false);
  if (diriter == NULL) {
    return FE_ERRNO;
  }

  struct fat32_fs_object_t *child;
  enum fat32_error_t ret = fat32_diriter_next(diriter, &child);
  if (ret != FE_OK) {
    return ret;
  }

  if (child == NULL) {
    *result = true;
  } else {
    *result = false;
    fat32_fs_object_free(child);
  }

  return FE_OK;
}

enum fat32_error_t
fat32_fs_object_delete(struct fat32_fs_object_t *fs_object)
{
  struct fat32_fat_t *fat = fs_object->fs->fat;
  enum fat32_error_t  ret;

  uint32_t cluster = fat32_fs_object_first_cluster(fs_object);
  ret = fat32_fs_object_mark_free(fs_object);
  if (ret != FE_OK) {
    return ret;
  }

  if (!(fat32_fs_object_is_file(fs_object) &&
        fat32_fs_object_is_empty_file(fs_object))) {
    enum fat32_error_t ret = fat32_fat_mark_cluster_chain_free(fat, cluster);
    switch (ret) {
    case FE_OK:
      break;
    case FE_ERRNO:
    case FE_FS_INCONSISTENT:
    case FE_INVALID_FS:
       /* We treat FE_FS_INCONSISTENT that can be returned by
       * ::fat32_fat_mark_cluster_chain_free as FE_FS_PARTIALLY_CONSISTENT
       * state. It's because in such situation some FAT entry has not been
       * set correctly but its corresponding cluster is not referenced by any
       * directory entry. So we do for FE_INVALID_FS error. */
      return FE_FS_PARTIALLY_CONSISTENT;
    case FE_INVALID_DEV:
      return FE_INVALID_DEV;
    default:
      assert( false );
    }
  }
  return FE_OK;
}

enum fat32_error_t
fat32_fs_object_truncate(struct fat32_fs_object_t *fs_object,
                         uint32_t length)
{
  assert( fat32_fs_object_is_file(fs_object) );

  const struct fat32_fs_t *fs      = fs_object->fs;
  struct fat32_fat_t      *fat     = fs->fat;
  uint32_t                 csize   = fs->cluster_size;
  uint32_t                 fsize   = fat32_fs_object_size(fs_object);
  uint32_t                 cluster = fat32_fs_object_first_cluster(fs_object);
  uint32_t                 next;

  /* number of cluster needed for the resized file */
  uint32_t clusters = (length + csize - 1) / csize;


  if (length < fsize) {
    fat32_fat_entry_t entry;
    enum fat32_error_t       ret;
    if (clusters == 0) {
      next = cluster;
      ret = fat32_direntry_make_empty(fs_object->direntry, fs->fd,
                                      fs_object->offset);
      switch (ret) {
      case FE_OK:
        break;
      case FE_ERRNO:
      case FE_FS_INCONSISTENT:
        return ret;
      default:
        assert( false );
      }
    } else {
      ret = fat32_fat_get_nth_entry(fat, cluster, clusters - 1, &entry);
      switch (ret) {
      case FE_OK:
        break;
      case FE_ERRNO:
      case FE_INVALID_DEV:
      case FE_INVALID_FS:
        return ret;
      case FE_CLUSTER_CHAIN_ENDED:
        /* by construction FE_CLUSTER_CHAIN_ENDED can't be returned */
        return FE_INVALID_FS;
      default:
        assert( false );
      }
      cluster = fat32_fat_entry_to_cluster(entry);
      ret     = fat32_fat_get_entry(fat, cluster, &entry);
      switch (ret) {
      case FE_OK:
        break;
      case FE_ERRNO:
      case FE_INVALID_DEV:
        return ret;
      default:
        assert( false );
      }
      next    = fat32_fat_entry_to_cluster(entry);

      ret = fat32_fat_mark_cluster_last(fat, cluster);
      switch (ret) {
      case FE_OK:
        break;
      case FE_FS_INCONSISTENT:
        return ret;
      default:
        assert( false );
      }
    }

    ret = fat32_fat_mark_cluster_chain_free(fat, next);
    switch (ret) {
    case FE_OK:
      return FE_OK;
    case FE_ERRNO:
    case FE_FS_INCONSISTENT:
    case FE_INVALID_FS:
      return FE_FS_PARTIALLY_CONSISTENT;
    case FE_INVALID_DEV:
      return FE_INVALID_DEV;
    default:
      assert( false );
    }
  } else if (length > fsize) {
    assert( false );
  } else {
    /* length == fsize */
    return FE_OK;
  }
}
