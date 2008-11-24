/***************************************************************************
 * Gens: [MDP] Interpolated renderer.                                      *
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "mdp_render_interpolated.hpp"
#include <string.h>
#include <stdint.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// TODO: Remove this when x86 asm versions are added.
#undef GENS_X86_ASM

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_interpolated_x86.h"
#endif /* GENS_X86_ASM */

// Mask constants
#define MASK_DIV2_15 ((uint16_t)(0x3DEF))
#define MASK_DIV2_16 ((uint16_t)(0x7BEF))
#define MASK_DIV2_15_ASM ((uint32_t)(0x3DEF3DEF))
#define MASK_DIV2_16_ASM ((uint32_t)(0x7BEF7BEF))
#define MASK_DIV2_32 ((uint32_t)(0x007F7F7F))


#ifndef GENS_X86_ASM
/**
 * T_mdp_render_interpolated_cpp: Blits the image to the screen, interpolated size, interpolation.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Number of bytes per line.
 * @param offset ???
 * @param mask Mask for the interpolation data.
 */
template<typename pixel>
static inline void T_mdp_render_interpolated_cpp(pixel *destScreen, pixel *mdScreen,
						 int width, int height,
						 int pitch, int offset,
						 pixel mask)
{
	pitch /= sizeof(pixel);
	
	for (int y = 0; y < height; y++)
	{
		pixel *SrcLine = &mdScreen[y * (width + offset)];
		pixel *DstLine1 = &destScreen[(y * 2) * pitch];
		pixel *DstLine2 = &destScreen[((y * 2) + 1) * pitch];
		
		for (int i = 0; i < width; i++)
		{
			pixel C = *(SrcLine);
			pixel R = *(SrcLine+1);
			pixel D = *(SrcLine + width + offset);
			pixel DR = *(SrcLine + width + offset + 1);
			
			*DstLine1++ = C;
			*DstLine1++ = ((C >> 1) & mask) + ((R >> 1) & mask);
			*DstLine2++ = ((C >> 1) & mask) + ((D >> 1) & mask);
			*DstLine2++ = (((((C >> 1) & mask) + ((R >> 1) & mask)) >> 1) & mask) +
				      (((((D >> 1) & mask) + ((DR >> 1) & mask)) >> 1) & mask);
			SrcLine++;
		}
	}
}
#endif /* GENS_X86_ASM */


void mdp_render_interpolated_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_interpolated_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset,
				    (renderInfo->bpp == 15));
		}
		else
		{
			mdp_render_interpolated_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset,
				    (renderInfo->bpp == 15 ? MASK_DIV2_15_ASM : MASK_DIV2_16_ASM));
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_interpolated_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset,
			    (renderInfo->bpp == 15 ? MASK_DIV2_15 : MASK_DIV2_16));
#endif /* GENS_X86_ASM */
	}
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & CPUFLAG_MMX)
		{
			mdp_render_interpolated_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_interpolated_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_interpolated_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset,
			    MASK_DIV2_32);
#endif /* GENS_X86_ASM */
	}
}
