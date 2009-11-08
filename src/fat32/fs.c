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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "fat32/fs.h"
#include "utils/files.h"
#include "utils/log.h"
#include <stdio.h>

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

    free(fs);
  }

  return 0;
}

enum fat32_error_t
fat32_fs_open(const char *path, params_t params,
	      struct fat32_fs_t **result)
{
  struct fat32_fs_t      *fs;
  struct fat32_bpb_t     *bpb;
  struct fat32_fs_info_t *fs_info;
  struct stat             dev_stat;
  pthread_mutex_t        *lock;
  enum fat32_error_t      error = FE_ERRNO;

  int fd;

  fs = (struct fat32_fs_t *) malloc(sizeof(struct fat32_fs_t));
  if (fs == NULL) {
    goto open_fs_error;
  }

  *result = fs;

  fs->fd         = -1;
  fs->bpb        = NULL;
  fs->fs_info    = NULL;
  fs->write_lock = NULL;

  lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
  if (lock == NULL) {
    goto open_device_cleanup;
  }

  int ret = pthread_mutex_init(lock, NULL);
  if (pthread_mutex_init(lock, NULL) != 0) {
    /* we are deallocating memory to hold the invariant described
       at @link fat32_fs_t.write_lock @endlink
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

  enum fat32_error_t reading_result;
  reading_result = fat32_bpb_read(fd, fs->bpb);
  if (reading_result != FE_OK) {
    error = reading_result;
    goto open_device_cleanup;
  }

  reading_result = fat32_fs_info_read(fd, bpb, fs->fs_info);
  if (reading_result != FE_OK) {
    error = reading_result;
    goto open_device_cleanup;
  }

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
