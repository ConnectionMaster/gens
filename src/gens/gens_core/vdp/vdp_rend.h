/***************************************************************************
 * Gens: VDP rendering functions.                                          *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2009 by David Korth                                  *
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

#ifndef GENS_VDP_REND_H
#define GENS_VDP_REND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Color palettes.
typedef union
{
	uint16_t u16[0x1000];
	uint32_t u32[0x1000];
} Palette_t;
extern Palette_t Palette;

// 15/16-bit color
extern uint16_t MD_Screen[336 * 240];
extern uint16_t MD_Palette[256];

// 32-bit color
extern uint32_t MD_Screen32[336 * 240];
extern uint32_t MD_Palette32[256];

// _32X_Rend_Mode is used for the 32X 32-bit color C functions.
// See g_32x_32bit.h
extern unsigned int _32X_Rend_Mode;

extern unsigned int TAB336[336];

typedef struct
{
	int Pos_X;
	int Pos_Y;
	unsigned int Size_X;
	unsigned int Size_Y;
	int Pos_X_Max;
	int Pos_Y_Max;
	unsigned int Num_Tile;	// Inclues palette, priority, and flip bits.
	int dirt;
} Sprite_Struct_t;
extern Sprite_Struct_t Sprite_Struct[256];

extern int Sprite_Visible[256];

extern int Sprite_Over;

void VDP_Render_Line(void);

// Old asm versions.
void Render_Line(void);
void Render_Line_32X(void);

// VDP layer control
extern unsigned int VDP_Layers;

// VDP layer flags
#define VDP_LAYER_SCROLLA_LOW		((uint32_t)(1 << 0))
#define VDP_LAYER_SCROLLA_HIGH		((uint32_t)(1 << 1))
#define VDP_LAYER_SCROLLA_SWAP		((uint32_t)(1 << 2))
#define VDP_LAYER_SCROLLB_LOW		((uint32_t)(1 << 3))
#define VDP_LAYER_SCROLLB_HIGH		((uint32_t)(1 << 4))
#define VDP_LAYER_SCROLLB_SWAP		((uint32_t)(1 << 5))
#define VDP_LAYER_SPRITE_LOW		((uint32_t)(1 << 6))
#define VDP_LAYER_SPRITE_HIGH		((uint32_t)(1 << 7))
#define VDP_LAYER_SPRITE_SWAP		((uint32_t)(1 << 8))
#define VDP_LAYER_SPRITE_ALWAYSONTOP	((uint32_t)(1 << 9))
#define	VDP_LAYER_PALETTE_LOCK		((uint32_t)(1 << 10))

#define VDP_LAYER_DEFAULT	  \
	(VDP_LAYER_SCROLLA_LOW	| \
	 VDP_LAYER_SCROLLA_HIGH	| \
	 VDP_LAYER_SCROLLB_LOW	| \
	 VDP_LAYER_SCROLLB_HIGH	| \
	 VDP_LAYER_SPRITE_LOW	| \
	 VDP_LAYER_SPRITE_HIGH)

// C++ functions in vdp_rend.cpp.
void VDP_Update_Palette(void);
void VDP_Update_Palette_HS(void);

#ifdef __cplusplus
}
#endif

#endif
