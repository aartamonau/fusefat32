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

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <fuse_opt.h>

#include "i18n.h"

#include "fat32/fs.h"

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
};

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
  switch (key) {
  case KEY_VERSION:
    fputs(FUSEFAT32_VERSION, stderr);
    fuse_opt_add_arg(outargs, "--version");
    fuse_main(outargs->argc, outargs->argv, NULL, NULL);
    exit(EXIT_SUCCESS);
  case KEY_HELP:
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
  struct fusefat32_config_t config;

  memset(&config, 0, sizeof(struct fusefat32_config_t));
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

  fprintf(stderr, "Bytes per sector: %" PRIu16 "\n",
          fs->bpb->bytes_per_sector);
  fprintf(stderr, "Sectors per cluster: %" PRIu8 "\n",
          fs->bpb->sectors_per_cluster);
  fprintf(stderr, "Reserved sectors: %" PRIu16 "\n",
          fs->bpb->reserved_sectors_count);
  fprintf(stderr, "Number of FATs: %" PRIu8 "\n",
          fs->bpb->fats_count);
  fprintf(stderr, "Number of root entries: %" PRIu16 "\n",
          fs->bpb->root_entries_count);

  fprintf(stderr, "Media type: %#" PRIx8 "\n",
          fs->bpb->media_type);

  fprintf(stderr, "Sectors per track: %" PRIu16 "\n",
          fs->bpb->sectors_per_track);
  fprintf(stderr, "Heads: %" PRIu16 "\n",
          fs->bpb->heads_number);
  fprintf(stderr, "Hidden sectors: %" PRIu32 "\n",
          fs->bpb->hidden_sectors_count);
  fprintf(stderr, "Total sectors: %" PRIu32 "\n",
          fs->bpb->total_sectors_count_32);

  fprintf(stderr, "Fat size: %" PRIu32 "\n",
          fs->bpb->fat_size_32);
  fprintf(stderr, "Root cluster: %" PRIu32 "\n",
          fs->bpb->root_cluster);

  fprintf(stderr, "Boot signature: %#" PRIx8 "\n",
          fs->bpb->boot_signature);


  assert(fat32_close_device(fs) == 0);

  return EXIT_SUCCESS;
}
