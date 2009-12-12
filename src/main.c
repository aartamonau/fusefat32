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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <libgen.h>

/* /\* using the latest API version *\/ */
/* #define FUSE_USE_VERSION 26 */

#include <fuse.h>
#include <fuse_opt.h>

#include "i18n.h"

#include "context.h"
#include "operations.h"

#include "fat32/fs.h"
#include "utils/errors.h"
#include "utils/log.h"

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
 * Key parameters of fusefat32
 *
 */
enum {
  KEY_VERSION,                  /**< indicates that user has acquired version
                                   information */
  KEY_HELP,                     /**< indicates that user has acquired program
                                   usage information */
  KEY_VERBOSE,                  /**< print verbose information while mounting */
  KEY_FOREGROUND                /**< run program in foreground and log all
                                   messages to @em stderr */
};


/**
 * Additional options which fusefat32 accepts
 *
 */
static struct fuse_opt fusefat32_options[] = {
  FUSEFAT32_OPT("dev=%s", device),
  FUSEFAT32_OPT("log=%s", log),

  FUSE_OPT_KEY("--version",    KEY_VERSION),
  FUSE_OPT_KEY("-V",           KEY_VERSION),
  FUSE_OPT_KEY("--help",       KEY_HELP),
  FUSE_OPT_KEY("-h",           KEY_HELP),
  FUSE_OPT_KEY("--verbose",    KEY_VERBOSE),
  FUSE_OPT_KEY("-v",           KEY_VERBOSE),
  FUSE_OPT_KEY("-f",           KEY_FOREGROUND),
  FUSE_OPT_KEY("--foreground", KEY_FOREGROUND),
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
  struct fusefat32_context_t *fusefat32 =
    (struct fusefat32_context_t *) data;
  struct fusefat32_config_t  *config    = &fusefat32->config;

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

    /* discard option */
    return 0;
  case KEY_FOREGROUND:
    config->foreground = true;

    /* discard option */
    return 0;
  case FUSE_OPT_KEY_NONOPT:
    if (config->parent_dir == NULL) {
      char *path;

      if ((path = realpath(arg, NULL)) == NULL) {
        fprintf(stderr, _("fusefat32: Bad mountpoint `%s`: %s\n"),
                arg, strerror(errno));
        return -1;
      } else {
        config->parent_dir = strdup(dirname(path));
        if (config->parent_dir == NULL) {
          fputs(_("fusefat32: Memory allocation error\n"), stderr);
          return -1;
        }
        free(path);
        return 1;
      }
    } else {
      fprintf(stderr, _("fusefat32: Invalid options `%s`\n"), arg);
      return -1;
    }
  default:
    /* keep option */
    return 1;
  }
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
  struct fuse_args           args        = FUSE_ARGS_INIT(argc, argv);
  struct fusefat32_context_t fusefat32   = { .config = FUSEFAT32_CONFIG_DEFAULT,
                                             .fs     = NULL };
  bool logging_used                      = false;
  int  return_code                       = EXIT_FAILURE;
  struct fusefat32_config_t *config      = NULL;



  if (fuse_opt_parse(&args, &fusefat32, fusefat32_options,
                     fusefat32_process_options) == -1) {
    goto main_cleanup;
  }

  config = &fusefat32.config;

  /* if we are here then it means that neither version nor help key has been
     activated
  */
  if (config->device == NULL) {
    fputs(_("A device to mount must be specified (use `dev` option)\n"),
      stderr);

    goto main_cleanup;
  }

  /* Initializing logging.
     If @em log option has been specified then we use specified
     file for logging. If @em foreground option has been chosen then
     we use @em stderr for logging. If both options has been chosen
     at the same time then we prefer @em foreground option.
     Default importance level for logs is #LOG_WARNING.
     If @em verbose flag is set then log level to use is
     #LOG_DEBUG.
  */

  enum log_level_t log_level = LOG_WARNING;
  if (config->verbose) {
    log_level = LOG_DEBUG;
  }

  if (config->log != NULL && !config->foreground) {
    int lret = log_init_from_path(config->log, log_level);
    if (lret < 0) {
      // @em strerror function is used only in the main thread
      fprintf(stderr, _("Can't initialize logging facility. Error: %s\n"),
              strerror(errno));

      goto main_cleanup;
    } else {
      logging_used = true;
    }
  } else if (config->foreground) {
    log_init_from_file(stderr, log_level);
    logging_used = true;
  } /* otherwise no logging required */


  log_info(_("Opening file system..."));

  enum fat32_error_t ret;
  struct fat32_fs_params_t params = { .file_table_size = 1024,
                                      .fh_table_size   = 1024, };
  ret = fat32_fs_open(config->device, &params,
                      &fusefat32.fs);

  if (ret == FE_OK) {
    log_info(_("File system has been opened successfully."));
  } else {
    log_error(_("Error occured while opening file system."));

    if (ret == FE_ERRNO) {
      log_error(_("Error description: %s"), strerror(errno));
    } else {
      log_error(_("Can't get error description. Error code is %d"), ret);
    }

    goto main_cleanup;
  }

  if (fat32_bpb_verbose_info(fusefat32.fs->bpb) < 0) {
    goto main_cleanup;
  }

  if (fat32_fs_info_verbose_info(fusefat32.fs->fs_info) < 0) {
    goto main_cleanup;
  }

  log_info(_("Starting main FUSE loop..."));
  int fret = fuse_main(args.argc, args.argv, &fusefat32_operations, &fusefat32);

  if (fret != 0) {
    log_error(_("Unable to start FUSE loop: %s"), strerror(errno));
    goto main_cleanup;
  }

  return_code = EXIT_SUCCESS;

 main_cleanup:
  log_info(_("Freeing acquired resources..."));

  fuse_opt_free_args(&args);

  if (fusefat32.fs != NULL) {
    if (fat32_fs_close(fusefat32.fs) < 0) {
      log_error(_("Can't close filesystem correctly: %s"), strerror(errno));
    }
  }

  if (logging_used) {
    log_close();
  }

  if (config != NULL) {
    if (config->parent_dir != NULL) {
      free(config->parent_dir);
    }
  }

  return return_code;
}
