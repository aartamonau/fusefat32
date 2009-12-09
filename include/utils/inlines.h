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
/* IMPORTANT: this header MUST not have inclusion guard */

/// macro which must be used instead @em inline keyword
#undef INLINE
#ifndef EXTERN_INLINE_DEFINITIONS
  #define INLINE inline
#else
  #ifndef REIMPORT_INLINES
    #define INLINE extern
  #else
    #define INLINE inline
  #endif
#endif
