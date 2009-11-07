/**
 * @file   fs_info.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 16:56:54 2009
 * 
 * @brief FSInfo related functions.
 * 
 */

#include <unistd.h>

#include "fat32/fs_info.h"
#include "fat32/utils.h"

#include "utils/errors.h"
#include "utils/log.h"
#include "utils/files.h"

/// magic number that must be stored in
/// @link fat32_fs_info_t.lead_signature @endlink
const uint32_t FS_INFO_LEAD_SIGNATURE   = 0x41615252;

/// magic number that must be stored in
/// @link fat32_fs_info_t.struct_signature @endlink
const uint32_t FS_INFO_STRUCT_SIGNATURE = 0x61417272;


/// magic number that must be stored in
/// @link fat32_fs_info_t.trail_signature @endlink
const uint32_t FS_INFO_TRAIL_SIGNATURE  = 0xaa550000;

bool
fs_info_check_validity(const struct fat32_fs_info_t *fs_info)
{
  /* actually only three signatures must be checked */
  return (fs_info->lead_signature   == FS_INFO_LEAD_SIGNATURE)   && \
         (fs_info->struct_signature == FS_INFO_STRUCT_SIGNATURE) && \
         (fs_info->trail_signature  == FS_INFO_TRAIL_SIGNATURE);
}

int
fs_info_verbose_info(const struct fat32_fs_info_t *fs_info)
{
  CHECK_NN( log_debug("FSInfo verbose info: ") );

  CHECK_NN( log_debug("\tLead signature: %#" PRIx32,
		      fs_info->lead_signature) );
  CHECK_NN( log_debug("\tStruct signature: %#" PRIx32,
		      fs_info->struct_signature) );
  CHECK_NN( log_debug("\tTrail signature: %#" PRIx32,
		      fs_info->trail_signature) );
  CHECK_NN( log_debug("\tLast known free cluster: %#" PRIu32,
		      fs_info->last_free_cluster) );
  CHECK_NN( log_debug("\tFree cluster hint: %#" PRIu32,
		      fs_info->free_cluster_hint) );

  return 0;
}

enum fat32_error_t
fs_info_read(int fd, const struct fat32_bpb_t *bpb,
	     struct fat32_fs_info_t *fs_info)
{
  off_t fs_info_offset = sector_to_offset(bpb, bpb->fs_info_sector);
  off_t old_offset     = lseek(fd, fs_info_offset, SEEK_SET);
  if (old_offset == (off_t) -1) {
    return FE_ERRNO;
  }

  /* @todo endianness */
  ssize_t nread = xread(fd, fs_info, sizeof(struct fat32_fs_info_t));
  if (nread >= 0) {
    if (nread < sizeof(struct fat32_fs_info_t)) {
      return FE_INVALID_DEV;
    }
  } else {
    return FE_ERRNO;
  }

  if (!fs_info_check_validity(fs_info)) {
    return FE_INVALID_FS;
  }

  if (lseek(fd, old_offset, SEEK_SET) == (off_t) -1) {
    return FE_ERRNO;
  }

  return FE_OK;
}
