/***************************************************************************
 * Gens: [MDP] 1x renderer.                                                *
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

#include "mdp_render_1x.hpp"
#include <string.h>
#include <stdint.h>

// CPU flags
#include "plugins/mdp_cpuflags.h"

// x86 asm versions
#ifdef GENS_X86_ASM
#include "mdp_render_1x_x86.h"
#endif /* GENS_X86_ASM */


#ifndef GENS_X86_ASM
/**
 * T_mdp_render_1x_cpp: Blits the image to the screen, 1x size, no filtering.
 * @param destScreen Pointer to the destination screen buffer.
 * @param mdScreen Pointer to the MD screen buffer.
 * @param width Width of the image.
 * @param height Height of the image.
 * @param pitch Number of bytes per line.
 * @param offset ???
 */
template<typename pixel>
static inline void T_mdp_render_1x_cpp(pixel *destScreen, pixel *mdScreen,
				       int width, int height,
				       int pitch, int offset)
{
	for (int i = 0; i < height; i++)
	{
		memcpy(destScreen, mdScreen, width * sizeof(pixel));
		
		// Next line.
		mdScreen += width + offset;
		destScreen += (pitch / sizeof(pixel));
	}
}
#endif /* GENS_X86_ASM */


void mdp_render_1x_cpp(MDP_Render_Info_t *renderInfo)
{
	if (!renderInfo)
		return;
	
	if (renderInfo->bpp == 16 || renderInfo->bpp == 15)
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_1x_16_x86_mmx(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_1x_16_x86(
				    (uint16_t*)renderInfo->destScreen,
				    (uint16_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_1x_cpp(
			    (uint16_t*)renderInfo->destScreen,
			    (uint16_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
	else
	{
#ifdef GENS_X86_ASM
		if (renderInfo->cpuFlags & MDP_CPUFLAG_MMX)
		{
			mdp_render_1x_32_x86_mmx(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
		else
		{
			mdp_render_1x_32_x86(
				    (uint32_t*)renderInfo->destScreen,
				    (uint32_t*)renderInfo->mdScreen,
				    renderInfo->width, renderInfo->height,
				    renderInfo->pitch, renderInfo->offset);
		}
#else /* !GENS_X86_ASM */
		T_mdp_render_1x_cpp(
			    (uint32_t*)renderInfo->destScreen,
			    (uint32_t*)renderInfo->mdScreen,
			    renderInfo->width, renderInfo->height,
			    renderInfo->pitch, renderInfo->offset);
#endif /* GENS_X86_ASM */
	}
}
