/**
 * @file   error_messages.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sun Dec 13 12:35:09 2009
 *
 * @brief  Contains error messages that can be issued to the user by
 *         FUSE operations implementations from operations.c file.
 *
 *
 */
#ifndef _ERROR_MESSAGES_H_
#define _ERROR_MESSAGES_H_

#include "i18n.h"

/// An error message logged in case of file system becomes
/// partially inconsistent.
#define FUSEFAT32_PARTIALLY_INCONSISTENT_FS_MSG \
  _("Unable to free cluster chain. " \
    "Some of the clusters won't be used before you run fsck.")

#define FUSEFAT32_INCONSISTENT_FS_MSG \
  _("Due to IO errors a file system could have been left in " \
    "inconsistent state. Please run fsck to verify it")

/// An error message logged in case of provided device is invalid
#define FUSEFAT32_INVALID_DEVICE_MSG \
  _("Specified device is invalid. You should check it back.")

/// An error message logged in case of file system is invalid.
#define FUSEFAT32_INVALID_FS_MSG \
  _("A file system on the device is broken. Check it with fsck.")

#endif /* _ERROR_MESSAGES_H_ */
