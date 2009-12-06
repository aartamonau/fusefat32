/**
 * @file   inlines.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Sat Nov  7 23:48:16 2009
 *
 * @brief  Some magic to define inline functions obligating c99
 *         standard without without having one instance defined in
 *         several places. See the following links for details:
 *         http://www.keil.com/support/man/docs/armcc/armcc_ch04s04s05.htm
 *         http://www.greenend.org.uk/rjk/2003/03/inline.html
 *
 *
 */
#ifndef _INLINES_H_
#define _INLINES_H_

/// macro which must be used instead @em inline keyword
#ifndef EXTERN_INLINE_DEFINITIONS
  #define INLINE inline
#else
  #define INLINE extern
#endif

#endif /* _INLINES_H_ */
