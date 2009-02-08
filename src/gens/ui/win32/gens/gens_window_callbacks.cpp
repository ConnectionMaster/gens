/***************************************************************************
 * Gens: (Win32) Main Window - Callback Functions.                         *
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

#include "emulator/g_main.hpp"
#include "gens_window.hpp"
#include "gens_window_callbacks.hpp"
#include "gens_window_sync.hpp"

#include "emulator/options.hpp"
#include "util/file/config_file.hpp"

// Unused Parameter macro.
#include "macros/unused.h"

#include "ui/gens_ui.hpp"

#include "util/sound/gym.hpp"
#include "util/file/rom.hpp"
#include "gens_core/vdp/vdp_io.h"
#include "gens_core/vdp/vdp_rend.h"
#include "util/file/save.hpp"
#include "mdZ80/mdZ80.h"
#include "util/gfx/imageutil.hpp"

// Sega CD
#include "emulator/g_mcd.hpp"

// 32X
#include "gens_core/cpu/sh2/sh2.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "segacd/cd_aspi.hpp"
#endif /* GENS_CDROM */

// Debugger
#ifdef GENS_DEBUGGER
#include "debugger/debugger.hpp"
#endif /* GENS_DEBUGGER */

// Plugin Manager
#include "plugins/pluginmgr.hpp"

// File management functions.
#include "util/file/file.hpp"

// Audio Handler.
#include "audio/audio.h"

// Needed on Win32
#include "gens_core/mem/mem_m68k.h"
#include "gens_core/sound/ym2612.h"
#include "gens_core/sound/psg.h"
#include "gens_core/sound/pcm.h"
#include "gens_core/sound/pwm.h"
#include "segacd/cd_sys.hpp"

#include <sys/types.h>
#include <unistd.h>

// C includes
#include <cstring>

// C++ includes
#include <string>
#include <list>
using std::string;
using std::list;

// For some reason, these aren't extern'd anywhere...
extern "C"
{
	void main68k_reset();
	void sub68k_reset();
}

// New menu handler.
#include "ui/common/gens/gens_menu.h"
#include "ui/common/gens/gens_menu_callbacks.hpp"

// Non-Menu Command Definitions
#include "gens_window_cmds.h"

// Video, Audio, Input.
#include "video/vdraw.h"
#include "video/vdraw_cpp.hpp"
#include "input/input.h"
#include "input/input_dinput.hpp"

static bool paintsEnabled = true;

static void on_gens_window_close(void);
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
static void fullScreenPopupMenu(HWND hWnd);
static void dragDropFile(HDROP hDrop);


// TODO: If a radio menu item is selected but is already enabled, don't do anything.


/**
 * Gens_Window_WndProc(): The Gens window procedure.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam
 * @param lParam
 * @return
 */
LRESULT CALLBACK Gens_Window_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rectGensWindow;
	HMENU mnuCallback;
	bool state;
	
	switch(message)
	{
		case WM_CLOSE:
			on_gens_window_close();
			return 0;
		
		case WM_CREATE:
			Active = 1;
			break;
		
		case WM_DESTROY:
			// Delete the menu command accelerator table.
			if (hAccelTable_Menu)
			{
				DestroyAcceleratorTable(hAccelTable_Menu);
				hAccelTable_Menu = NULL;
			}
			break;
		
		case WM_EXITSIZEMOVE:
			if (vdraw_get_fullscreen())
				break;
			
			// Save the window coordinates.
			GetWindowRect(hWnd, &rectGensWindow);
			Window_Pos.x = rectGensWindow.left;
			Window_Pos.y = rectGensWindow.top;
			break;
		
		case WM_ACTIVATE:
			if (LOWORD(wParam) != WA_INACTIVE)
			{
				// Set the DirectInput cooperative level.
				input_set_cooperative_level(hWnd);
				
				// Initialize joysticks.
				input_dinput_init_joysticks(hWnd);
				
				// Auto Pause - reactivate the game.
				Active = 1;
			}
			else
			{
				// Auto Pause - deactivate the game.
				if (Auto_Pause && Active)
				{
					Active = 0;
					audio_clear_sound_buffer();
				}
			}
			
			break;
		
		case WM_PAINT:
			HDC hDC;
			PAINTSTRUCT ps;
			
			hDC = BeginPaint(hWnd, &ps);
			
			if (paintsEnabled)
			{
				if (vdraw_restore_primary)
					vdraw_restore_primary();
				vdraw_flip();
			}
			
			EndPaint(hWnd, &ps);
			break;
		
		case WM_RBUTTONDOWN:
			if (vdraw_get_fullscreen())
				fullScreenPopupMenu(hWnd);
			break;
		
		case WM_COMMAND:
			// Menu item.
			switch (LOWORD(wParam) & 0xF000)
			{
				case IDCMD_NONMENU_COMMANDS:
					on_gens_window_NonMenuCmd(hWnd, message, wParam, lParam);
					break;
				
				default:
					// Menu item selected.
					mnuCallback = findMenuItem(LOWORD(wParam));
					state = (GetMenuState(mnuCallback, LOWORD(wParam), MF_BYCOMMAND) & MF_CHECKED);
					GensWindow_MenuItemCallback(LOWORD(wParam), state);
					break;
			}
			break;
		
#ifdef GENS_DEBUGGER
		case WM_KEYDOWN:
			if (Debug)
			{
				// Get modifiers.
				int mod = 0;
				if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
					mod |= GENS_KMOD_CTRL;
				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
					mod |= GENS_KMOD_SHIFT;
				// TODO: Other modifiers: Alt, Win/Meta, etc.
				
				Debug_Event(wParam, mod);
			}
			break;
#endif /* GENS_DEBUGGER */
		
		case WM_DROPFILES:
			// A file was dragged onto the Gens window.
			dragDropFile((HDROP)wParam);
			break;
	}
	
	return DefWindowProc(hWnd, message, wParam, lParam);
}


/**
 * Window is closed.
 */
static void on_gens_window_close(void)
{
	//Modif N - making sure sound doesn't stutter on exit
	if (audio_initialized)
		audio_clear_sound_buffer();
	
	close_gens();
}


/**
 * on_gens_window_NonMenuCmd(): Non-Menu Command has been activated.
 * @param hWnd hWnd of the object sending a message.
 * @param message Message being sent by the object.
 * @param wParam LOWORD(wParam) == Command.
 * @param lParam
 */
static void on_gens_window_NonMenuCmd(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GENS_UNUSED_PARAMETER(hWnd);
	GENS_UNUSED_PARAMETER(message);
	GENS_UNUSED_PARAMETER(lParam);
	
	switch (LOWORD(wParam))
	{
		case IDCMD_ESC:
			if (Quick_Exit)
				close_gens();
			
#ifdef GENS_DEBUGGER
			if (Debug)
			{
				Options::setDebugMode(DEBUG_NONE);
				Paused = 0;
				Sync_Gens_Window_CPUMenu();
			}
			else
#endif /* GENS_DEBUGGER */
			if (Paused)
			{
				Paused = 0;
			}
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case IDCMD_PAUSE:
			if (Paused)
			{
				Paused = 0;
			}
			else
			{
				Paused = 1;
				//Pause_Screen();
				audio_clear_sound_buffer();
			}
			break;
		
		case IDCMD_FRAMESKIP_AUTO:
			Options::setFrameSkip(-1);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_FRAMESKIP_DEC:
			if (Options::frameSkip() == -1)
			{
				Options::setFrameSkip(0);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Options::frameSkip() > 0)
			{
				Options::setFrameSkip(Options::frameSkip() - 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_FRAMESKIP_INC:
			if (Options::frameSkip() == -1)
			{
				Options::setFrameSkip(1);
				Sync_Gens_Window_GraphicsMenu();
			}
			else if (Options::frameSkip() < 8)
			{
				Options::setFrameSkip(Options::frameSkip() + 1);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		
		case IDCMD_SAVESLOT_DEC:
			Options::setSaveSlot((Options::saveSlot() + 9) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SAVESLOT_INC:
			Options::setSaveSlot((Options::saveSlot() + 1) % 10);
			Sync_Gens_Window_FileMenu();
			break;
		
		case IDCMD_SWBLIT:
			Options::setSwRender(!Options::swRender());
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_FASTBLUR:
			Options::setFastBlur(!Options::fastBlur());
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_STRETCH_MODE:
			Options::setStretch((Options::stretch() + 1) % 4);
			Sync_Gens_Window_GraphicsMenu();
			break;
		
		case IDCMD_YM2612_IMPROVED:
			Options::setSoundYM2612_Improved(!Options::soundYM2612_Improved());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_DAC_IMPROVED:
			Options::setSoundDAC_Improved(!Options::soundDAC_Improved());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_PSG_IMPROVED:
			Options::setSoundPSG_Sine(!Options::soundPSG_Sine());
			Sync_Gens_Window_SoundMenu();
			break;
		
		case IDCMD_FPS:
			vdraw_set_fps_enabled(!vdraw_get_fps_enabled());
			break;
		
		case IDCMD_RENDERMODE_DEC:
		{
			list<MDP_Render_t*>::iterator rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
			if (rendMode != PluginMgr::lstRenderPlugins.begin())
			{
				rendMode--;
				vdraw_set_renderer(rendMode);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		}
		case IDCMD_RENDERMODE_INC:
		{
			list<MDP_Render_t*>::iterator rendMode = (vdraw_get_fullscreen() ? rendMode_FS : rendMode_W);
			rendMode++;
			if (rendMode != PluginMgr::lstRenderPlugins.end())
			{
				vdraw_set_renderer(rendMode);
				Sync_Gens_Window_GraphicsMenu();
			}
			break;
		}
		case IDCMD_CHANGE_CD:
			if (SegaCD_Started)
				Change_CD();
			break;
		
		case IDCMD_VDRAW_BACKEND:
		{
			int curBackend = vdraw_cur_backend_id;
			curBackend++;
			if (curBackend >= VDRAW_BACKEND_MAX)
				curBackend = 0;
			Options::setBackend((VDRAW_BACKEND)curBackend);
			break;
		}
		
		default:
		{
			int value = LOWORD(wParam) & 0x0F;
			
			switch (LOWORD(wParam) & 0xFF00)
			{
				case IDCMD_SAVESLOT_NUMERIC:
					// Change the save slot.
					if (value > 9)
						break;
					
					Options::setSaveSlot(value);
					Sync_Gens_Window_FileMenu();
					break;
				
				case IDCMD_ROMHISTORY_NUMERIC:
					// Load a ROM from the ROM History submenu.
					if (value == 0 || value > 9 || ROM::Recent_ROMs.size() < value)
						break;
					
					//if ((Check_If_Kaillera_Running())) return 0;
					if (audio_get_gym_playing())
						Stop_Play_GYM();
					ROM::openROM(ROM::Recent_ROMs.at(value - 1).filename);
					Sync_Gens_Window();
					break;
			}
		}
	}
}


/**
 * fullScreenPopupMenu(): Show the Popup Menu while in fullscreen mode.
 * @param hWnd Window handle.
 */
static void fullScreenPopupMenu(HWND hWnd)
{
	// Full Screen, right mouse button click.
	// Show the popup menu.
	audio_clear_sound_buffer();
	
	// Show the mouse pointer.
	while (ShowCursor(false) >= 0) { }
	while (ShowCursor(true) < 0) { }
	
	POINT pt;
	GetCursorPos(&pt);
	SendMessage(hWnd, WM_PAINT, 0, 0);
	
	// Disable painting while the popup menu is open.
	paintsEnabled = false;
	TrackPopupMenu(MainMenu, TPM_LEFTALIGN | TPM_TOPALIGN, pt.x, pt.y, NULL, hWnd, NULL);
	paintsEnabled = true;
	
	// Hide the mouse pointer.
	while (ShowCursor(true) < 0) { }
	while (ShowCursor(false) >= 0) { }
}


/**
 * dragDropFile(): Called when a file is dragged onto the Gens window.
 */
static void dragDropFile(HDROP hDrop)
{
	char filename[GENS_PATH_MAX];
	unsigned int rval;
	
	rval = DragQueryFile(hDrop, 0, filename, sizeof(filename));
	
	if (rval > 0 && rval < GENS_PATH_MAX)
	{
		// Check that the file exists.
		if (File::Exists(filename))
		{
			// File exists. Open it as a ROM image.
			ROM::openROM(filename);
			Sync_Gens_Window();
		}
	}
	
	DragFinish(hDrop);
}
