/**
 * @file   bpb.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 23:37:07 2009
 * 
 * @brief  Functions for working with BPB.
 *
 * @todo Apparently #fat32_bpb_t structure must not be a mirror of the data
 *       that is stored on the disk. On the contrary it must contain as its
 *       substructure the data stored on the disk and some of the fields for
 *       caching or something of the sort.
 * 
 */

#include <stdio.h>
#include <sys/types.h>

#include "fat32/bpb.h"
#include "utils/files.h"
#include "utils/errors.h"
#include "utils/log.h"

/// maximum possible size of a single cluster in bytes
static const uint16_t MAX_CLUSTER_SIZE   = 32 * 1024;

/// value of #fat32_bpb_t::fs_version field which specifies
/// that given filesystem is FAT32
static const uint16_t FAT32_FS_VERSION   = 0x0000;

/// minimum valid cluster number
static const uint32_t MIN_CLUSTER_NUMBER = 2;

/// minimum number of clusters that valid FAT32 file system can contain
static const uint32_t FAT32_MIN_CLUSTERS = 65525;

int
fat32_bpb_verbose_info(const struct fat32_bpb_t *bpb)
{
  CHECK_NN( log_debug("BPB verbose info: ") );

  CHECK_NN( log_debug("\tBytes per sector: %" PRIu16,
		      bpb->bytes_per_sector) );
  CHECK_NN( log_debug("\tSectors per cluster: %" PRIu8,
		      bpb->sectors_per_cluster) );
  CHECK_NN( log_debug("\tReserved sectors: %" PRIu16,
		      bpb->reserved_sectors_count) );
  CHECK_NN( log_debug("\tNumber of FATs: %" PRIu8,
		      bpb->fats_count) );
  CHECK_NN( log_debug("\tNumber of root entries: %" PRIu16,
		      bpb->root_entries_count) );

  CHECK_NN( log_debug("\tMedia type: %#" PRIx8,
		      bpb->media_type) );

  CHECK_NN( log_debug("\tSectors per track: %" PRIu16,
		      bpb->sectors_per_track) );
  CHECK_NN( log_debug("\tHeads: %" PRIu16,
		      bpb->heads_number) );
  CHECK_NN( log_debug("\tHidden sectors: %" PRIu32,
		      bpb->hidden_sectors_count) );
  CHECK_NN( log_debug("\tTotal sectors: %" PRIu32,
		      bpb->total_sectors_count) );

  CHECK_NN( log_debug("\tFat size: %" PRIu32,
		      bpb->fat_size) );
  CHECK_NN( log_debug("\tRoot cluster: %" PRIu32,
		      bpb->root_cluster) );
  CHECK_NN( log_debug("\tFSInfo sector: %" PRIu32,
		      bpb->fs_info_sector) );

  CHECK_NN( log_debug("\tBoot signature: %#" PRIx8,
		      bpb->boot_signature) );

  return 0;
}

bool
fat32_bpb_check_validity(const struct fat32_bpb_t *bpb)
{
  /* Checking jmp_boot. Two forms are allowed:
       - jmp_boot[0] == 0xEB && jmp_boot[1] == 0x?? && jmp_boot[2] == 0x90
       - jmp_boot[0] == 0xE9 && jmp_boot[1] == 0x?? && jmp_boot[2] == 0x??
  */

  const uint8_t *jmp_boot = bpb->jmp_boot;
  if ((jmp_boot[0] != 0xEB || jmp_boot[2] != 0x90) && (jmp_boot[0] != 0xE9)) {
    return false;
  }

  /* Number of bytes per sector can be only 512, 1024, 2048 or 4096.
     0x1e00 = 0001111000000000 (binary representation) indicates the places
     which can be set for each of mentioned numbers.
  */
  uint16_t bps  = bpb->bytes_per_sector;
  uint16_t mask = ~ 0x1e00;

  if (((bps & (bps - 1)) != 0) || ((bps & mask) != 0)) {
    return false;
  }

  /* Number of sectors per cluster can be any power of two. But the result
     of multiplication of @em bytes_per_sector and @em sectors_per_cluster
     can't exceed #MAX_CLUSTER_SIZE (which is 32K bytes) value.
  */
  uint8_t  spc     = bpb->sectors_per_cluster;
  uint32_t product = spc * bps;

  if ((spc == 0) || ((spc & (spc - 1)) != 0) || (product > MAX_CLUSTER_SIZE)) {
    return false;
  }

  /* Number of reserved sectors can't be zero */
  if (bpb->reserved_sectors_count == 0) {
    return false;
  }

  /* Root entries count must be zero on FAT32 */
  if (bpb->root_entries_count != 0) {
    return false;
  }

  /* 16-bit total number of sectors must be zero on FAT32 */
  if (bpb->total_sectors_count_16 != 0) {
    return false;
  }

  /* media type can be 0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFE, 0xFD, 0xFF */
  uint8_t media_type = bpb->media_type;
  if ((media_type != 0xF0) && (media_type < 0xF8)) {
    return false;
  }

  /* 16-bit FAT's size must be zero on FAT32 */
  if (bpb->fat_size_16 != 0) {
    return false;
  }

  /* total number of sectors can't be zero
     @todo more elaborate check
  */
  if (bpb->total_sectors_count == 0) {
    return false;
  }

  /* FAT's size can't be zero
     @todo elaborate check
  */
  if (bpb->fat_size == 0) {
    return false;
  }

  /* fs version must be 0x0000 on FAT32 */
  if (bpb->fs_version != FAT32_FS_VERSION) {
    return false;
  }

  /* clusters count must be at leas #FAT32_MIN_CLUSTERS */
  uint32_t clusters_count = fat32_bpb_clusters_count(bpb);
  if (clusters_count < FAT32_MIN_CLUSTERS) {
    return false;
  }

  /* root cluster number can be any valid cluster number */
  if (!fat32_bpb_is_valid_cluster(bpb, bpb->root_cluster)) {
    return false;
  }

  /* fs_info sector can be any sector in reserved area */
  uint16_t fs_info_sector = bpb->fs_info_sector;
  if ((fs_info_sector < 1) || (fs_info_sector > bpb->reserved_sectors_count)) {
    return false;
  }

  /* @todo check backup_boot_sector intersection with fs_info */

  return true;
}

enum fat32_error_t
fat32_bpb_read(int fd, struct fat32_bpb_t *bpb)
{
  /* @todo endianess */
  ssize_t nread = xread(fd, bpb, sizeof(struct fat32_bpb_t));
  if (nread >= 0) {
    if (nread < sizeof(struct fat32_bpb_t)) {
      return FE_INVALID_DEV;
    }
  } else {
    return FE_ERRNO;
  }

  if (!fat32_bpb_check_validity(bpb)) {
    return FE_INVALID_FS;
  }

  return FE_OK;
}

uint32_t
fat32_bpb_clusters_count(const struct fat32_bpb_t *bpb)
{
  uint32_t total    = bpb->total_sectors_count;
  uint32_t reserved = bpb->reserved_sectors_count;
  uint32_t fat      = bpb->fats_count * bpb->fat_size;
  uint32_t data_sectors = total - reserved - fat;

  return data_sectors / bpb->sectors_per_cluster;
}

bool
fat32_bpb_is_valid_cluster(const struct fat32_bpb_t *bpb,
			   uint32_t cluster)
{
  uint32_t clusters_count = fat32_bpb_clusters_count(bpb);

  return (cluster >= MIN_CLUSTER_NUMBER &&
	  cluster <= clusters_count + 1);
}
