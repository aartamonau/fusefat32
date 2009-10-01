/**
 * @file   bpb.c
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Thu Oct  1 23:37:07 2009
 * 
 * @brief  Functions for working with BPB.
 * 
 * 
 */

#include "fat32/bpb.h"
#include "utils/errors.h"

int
bpb_verbose_info(FILE *file, struct fat32_bpb_t *bpb)
{
  CHECK_NN( fprintf(stderr, "Bytes per sector: %" PRIu16 "\n",
                  bpb->bytes_per_sector) );
  CHECK_NN( fprintf(stderr, "Sectors per cluster: %" PRIu8 "\n",
                  bpb->sectors_per_cluster) );
  CHECK_NN( fprintf(stderr, "Reserved sectors: %" PRIu16 "\n",
                  bpb->reserved_sectors_count) );
  CHECK_NN( fprintf(stderr, "Number of FATs: %" PRIu8 "\n",
                  bpb->fats_count) );
  CHECK_NN( fprintf(stderr, "Number of root entries: %" PRIu16 "\n",
                  bpb->root_entries_count) );

  CHECK_NN( fprintf(stderr, "Media type: %#" PRIx8 "\n",
                  bpb->media_type) );

  CHECK_NN( fprintf(stderr, "Sectors per track: %" PRIu16 "\n",
                  bpb->sectors_per_track) );
  CHECK_NN( fprintf(stderr, "Heads: %" PRIu16 "\n",
                  bpb->heads_number) );
  CHECK_NN( fprintf(stderr, "Hidden sectors: %" PRIu32 "\n",
                  bpb->hidden_sectors_count) );
  CHECK_NN( fprintf(stderr, "Total sectors: %" PRIu32 "\n",
                  bpb->total_sectors_count_32) );

  CHECK_NN( fprintf(stderr, "Fat size: %" PRIu32 "\n",
                  bpb->fat_size_32) );
  CHECK_NN( fprintf(stderr, "Root cluster: %" PRIu32 "\n",
                  bpb->root_cluster) );

  CHECK_NN( fprintf(stderr, "Boot signature: %#" PRIx8 "\n",
                  bpb->boot_signature) );

  return 0;
}
