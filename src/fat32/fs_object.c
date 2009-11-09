#include <stdlib.h>

#include "fat32/fs_object.h"

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
