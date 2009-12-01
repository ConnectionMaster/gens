/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_priv.c: Private functions.                                         *
 *                                                                         *
 * Copyright (c) 2009 by David Korth.                                      *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "w32u_priv.h"

// C includes.
#include <stdlib.h>
#include <wchar.h>

// Win32 includes.
#include <winnls.h>


/**
 * w32u_mbstowcs(): Convert a multibyte (UTF-8) string to a wide-character (UTF-16) string.
 * @param mbs Multibyte string.
 * @return Wide-character string. (MUST BE free()'d AFTER USE!)
 */
wchar_t* WINAPI w32u_mbstowcs(const char *mbs)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, mbs, -1, NULL, 0);
	if (len <= 0)
		return NULL;
	
	len *= sizeof(wchar_t);
	wchar_t *wcs = (wchar_t*)malloc(len);
	
	MultiByteToWideChar(CP_UTF8, 0, mbs, -1, wcs, len);
	return wcs;
}
