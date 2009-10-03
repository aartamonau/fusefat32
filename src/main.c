/**
 * @file   main.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Sep  8 20:30:41 2009
 * 
 * @brief Integrates all filesystem functionality.
 *
 * This file is the main file of filesystem. It's responsibility is
 * to read needed options and unite all other functionality in a single
 * block.
 * 
 */

/* workaround allowing to build the filesystem using -std=c99 */
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fuse_opt.h>

#include "i18n.h"

#include "fat32/fs.h"
#include "utils/errors.h"

/// information about fusefat32 version
#define FUSEFAT32_VERSION "fusefat32 1.4\n"

/// fusefat32 usage information
#define FUSEFAT32_USAGE _("usage: %s mountpoint [options]\n"     \
                          "\n"                                   \
                          "general options:\n"                   \
                          "    -o opt,[opt...]  mount options\n" \
                          "    -h   --help      print help\n"    \
                          "    -V   --version   print version\n" \
                          "\n"                                   \
                          "fusefat32 options:\n"                 \
                          "    -o dev=STRING    a path to device to mount\n")


/**
 * A structure to store mounting options
 * 
 */
struct fusefat32_config_t {
  char *device;                 /**< a path to device to mount */
  bool  verbose;                /**< behave verbosely */
};

/// default fusefat32 config
#define FUSEFAT32_CONFIG_DEFAULT { .device  = NULL, \
                                   .verbose = false }

/**
 * Generates FUSE input option descriptor
 *
 * @param t option template
 * @param p member of @link fusefat32_config_t @endlink where a value for the option
 *          must be stored
 *
 * @return option definition
 */
#define FUSEFAT32_OPT(t, p) { t, offsetof(struct fusefat32_config_t, p), 0}

/**
 * Key parameters of fusefat32
 * 
 */
enum {
  KEY_VERSION,                  /**< indicates that user has acquired version
                                   information */
  KEY_HELP,                     /**< indicates that user has acquired program
                                   usage information */
  KEY_VERBOSE                   /**< print verbose information while mounting */
};


/**
 * Additional options which fusefat32 accepts
 * 
 */
static struct fuse_opt fusefat32_options[] = {
  FUSEFAT32_OPT("dev=%s", device),

  FUSE_OPT_KEY("--version", KEY_VERSION),
  FUSE_OPT_KEY("-V",        KEY_VERSION),
  FUSE_OPT_KEY("--help",    KEY_HELP),
  FUSE_OPT_KEY("-h",        KEY_HELP),
  FUSE_OPT_KEY("--verbose", KEY_VERBOSE),
  FUSE_OPT_KEY("-v",        KEY_VERBOSE),
  FUSE_OPT_END
};

/**
 * Processes version and help keys.
 * 
 */
static int
fusefat32_process_options(void *data, const char *arg, int key,
                          struct fuse_args *outargs)
{
  struct fusefat32_config_t *config = (struct fusefat32_config_t *) data;

  switch (key) {
  case KEY_VERSION:
    fputs(FUSEFAT32_VERSION, stderr);
    fuse_opt_add_arg(outargs, "--version");
    fuse_main(outargs->argc, outargs->argv, NULL, NULL);
    exit(EXIT_SUCCESS);
  case KEY_HELP:
    fprintf(stderr, FUSEFAT32_USAGE, outargs->argv[0]);
    exit(EXIT_SUCCESS);
  case KEY_VERBOSE:
    config->verbose = true;
    break;
  }

  /* keep option */
  return 1;
}

/** 
 * Main function
 * 
 * @param argc 
 * @param argv 
 * 
 * @return 
 */
int
main(int argc, char *argv[])
{
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  struct fusefat32_config_t config = FUSEFAT32_CONFIG_DEFAULT;
  
  fuse_opt_parse(&args, &config, fusefat32_options, fusefat32_process_options);

  /* if we are here then it means that neither version nor help key has been
     activated
  */
  if (config.device == NULL) {
    fputs(_("A device with filesystem must be specified (use `dev` option)\n"),
          stderr);

    return EXIT_SUCCESS;
  }

  enum fat32_error_t ret;
  struct fat32_fs_t *fs;

  ret = fat32_open_device(config.device, 0, &fs);

  if (ret == FE_OK) {
    fputs(_("OK\n"), stderr);

    
  } else {
    fputs(_("ERROR\n"), stderr);

    if (ret == FE_ERRNO) {
      fprintf(stderr, "%s\n", strerror(errno));
    } else {
      fprintf(stderr, "errorcode: %d\n", ret);
    }

    return EXIT_FAILURE;
  }

  if (config.verbose) {
    CHECK_NN_RET( bpb_verbose_info(stderr, fs->bpb),
                  EXIT_FAILURE );
  }

  assert(fat32_close_device(fs) == 0);

  return EXIT_SUCCESS;
}
