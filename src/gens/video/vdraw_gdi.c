/***************************************************************************
 * Gens: Video Drawing - GDI Backend.                                      *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2009 by David Korth                                       *
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

#include "vdraw.h"
#include "vdraw_gdi.h"

#include "emulator/g_main.hpp"

// Gens window.
#include "gens/gens_window.hpp"
#include "gens/gens_window_sync.hpp"
#include "ui/gens_ui.hpp"

// VDP includes.
#include "gens_core/vdp/vdp_rend.h"
#include "gens_core/vdp/vdp_io.h"

// CPU flags.
#include "gens_core/misc/cpuflags.h"

// Text drawing functions.
#include "vdraw_text.hpp"

// Windows API.
#include <windows.h>


// Function prototypes.
static int	vdraw_gdi_init(void);
static int	vdraw_gdi_end(void);

static void	vdraw_gdi_clear_screen(void);

static int	vdraw_gdi_flip(void);
static void	vdraw_gdi_draw_border(void); // Not used in vdraw_backend_t.
static void	vdraw_gdi_stretch_adjust(void);
static void	vdraw_gdi_update_renderer(void);

// Win32-specific functions.
static int	vdraw_gdi_reinit_gens_window(void);
static int	vdraw_gdi_clear_primary_screen(void);
static int	vdraw_gdi_clear_back_screen(void);
static int	vdraw_gdi_restore_primary(void);
static int	vdraw_gdi_set_cooperative_level(void);


// VDraw Backend struct.
const vdraw_backend_t vdraw_backend_gdi =
{
	.name = "GDI",
	.flags = VDRAW_BACKEND_FLAG_STRETCH,
	
	.init = vdraw_gdi_init,
	.end = vdraw_gdi_end,
	
	.init_subsystem = NULL,
	.shutdown = NULL,
	
	.clear_screen = vdraw_gdi_clear_screen,
	.update_vsync = NULL,
	
	.flip = vdraw_gdi_flip,
	.stretch_adjust = vdraw_gdi_stretch_adjust,
	.update_renderer = vdraw_gdi_update_renderer,
	
	// Win32-specific functions.
	.reinit_gens_window	= vdraw_gdi_reinit_gens_window,
	.clear_primary_screen	= vdraw_gdi_clear_primary_screen,
	.clear_back_screen	= vdraw_gdi_clear_back_screen,
	.restore_primary	= vdraw_gdi_restore_primary,
	.set_cooperative_level	= vdraw_gdi_set_cooperative_level
};


static HDC     hdcComp  = NULL;    // DC associado ao bitmap
static HBITMAP hbmpDraw = NULL;    // bitmap usado como buffer para imprimir a imagem
static LPBYTE  pbmpData = NULL;    // buffer
static SIZE    szGDIBuf;


/**
 * vdraw_gdi_init(): Initialize the GDI video subsystem.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_init(void)
{
	// TODO: GDI doesn't support fullscreen.
	
	MDP_Render_t *rendMode = get_mdp_render_t();
	const int scale = rendMode->scale;
	
	// Determine the window size using the scaling factor.
	if (scale <= 0)
		return 0;
	szGDIBuf.cx = 320 * scale;
	szGDIBuf.cy = 240 * scale;
	
	// Create the DC.
	hdcComp = CreateCompatibleDC(GetDC(Gens_hWnd));
	if (!hdcComp)
	{
		// Error creating the DC.
		vdraw_init_fail("vdraw_gdi_init(): Error in CreateCompatibleDC().");
		return -1;
	}
	
	// Create the DIB.
	// TODO: Currently always uses 16-bit color.
	// Maybe make it user-specified.
	BITMAPINFOHEADER bih;
	memset(&bih, 0x00, sizeof(bih));
	bih.biSize	= sizeof(bih);
	bih.biPlanes	= 1;
	bih.biBitCount	= 16;
	bih.biWidth	= szGDIBuf.cx;
	bih.biHeight	= -szGDIBuf.cy;
	hbmpDraw = CreateDIBSection(hdcComp, (BITMAPINFO*)&bih, DIB_RGB_COLORS, (LPVOID*)&pbmpData, NULL, 0);
	if (!hbmpDraw)
	{
		// Error creating the DIB.
		vdraw_init_fail("vdraw_gdi_init(): Error in CreateDIBSection().");
		return -2;
	}
	
	// Select the bitmap object on the device context.
	SelectObject(hdcComp, hbmpDraw);
	
	// Set bpp to 16-bit color.
	if (bppOut != 16)
		vdraw_set_bpp(16, FALSE);
	
	// GDI initialized.
	return 0;
}


/**
 * vdraw_gdi_end(): Close the GDI renderer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_end(void)
{
	if (hdcComp)
	{
		DeleteDC(hdcComp);
		hdcComp = NULL;
	}
	
	if (hbmpDraw)
	{
		DeleteObject(hbmpDraw);
		hbmpDraw = NULL;
		pbmpData = NULL;
	}
	
	return 0;
}


/**
 * vdraw_gdi_clear_screen(): Clear the screen.
 */
static void vdraw_gdi_clear_screen(void)
{
	// Clear both screen buffers.
	vdraw_gdi_clear_primary_screen();
	vdraw_gdi_clear_back_screen();
}


/**
 * vdraw_gdi_clear_primary_screen(): Clear the primary screen.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_clear_primary_screen(void)
{
	if (!pbmpData)
		return -1;
	
	memset(pbmpData, 0, (szGDIBuf.cx << 1) * szGDIBuf.cy);
	return 0;
}


/**
 * vdraw_gdi_clear_back_screen(): Clear the back buffer.
 * @return 0 on success; non-zero on error.
 */
static int vdraw_gdi_clear_back_screen(void)
{
	HDC  hdcDest;
	RECT rectDest;
	
	hdcDest = GetDC(Gens_hWnd);
	InvalidateRect(Gens_hWnd, NULL, FALSE);
	GetClientRect(Gens_hWnd, &rectDest);
	FillRect(hdcDest, &rectDest, (HBRUSH)GetStockObject(BLACK_BRUSH));
	
	return 0;
}