/**
 * @file   context.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Dec  6 05:04:29 2009
 *
 * @brief  Defines a context structures of file system.
 *
 *
 */

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <stdbool.h>

/**
 * A structure to store mounting options
 *
 */
struct fusefat32_config_t {
  char *parent_dir;             /**< a parent directory to mount point */
  char *device;                 /**< a path to device to mount */
  char *log;                    /**< a path to log file */
  bool  verbose;                /**< behave verbosely */
  bool  foreground;             /**< run program in foreground and
                                   do all logging to @em stderr  */
};

/// default fusefat32 config
#define FUSEFAT32_CONFIG_DEFAULT { .parent_dir  = NULL, \
                                   .device      = NULL, \
                                   .log         = NULL, \
                                   .foreground  = false,\
                                   .verbose     = false }

/**
 * Generates FUSE input option descriptor
 *
 * @param t option template
 * @param p member of #fusefat32_config_t where a value for the option
 *          must be stored
 *
 * @return option definition
 */
#define FUSEFAT32_OPT(t, p) { t, offsetof(struct fusefat32_config_t, p), 0}

/**
 * All data needed for program gathered in one place.
 *
 */
struct fusefat32_context_t {
  struct fusefat32_config_t config; /**< config */
  struct fat32_fs_t *fs;            /**< filesytstem */
};

#endif /* _CONTEXT_H_ */
