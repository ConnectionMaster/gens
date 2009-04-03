/***************************************************************************
 * Gens: Input Handler - SDL Key Mappings.                                 *
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

#ifndef GENS_INPUT_SDL_KEYS_H
#define GENS_INPUT_SDL_KEYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <SDL/SDL_keysym.h>

// Maps key symbols from SDL to OS-independent key symbols.
#define GENS_KEY_BACKSPACE	SDLK_BACKSPACE
#define GENS_KEY_TAB		SDLK_TAB
#define GENS_KEY_CLEAR		SDLK_CLEAR
#define GENS_KEY_RETURN		SDLK_RETURN
#define GENS_KEY_PAUSE		SDLK_PAUSE
#define GENS_KEY_ESCAPE		SDLK_ESCAPE
#define GENS_KEY_SPACE		SDLK_SPACE
#define GENS_KEY_EXCLAIM	SDLK_EXCLAIM
#define GENS_KEY_QUOTEDBL	SDLK_QUOTEDBL
#define GENS_KEY_HASH		SDLK_HASH
#define GENS_KEY_DOLLAR		SDLK_DOLLAR
#define GENS_KEY_AMPERSAND	SDLK_AMPERSAND
#define GENS_KEY_QUOTE		SDLK_QUOTE
#define GENS_KEY_LEFTPAREN	SDLK_LEFTPAREN
#define GENS_KEY_RIGHTPAREN	SDLK_RIGHTPAREN
#define GENS_KEY_ASTERISK	SDLK_ASTERISK
#define GENS_KEY_PLUS		SDLK_PLUS
#define GENS_KEY_COMMA		SDLK_COMMA
#define GENS_KEY_MINUS		SDLK_MINUS
#define GENS_KEY_PERIOD		SDLK_PERIOD
#define GENS_KEY_SLASH		SDLK_SLASH
#define GENS_KEY_COLON		SDLK_COLON
#define GENS_KEY_SEMICOLON	SDLK_SEMICOLON
#define GENS_KEY_LESS		SDLK_LESS
#define GENS_KEY_EQUALS		SDLK_EQUALS
#define GENS_KEY_GREATER	SDLK_GREATER
#define GENS_KEY_QUESTION	SDLK_QUESTION
#define GENS_KEY_AT		SDLK_AT
#define GENS_KEY_LEFTBRACKET	SDLK_LEFTBRACKET
#define GENS_KEY_BACKSLASH	SDLK_BACKSLASH
#define GENS_KEY_RIGHTBRACKET	SDLK_RIGHTBRACKET
#define GENS_KEY_CARET		SDLK_CARET
#define GENS_KEY_UNDERSCORE	SDLK_UNDERSCORE
#define GENS_KEY_BACKQUOTE	SDLK_BACKQUOTE

// Numbers
#define GENS_KEY_0		SDLK_0
#define GENS_KEY_1		SDLK_1
#define GENS_KEY_2		SDLK_2
#define GENS_KEY_3		SDLK_3
#define GENS_KEY_4		SDLK_4
#define GENS_KEY_5		SDLK_5
#define GENS_KEY_6		SDLK_6
#define GENS_KEY_7		SDLK_7
#define GENS_KEY_8		SDLK_8
#define GENS_KEY_9		SDLK_9

// Letters
#define	GENS_KEY_a		SDLK_a
#define	GENS_KEY_b		SDLK_b
#define	GENS_KEY_c		SDLK_c
#define	GENS_KEY_d		SDLK_d
#define	GENS_KEY_e		SDLK_e
#define	GENS_KEY_f		SDLK_f
#define	GENS_KEY_g		SDLK_g
#define	GENS_KEY_h		SDLK_h
#define	GENS_KEY_i		SDLK_i
#define	GENS_KEY_j		SDLK_j
#define	GENS_KEY_k		SDLK_k
#define	GENS_KEY_l		SDLK_l
#define	GENS_KEY_m		SDLK_m
#define	GENS_KEY_n		SDLK_n
#define	GENS_KEY_o		SDLK_o
#define	GENS_KEY_p		SDLK_p
#define	GENS_KEY_q		SDLK_q
#define	GENS_KEY_r		SDLK_r
#define	GENS_KEY_s		SDLK_s
#define	GENS_KEY_t		SDLK_t
#define	GENS_KEY_u		SDLK_u
#define	GENS_KEY_v		SDLK_v
#define	GENS_KEY_w		SDLK_w
#define	GENS_KEY_x		SDLK_x
#define	GENS_KEY_y		SDLK_y
#define	GENS_KEY_z		SDLK_z

// Numeric Keypad
#define GENS_KEY_NUM_0		SDLK_KP0
#define GENS_KEY_NUM_1		SDLK_KP1
#define GENS_KEY_NUM_2		SDLK_KP2
#define GENS_KEY_NUM_3		SDLK_KP3
#define GENS_KEY_NUM_4		SDLK_KP4
#define GENS_KEY_NUM_5		SDLK_KP5
#define GENS_KEY_NUM_6		SDLK_KP6
#define GENS_KEY_NUM_7		SDLK_KP7
#define GENS_KEY_NUM_8		SDLK_KP8
#define GENS_KEY_NUM_9		SDLK_KP9
#define GENS_KEY_NUM_PERIOD	SDLK_KP_PERIOD
#define GENS_KEY_NUM_DIVIDE	SDLK_KP_DIVIDE
#define GENS_KEY_NUM_MULTIPLY	SDLK_KP_MULTIPLY
#define GENS_KEY_NUM_MINUS	SDLK_KP_MINUS
#define GENS_KEY_NUM_PLUS	SDLK_KP_PLUS
#define GENS_KEY_NUM_ENTER	SDLK_KP_ENTER
#define GENS_KEY_NUM_EQUALS	SDLK_KP_EQUALS	// lol Macintosh

// Arrow Keys
#define GENS_KEY_UP		SDLK_UP
#define GENS_KEY_DOWN		SDLK_DOWN
#define GENS_KEY_LEFT		SDLK_LEFT
#define GENS_KEY_RIGHT		SDLK_RIGHT

// Home/End section
#define GENS_KEY_INSERT		SDLK_INSERT
#define GENS_KEY_DELETE		SDLK_DELETE
#define GENS_KEY_HOME		SDLK_HOME
#define GENS_KEY_END		SDLK_END
#define GENS_KEY_PAGEUP		SDLK_PAGEUP
#define GENS_KEY_PAGEDOWN	SDLK_PAGEDOWN

// Function Keys
#define GENS_KEY_F1		SDLK_F1
#define GENS_KEY_F2		SDLK_F2
#define GENS_KEY_F3		SDLK_F3
#define GENS_KEY_F4		SDLK_F4
#define GENS_KEY_F5		SDLK_F5
#define GENS_KEY_F6		SDLK_F6
#define GENS_KEY_F7		SDLK_F7
#define GENS_KEY_F8		SDLK_F8
#define GENS_KEY_F9		SDLK_F9
#define GENS_KEY_F10		SDLK_F10
#define GENS_KEY_F11		SDLK_F11
#define GENS_KEY_F12		SDLK_F12
#define GENS_KEY_F13		SDLK_F13
#define GENS_KEY_F14		SDLK_F14
#define GENS_KEY_F15		SDLK_F15

// Modifier Keys (not the same as modifiers, which are included with regular keypresses)
#define GENS_KEY_NUMLOCK	SDLK_NUMLOCK
#define GENS_KEY_CAPSLOCK	SDLK_CAPSLOCK
#define GENS_KEY_SCROLLLOCK	SDLK_SCROLLOCK
#define GENS_KEY_LSHIFT		SDLK_LSHIFT
#define GENS_KEY_RSHIFT		SDLK_RSHIFT
#define GENS_KEY_LCTRL		SDLK_LCTRL
#define GENS_KEY_RCTRL		SDLK_RCTRL
#define GENS_KEY_LALT		SDLK_LALT
#define GENS_KEY_RALT		SDLK_RALT
#define GENS_KEY_LMETA		SDLK_LMETA
#define GENS_KEY_RMETA		SDLK_RMETA
#define GENS_KEY_LSUPER		SDLK_LSUPER
#define GENS_KEY_RSUPER		SDLK_RSUPER
#define GENS_KEY_MODE		SDLK_MODE
#define GENS_KEY_COMPOSE	SDLK_COMPOSE

// Miscellaneous function keys
#define GENS_KEY_HELP		SDLK_HELP
#define GENS_KEY_PRINT		SDLK_PRINT
#define GENS_KEY_SYSREQ		SDLK_SYSREQ
#define GENS_KEY_BREAK		SDLK_BREAK
#define GENS_KEY_MENU		SDLK_MENU
#define GENS_KEY_POWER		SDLK_POWER
#define GENS_KEY_EURO		SDLK_EURO
#define GENS_KEY_UNDO		SDLK_UNDO

// Maps key modifiers from SDL to OS-independent key modifiers.
#define GENS_KMOD_NONE		KMOD_NONE
#define GENS_KMOD_LSHIFT	KMOD_LSHIFT
#define GENS_KMOD_RSHIFT	KMOD_RSHIFT
#define GENS_KMOD_LCTRL		KMOD_LCTRL
#define GENS_KMOD_RCTRL		KMOD_RCTRL
#define GENS_KMOD_LALT		KMOD_LALT
#define GENS_KMOD_RALT		KMOD_RALT
#define GENS_KMOD_LMETA		KMOD_LMETA
#define GENS_KMOD_RMETA		KMOD_RMETA
#define GENS_KMOD_NUM		KMOD_NUM
#define GENS_KMOD_CAPS		KMOD_CAPS
#define GENS_KMOD_MODE		KMOD_MODE

// Composite modifiers.
#define GENS_KMOD_CTRL		KMOD_CTRL
#define GENS_KMOD_SHIFT		KMOD_SHIFT
#define GENS_KMOD_ALT		KMOD_ALT
#define GENS_KMOD_META		KMOD_META

#ifdef __cplusplus
}
#endif

#endif /* GENS_INPUT_SDL_KEYS_H */
