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
 * Frees all allocated recources for the device.
 * 
 * @param device a device to free
 * 
 * @return 0 on success, -1 otherwise
 */
static inline int
cleanup_device(struct fat32_fs_t* device)
{
  if (device != NULL) {
    /* trying to close file descriptor first in order to give
       possibility to retry on error to the user
    */
    if (device->fd != -1) {
      int ret = eintr_safe_close(device->fd);
      if (ret != 0) {
        return ret;
      }
    }

    if (device->write_lock != NULL) {
      /* by invariant we assume that lock has been intitialized */
      int ret = pthread_mutex_destroy(device->write_lock);
      if (ret != 0) {
        return ret;
      }

      free(device->write_lock);
    }

    if (device->bpb != NULL) {
      free(device->bpb);
    }

    free(device);
  }

  return 0;
}

enum fat32_error_t
fat32_open_device(const char *path, params_t params,
                  struct fat32_fs_t **device)
{
  struct fat32_fs_t  *fs_device;
  struct fat32_bpb_t *bpb;
  struct stat         dev_stat;
  pthread_mutex_t    *lock;
  enum fat32_error_t  error = FE_ERRNO;

  int fd;

  fs_device = (struct fat32_fs_t *) malloc(sizeof(struct fat32_fs_t));
  if (fs_device == NULL) {
    goto open_device_error;
  }

  *device = fs_device;

  fs_device->fd         = -1;
  fs_device->bpb        = NULL;
  fs_device->write_lock = NULL;

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
  fs_device->bpb = bpb;

  fd = open(path, O_RDWR);
  if (fd < 0) {
    goto open_device_cleanup;
  }
  fs_device->fd = fd;

  if (fstat(fd, &dev_stat) < 0) {
    goto open_device_cleanup;
  }

  if (!S_ISBLK(dev_stat.st_mode)) {
    /* provided file is not a block device */
    error = FE_NONBLOCK_DEV;
    goto open_device_cleanup;
  }

  /* @todo endianness */
  ssize_t nread = xread(fd, fs_device->bpb, sizeof(struct fat32_bpb_t));
  if (nread >= 0) {
    if (nread < sizeof(struct fat32_bpb_t)) {
      error = FE_INVALID_DEV;
      goto open_device_cleanup;
    }
  } else {
    goto open_device_cleanup;
  }

  /* checking BPB consistency */
  if (!bpb_check_validity(bpb)) {
    /* inconsistent */
    return FE_INVALID_DEV;
  }

  return FE_OK;  

 open_device_cleanup:
  /* @todo make something more sensible with return value */
  assert(cleanup_device(fs_device) == 0);

 open_device_error:
  return error;
}

int
fat32_close_device(struct fat32_fs_t *device)
{
  return cleanup_device(device);
}
