#include <unistd.h>

#include "utils/files.h"

#include "fat32/fs.h"
#include "fat32/fat.h"
#include "fat32/utils.h"

enum fat32_error_t
fat32_fat_init(struct fat32_fat_t *fat,
	       const struct fat32_fs_t *fs)
{
  int fd = dup(fs->fd);
  if (fd < 0) {
    return FE_ERRNO;
  }

  fat->fd      = fd;
  fat->bpb     = fs->bpb;
  fat->fs_info = fs->fs_info;

  fat->bytes_per_sector_log =
    fat32_highest_bit_number(fs->bpb->bytes_per_sector);

  return FE_OK;
}

enum fat32_error_t
fat32_fat_finalize(struct fat32_fat_t *fat)
{
  if (xclose(fat->fd) < 0) {
    return FE_ERRNO;
  }

  return FE_OK;
}

static const uint8_t  FAT32_FAT_ENTRY_SIZE = sizeof(uint32_t);
static const uint32_t FAT32_FAT_ENTRY_MASK = 0x0fffffff;

enum fat32_error_t
fat32_fat_next_cluster(const struct fat32_fat_t *fat,
		       uint32_t *cluster)
{
  /* an offset of the entry in a FAT corresponding to @em cluster */
  uint32_t entry_fat_offset    = *cluster * FAT32_FAT_ENTRY_SIZE;
  uint32_t entry_sector        = fat->bpb->reserved_sectors_count + \
    (entry_fat_offset / fat->bpb->bytes_per_sector);
  uint32_t entry_sector_offset = entry_fat_offset % fat->bpb->bytes_per_sector;

  uint32_t entry;
  off_t file_offset = fat32_sector_offset_to_offset(fat->bpb,
						    entry_sector,
						    entry_sector_offset);

  off_t ret = lseek(fat->fd, file_offset, SEEK_SET);
  if (ret < (off_t) 0) {
    return FE_ERRNO;
  }
		    
  ssize_t nread = xread(fat->fd, &entry, FAT32_FAT_ENTRY_SIZE);
  if (nread >= 0) {
    if (nread < FAT32_FAT_ENTRY_SIZE) {
      return FE_INVALID_DEV;
    }
  } else {
    return FE_ERRNO;
  }

  *cluster = entry & FAT32_FAT_ENTRY_MASK;

  return FE_OK;
}

bool
fat32_fat_cluster_is_null(uint32_t cluster)
{
  return cluster == FAT32_FAT_ENTRY_MASK;
}

bool
fat32_fat_cluster_is_free(uint32_t cluster)
{
  return cluster == 0;
}
