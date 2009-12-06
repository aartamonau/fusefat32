/**
 * @file   operations.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Dec  6 04:49:04 2009
 *
 * @brief  Implementation of FUSE operations.
 *
 *
 */

#include <assert.h>
#include <errno.h>

#include "operations.h"
#include "context.h"

#include "utils/log.h"

#include "fat32/fs.h"
#include "fat32/fs_object.h"
#include "fat32/diriter.h"

/**
 * Fills @em stbuf structure with fs object attributes.
 *
 * @param      fs_object fs object
 * @param[out] stbuf buffer for attr
 */
static void
fs_object_attrs(const struct fat32_fs_object_t *fs_object,
                struct stat *stbuf)
{
  /* TODO: command-line options */
  if (fat32_fs_object_is_directory(fs_object)) {
    stbuf->st_mode  = S_IFDIR | 0755;
    stbuf->st_nlink = 1;
  } else {
    stbuf->st_mode  = S_IFREG | 0444;
    stbuf->st_nlink = 1;
    stbuf->st_size  = 0;
  }
}

/**
 * Implementation of getattr call.
 *
 * @param      path  a path to file
 * @param[out] stbuf buffer for attributes
 *
 * @return     operation result
 */
int
fat32_getattr(const char *path, struct stat *stbuf)
{
  /* TODO: optimize */

  struct fuse_context        *context    = fuse_get_context();
  struct fusefat32_context_t *ff_context =
    (struct fusefat32_context_t *) context->private_data;
  struct fat32_fs_object_t   *fs_object;


  enum fat32_error_t ret = fat32_fs_get_object(ff_context->fs,
                                               path,
                                               &fs_object);
  if (ret == FE_OK) {
    if (fs_object == NULL) {
      return -ENOENT;
    } else {
      fs_object_attrs(fs_object, stbuf);
      fat32_fs_object_free(fs_object);

      return 0;
    }
  } else {
    switch (ret) {
    case FE_ERRNO:
      return -errno;
    case FE_INVALID_DEV:
      return -EBADF;
    default:
      assert( false );
    }
  }
}

/**
 * Implements readdir call
 *
 * @param path Path to directory.
 * @param buffer
 * @param filler
 * @param offset
 * @param file_info
 *
 * @return operation result.
 */
int
fat32_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
              off_t offset, struct fuse_file_info *file_info)
{
  (void) offset;
  (void) file_info;

  struct fuse_context        *context    = fuse_get_context();
  struct fusefat32_context_t *ff_context =
    (struct fusefat32_context_t *) context->private_data;

  struct fat32_fs_object_t *fs_object = NULL;
  struct fat32_diriter_t   *diriter   = NULL;
  enum   fat32_error_t      ret;

  int    retcode;

  ret = fat32_fs_get_object(ff_context->fs, path, &fs_object);
  switch (ret) {
  case FE_ERRNO:
    return -errno;
  case FE_INVALID_DEV:
    // possibly not the best choice
    return -EBADF;
  case FE_OK:
    /* all processing is done after the switch */
    break;
  default:
    /* impossible happened */
    assert( false );
  }

  if (fs_object == NULL) {
    return -ENOENT;
  }

  if (!fat32_fs_object_is_directory(fs_object)) {
    return -ENOTDIR;
  }

  diriter = fat32_diriter_create(fs_object);
  if (diriter == NULL) {
    retcode = -errno;
    goto cleanup;
  }

  fat32_fs_object_free(fs_object);
  fs_object = NULL;

  while (true) {
    ret = fat32_diriter_next(diriter, &fs_object);

    switch (ret) {
    case FE_ERRNO:
      retcode = -errno;
      goto cleanup;
    case FE_INVALID_DEV:
      retcode = -EBADF;
      goto cleanup;
    case FE_OK:
      break;
    default:
      /* impossible happened */
      assert( false );
    }

    if (fs_object != NULL) {
      struct stat stbuf;

      fs_object_attrs(fs_object, &stbuf);

      int fret = filler(buffer, fs_object->name, NULL, 0);
      if (fret != 0) {
        /* error */
        retcode = -errno;
        goto cleanup;
      }
    } else {
      break;
    }

    fat32_fs_object_free(fs_object);
  }

  fat32_diriter_free(diriter);

  return 0;

cleanup:
  if (fs_object != NULL) {
    fat32_fs_object_free(fs_object);
  }

  if (diriter != NULL) {
    fat32_diriter_free(diriter);
  }

  return retcode;
}

const struct fuse_operations fusefat32_operations = {
  .readdir = fat32_readdir,
  .getattr = fat32_getattr,
};
