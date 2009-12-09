/**
 * @file   fs.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 22:54:25 2009
 *
 * @brief  Implementation of functions for working with FAT32 file system.
 *
 *
 */
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "fat32/fs.h"
#include "fat32/utils.h"
#include "fat32/diriter.h"
#include "fat32/fs_object.h"
#include "fat32/fh.h"
#include "utils/files.h"

/**
 * Frees all resources allocated for the file system.
 *
 * @param fs a file system structure
 *
 * @return 0 on success, -1 otherwise
 */
static inline int
fat32_fs_cleanup(struct fat32_fs_t* fs)
{
  if (fs != NULL) {
    /* trying to close file descriptor first in order to give
       possibility to retry on error to the user
    */
    if (fs->fd != -1) {
      int ret = xclose(fs->fd);
      if (ret != 0) {
        return ret;
      }
    }

    if (fs->write_lock != NULL) {
      /* by invariant we assume that lock has been intitialized */
      int ret = pthread_mutex_destroy(fs->write_lock);
      if (ret != 0) {
        return ret;
      }

      free(fs->write_lock);
    }

    if (fs->bpb != NULL) {
      free(fs->bpb);
    }

    if (fs->fs_info != NULL) {
      free(fs->fs_info);
    }

    if (fs->fat != NULL) {
      if (fat32_fat_finalize(fs->fat) != FE_OK) {
        return -1;
      }
      free(fs->fat);
    }

    if (fs->fh_table != NULL) {
      hash_table_free(fs->fh_table);
    }

    if (fs->fh_allocator != NULL) {
      fat32_fh_allocator_free(fs->fh_allocator);
    }

    free(fs);
  }

  return 0;
}

enum fat32_error_t
fat32_fs_open(const char *path, const struct fat32_fs_params_t *params,
              struct fat32_fs_t **result)
{
  struct fat32_fs_t      *fs;
  struct fat32_bpb_t     *bpb;
  struct fat32_fs_info_t *fs_info;
  struct fat32_fat_t     *fat;
  struct stat             dev_stat;
  pthread_mutex_t        *lock;
  enum fat32_error_t      error = FE_ERRNO;

  int fd;

  fs = (struct fat32_fs_t *) malloc(sizeof(struct fat32_fs_t));
  if (fs == NULL) {
    goto open_fs_error;
  }

  *result = fs;

  fs->fd           = -1;
  fs->bpb          = NULL;
  fs->fs_info      = NULL;
  fs->write_lock   = NULL;
  fs->fat          = NULL;
  fs->fh_table     = NULL;
  fs->fh_allocator = NULL;

  lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  if (lock == NULL) {
    goto open_device_cleanup;
  }

  int ret = pthread_mutex_init(lock, NULL);
  if (pthread_mutex_init(lock, NULL) != 0) {
    /* we are deallocating memory to hold the invariant described
       at #fat32_fs_t::write_lock
    */
    free(lock);

    /* as pthread functions does not set errno we do it manually to keep
       interface uniform
    */
    errno = ret;
    goto open_device_cleanup;
  }

  bpb = (struct fat32_bpb_t *) malloc(sizeof(struct fat32_bpb_t));
  if (bpb == NULL) {
    goto open_device_cleanup;
  }
  fs->bpb = bpb;

  fs_info = (struct fat32_fs_info_t *) malloc(sizeof(struct fat32_fs_info_t));
  if (fs_info == NULL) {
    goto open_device_cleanup;
  }
  fs->fs_info = fs_info;

  fat = (struct fat32_fat_t *) malloc(sizeof(struct fat32_fat_t));
  if (bpb == NULL) {
    goto open_device_cleanup;
  }
  fs->fat = fat;

  fd = xopen(path, O_RDWR);
  if (fd < 0) {
    goto open_device_cleanup;
  }
  fs->fd = fd;

  if (fstat(fd, &dev_stat) < 0) {
    goto open_device_cleanup;
  }

  if (!S_ISBLK(dev_stat.st_mode)) {
    /* provided file is not a block device */
    error = FE_NONBLOCK_DEV;
    goto open_device_cleanup;
  }

  enum fat32_error_t op_status;
  op_status = fat32_bpb_read(fd, fs->bpb);
  if (op_status != FE_OK) {
    error = op_status;
    goto open_device_cleanup;
  }

  op_status = fat32_fs_info_read(fd, bpb, fs->fs_info);
  if (op_status != FE_OK) {
    error = op_status;
    goto open_device_cleanup;
  }

  op_status = fat32_fat_init(fs->fat, fs);
  if (op_status != FE_OK) {
    error = op_status;
    goto open_device_cleanup;
  }

  fs->fh_table =
    hash_table_create(params->fh_table_size,
                      fat32_fh_hash, fat32_fh_equal,
                      fat32_fh_cloner, NULL,
                      free, (deallocator_t) fat32_fs_object_free);
  if (fs->fh_table == NULL) {
    goto open_device_cleanup;
  }

  fs->fh_allocator = fat32_fh_allocator_create();
  if (fs->fh_allocator == NULL) {
    goto open_device_cleanup;
  }

  fs->cluster_size = fat32_bpb_cluster_size(bpb);

  return FE_OK;

 open_device_cleanup:
  /* @todo make something more sensible with return value */
  assert(fat32_fs_cleanup(fs) == 0);

 open_fs_error:
  return error;
}

enum fat32_error_t
fat32_fs_close(struct fat32_fs_t *fs)
{
  if (fat32_fs_cleanup(fs) == 0) {
    return FE_OK;
  } else {
    return FE_ERRNO;
  }
}

enum fat32_error_t
fat32_fs_read_cluster(const struct fat32_fs_t *fs, void *buffer,
                      uint32_t cluster)
{
  if (!fat32_bpb_is_valid_cluster(fs->bpb, cluster)) {
    return FE_INVALID_CLUSTER;
  }

  off_t    offset = fat32_cluster_to_offset(fs->bpb, cluster);
  uint32_t cluster_size = fs->cluster_size;

  if (lseek(fs->fd, offset, SEEK_CUR) == (off_t) -1) {
    return FE_ERRNO;
  }

  /* TODO: synchronization */
  int nread = xread(fs->fd, buffer, cluster_size);
  if (nread == -1) {
    return FE_ERRNO;
  } else if (nread < cluster_size) {
    return FE_INVALID_DEV;
  }

  return FE_OK;
}

enum fat32_error_t
fat32_fs_get_object(const struct fat32_fs_t *fs,
                    const char *path,
                    struct fat32_fs_object_t **fs_object)
{
  char *path_copy = NULL;
  char *saveptr;
  char *token;
  char *str;
  enum fat32_error_t return_code = FE_ERRNO;
  struct fat32_fs_object_t *parent  = NULL;
  struct fat32_diriter_t   *diriter = NULL;

  path_copy = strdup(path);
  if (path_copy == NULL) {
    goto cleanup;
  }

  str = path_copy;
  token = strtok_r(str, "/", &saveptr);

  parent = fat32_fs_object_root_dir(fs);
  if (parent == NULL) {
    goto cleanup;
  }

  while (token != NULL) {
    diriter = fat32_diriter_create(parent);

    if (diriter == NULL) {
      goto cleanup;
    }

    int ret;
    struct fat32_fs_object_t *child;

    while (true) {
      ret = fat32_diriter_next(diriter, &child);
      if (ret == FE_OK) {

        if (child == NULL) {
          /* wrong path */
          *fs_object = NULL;

          return_code = FE_OK;
          goto cleanup;
        }

        if (strcmp(token, child->name) == 0) {
          break;
        }

        fat32_fs_object_free(child);

      } else {
        return_code = ret;
        goto cleanup;
      }
    }

    fat32_diriter_free(diriter);
    diriter = NULL;

    fat32_fs_object_free(parent);

    parent = child;

    token = strtok_r(NULL, "/", &saveptr);
  }

  *fs_object = parent;

  // making cleanup code not to free returned object
  parent      = NULL;
  return_code = FE_OK;

cleanup:
  if (path_copy != NULL) {
    free(path_copy);
  }

  if (diriter != NULL) {
    fat32_diriter_free(diriter);
  }

  if (parent != NULL) {
    fat32_fs_object_free(parent);
  }

  return return_code;
}
