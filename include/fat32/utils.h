/**
 * @file   utils.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 18:34:40 2009
 * 
 * @brief  Various fs-related utility functions.
 * 
 * 
 */
#ifndef _UTILS_H_
#define _UTILS_H_

#include <sys/types.h>
#include <inttypes.h>

#include "fat32/bpb.h"
#include "utils/inlines.h"

/** 
 * Transforms a number of sector to file offset.
 * 
 * @param bpb BPB structure
 * @param sector sector number
 * 
 * @return offset in file
 */
INLINE off_t
fat32_sector_to_offset(const struct fat32_bpb_t *bpb, uint32_t sector)
{
  return (off_t) bpb->bytes_per_sector * (off_t) sector;
}

/** 
 * Transforms a number of sector and an offset inside this sector into
 * a global file offset.
 * 
 * @param bpb BPB structure
 * @param sector sector number
 * @param offset offset inside the sector
 * 
 * @return offset in file
 */
INLINE off_t
fat32_sector_offset_to_offset(const struct fat32_bpb_t *bpb,
			      uint32_t sector, uint32_t offset)
{
  return fat32_sector_to_offset(bpb, sector) + (off_t) offset;
}

/** 
 * Returns the number of the highest bit in the number set to 1. Obvious and
 * not optimized version.
 * 
 * @param number a number to analyze
 * 
 * @return a number of the highest 1-bit counted from zero
 */
INLINE uint8_t
fat32_highest_bit_number(uint32_t number)
{
  int result = 0;

  while (number >>= 1) {
    result++;
  }

  return result;
}

#endif /* _UTILS_H_ */
