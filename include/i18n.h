/**
 * @file   i18n.h
 * @author Aliaksiej Artamonaŭ <aliaksiej.artamonau@gmail.com>
 * @date   Tue Sep  8 21:44:35 2009
 *
 * @brief  Internatinalization related functions.
 *
 * At the time this file just includes @em libintl.h header and defines
 * underscore shortcut for @em gettext function.
 *
 */
#ifndef _I18N_H_
#define _I18N_H_

#include <libintl.h>

/// @em gettext function shortcut
#define _(...) gettext(__VA_ARGS__)

#endif
