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

#include "utils/files.h"

#include "fat32/diriter.h"
#include "fat32/direntry.h"
#include "fat32/errors.h"
#include "fat32/utils.h"

struct fat32_diriter_t *
fat32_diriter_create(const struct fat32_fs_object_t *fs_object,
                     bool list_dots)
{
  assert( (fs_object->type == FAT32_FS_OBJECT_DIR) ||
          (fs_object->type == FAT32_FS_OBJECT_ROOT_DIR) );

  struct fat32_diriter_t   *diriter;

  diriter = malloc(sizeof(struct fat32_diriter_t));
  if (diriter == NULL) {
    return NULL;
  }

  diriter->fs           = fs_object->fs;
  diriter->cluster      = fat32_fs_object_first_cluster(fs_object);
  diriter->offset       = 0;
  diriter->list_dots    = list_dots;

  return diriter;
}

/**
 * Checks whether directory entry is of interest for ::fat32_diriter_next.
 *
 * @param direntry  Directory entry.
 * @param list_dots Specifies whether to treat dot and dotdot as suitable
 *        entries.
 *
 * @return boolean result
 */
static bool
suitable_direntry(const struct fat32_direntry_t *direntry,
                  bool list_dots)
{
  if (fat32_direntry_is_last(direntry)) {
    return true;
  }

  bool result = (!fat32_direntry_is_free(direntry) &&
                 (fat32_direntry_is_directory(direntry) ||
                  fat32_direntry_is_file(direntry)));
  if (list_dots) {
    return result;
  } else {
    return result && !fat32_direntry_is_dot(direntry);
  }
}

enum fat32_error_t
fat32_diriter_next(struct fat32_diriter_t    *diriter,
                   struct fat32_fs_object_t **fs_object)
{
  assert( diriter->cluster != 0 );

  struct fat32_direntry_t  direntry;
  const struct fat32_fs_t *fs = diriter->fs;
  off_t offset;

  /* returned when there are no more fs objects */
  *fs_object = NULL;

  do {
    if (diriter->offset == diriter->fs->cluster_size) {
      diriter->offset = 0;

      uint32_t cluster = diriter->cluster;
      fat32_fat_entry_t entry;

      do {
        enum fat32_error_t ret =
          fat32_fat_get_entry(fs->fat, cluster, &entry);

        if (ret != FE_OK) {
          return ret;
        }

        cluster = fat32_fat_entry_to_cluster(entry);
      } while (fat32_fat_entry_is_bad(entry));

      if (fat32_fat_entry_is_null(entry)) {
        /* end of cluster chain */
        diriter->cluster = 0;
        return FE_OK;
      } else {
        diriter->cluster = cluster;
      }
    }

    off_t cluster_offset = fat32_cluster_to_offset(fs->bpb, diriter->cluster);
    offset               = cluster_offset + diriter->offset;

    if (lseek(fs->fd, offset, SEEK_SET) == (off_t) -1) {
      return FE_ERRNO;
    }

    int nread = xread(fs->fd, &direntry, sizeof(struct fat32_direntry_t));
    if (nread == -1) {
      return FE_ERRNO;
    } else if (nread < sizeof(struct fat32_direntry_t)) {
      return FE_INVALID_DEV;
    }

    diriter->offset += sizeof(struct fat32_direntry_t);

  } while (!suitable_direntry(&direntry, diriter->list_dots));

  if (fat32_direntry_is_last(&direntry)) {
    diriter->cluster = 0;
    return FE_OK;
  }

  /* TODO: long names */
  char *direntry_name = fat32_direntry_short_name(&direntry);
  *fs_object = fat32_fs_object_direntry(fs, &direntry, direntry_name, offset);
  free(direntry_name);

  return FE_OK;
}

void
fat32_diriter_free(struct fat32_diriter_t *diriter)
{
  free(diriter);
}
