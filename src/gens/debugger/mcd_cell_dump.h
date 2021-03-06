/***************************************************************************
 * Gens: Debugger - SegaCD Cell Dump Functions.                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
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

#ifndef GENS_MCD_CELL_DUMP_H
#define GENS_MCD_CELL_DUMP_H

#ifdef __cplusplus
extern "C" {
#endif

void Cell_8x8_Dump(unsigned char *Adr, int Palette);	// MD VDP, normal.
void Cell_8x16_Dump(unsigned char *Adr, int Palette);	// MD VDP, interlaced.
void Cell_16x16_Dump(unsigned char *Adr, int Palette);	// SegaCD Work RAM.
void Cell_32x32_Dump(unsigned char *Adr, int Palette);	// SegaCD Work RAM.

#ifdef __cplusplus
}
#endif

#endif /* GENS_MCD_CELL_DUMP_H */
