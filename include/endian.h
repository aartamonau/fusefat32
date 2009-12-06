/**
 * @file   endian.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Wed Sep  9 01:18:10 2009
 *
 * @brief  Byteorder conversion functions.
 *
 * For now this file just includes global @em endian.h header. This is
 * BSD-specific and it's available only since 2.9 of version of glibc.
 * As all this is factored out into separate header, it can be changed
 * easily later.
 *
 * To make it work _BSD_SOURCE must be activated in some way.
 *
 * @todo make something more than just inclusion of another endian.h
 * header
 */

#ifndef _ENDIAN_H_
#define _ENDIAN_H_

#include <endian.h>

#endif
