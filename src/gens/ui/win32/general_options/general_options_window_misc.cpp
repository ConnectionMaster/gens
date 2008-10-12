/***************************************************************************
 * Gens: (Win32) General Options Window - Miscellaneous Functions.         *
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

#include <string.h>

#include "general_options_window.h"
#include "general_options_window_callbacks.h"
#include "general_options_window_misc.hpp"
#include "gens/gens_window.h"

#include "emulator/g_main.hpp"

#include <windows.h>
#include <windowsx.h>


/**
 * Open_General_Options(): Opens the General Options window.
 */
void Open_General_Options(void)
{
	HWND go = create_general_options_window();
	if (!go)
	{
		// Either an error occurred while creating the General Options window,
		// or the General Options window is already created.
		return;
	}
	
	// TODO: Make the window modal.
	//gtk_window_set_transient_for(GTK_WINDOW(go), GTK_WINDOW(gens_window));
	
	// Get the current options.
	unsigned char curFPSStyle, curMsgStyle;
	
	// System
	Button_SetCheck(go_chkAutoFixChecksum, (Auto_Fix_CS ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkAutoPause, (Auto_Pause ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkFastBlur, (draw->fastBlur() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkSegaCDLEDs, (Show_LED ? BST_CHECKED : BST_UNCHECKED));
	
	// FPS counter
	curFPSStyle = draw->fpsStyle();
	Button_SetCheck(go_chkMsgEnable[0], (draw->fpsEnabled() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgDoubleSized[0], ((curFPSStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgTransparency[0], ((curFPSStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	go_stcColor_State[0] = (curFPSStyle & 0x06) >> 1;
	
	// Message
	curMsgStyle = draw->msgStyle();
	Button_SetCheck(go_chkMsgEnable[1], (draw->msgEnabled() ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgDoubleSized[1], ((curMsgStyle & 0x10) ? BST_CHECKED : BST_UNCHECKED));
	Button_SetCheck(go_chkMsgTransparency[1], ((curMsgStyle & 0x08) ? BST_CHECKED : BST_UNCHECKED));
	go_stcColor_State[1] = (curMsgStyle & 0x06) >> 1;
	
	// Intro effect color
	go_stcColor_State[2] = draw->introEffectColor();
	
	// Show the General Options window.
	ShowWindow(go, 1);
}


/**
 * General_Options_Save(): Save the General Options.
 */
void General_Options_Save(void)
{
#if 0
	GtkWidget *check_system_autofixchecksum, *check_system_autopause;
	GtkWidget *check_system_fastblur, *check_system_segacd_leds;
	unsigned char curFPSStyle;
	GtkWidget *check_fps_enable, *check_fps_doublesized;
	GtkWidget *check_fps_transparency, *radio_button_fps_color;
	unsigned char curMsgStyle;
	GtkWidget *check_message_enable, *check_message_doublesized;
	GtkWidget *check_message_transparency, *radio_button_message_color;
	GtkWidget *radio_button_intro_effect_color;
	char tmp[64]; short i;
	
	// Save the current options.
	
	// System
	check_system_autofixchecksum = lookup_widget(general_options_window, "check_system_autofixchecksum");
	Auto_Fix_CS = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autofixchecksum));
	check_system_autopause = lookup_widget(general_options_window, "check_system_autopause");
	Auto_Pause = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_autopause));
	check_system_fastblur = lookup_widget(general_options_window, "check_system_fastblur");
	draw->setFastBlur(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_fastblur)));
	check_system_segacd_leds = lookup_widget(general_options_window, "check_system_segacd_leds");
	Show_LED = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_system_segacd_leds));
	
	// FPS counter
	check_fps_enable = lookup_widget(general_options_window, "check_fps_enable");
	draw->setFPSEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_enable)));
	
	curFPSStyle = draw->fpsStyle() & ~0x18;
	check_fps_doublesized = lookup_widget(general_options_window, "check_fps_doublesized");
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_doublesized)) ? 0x10 : 0x00);
	check_fps_transparency = lookup_widget(general_options_window, "check_fps_transparency");
	curFPSStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_fps_transparency)) ? 0x08 : 0x00);
	
	// FPS counter color
	for (i = 0; i < 4; i++)
	{
		if (!GO_MsgColors[i])
			break;
		sprintf(tmp, "radio_button_fps_color_%s", GO_MsgColors[i * 3]);
		radio_button_fps_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_fps_color)))
		{
			curFPSStyle &= ~0x06;
			curFPSStyle |= (i << 1);
			break;
		}
	}
	
	draw->setFPSStyle(curFPSStyle);
	
	// Message
	check_message_enable = lookup_widget(general_options_window, "check_message_enable");
	draw->setMsgEnabled(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_enable)));
	
	curMsgStyle = draw->msgStyle() & ~0x18;
	check_message_doublesized = lookup_widget(general_options_window, "check_message_doublesized");
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_doublesized)) ? 0x10 : 0x00);
	check_message_transparency = lookup_widget(general_options_window, "check_message_transparency");
	curMsgStyle |= (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_message_transparency)) ? 0x08 : 0x00);
	
	// Message color
	for (i = 0; i < 4; i++)
	{
		if (!GO_MsgColors[i])
			break;
		sprintf(tmp, "radio_button_message_color_%s", GO_MsgColors[i * 3]);
		radio_button_message_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_message_color)))
		{
			curMsgStyle &= ~0x06;
			curMsgStyle |= (i << 1);
			break;
		}
	}
	
	draw->setMsgStyle(curMsgStyle);
	
	// Intro effect color
	for (i = 0; i < 8; i++)
	{
		if (!GO_IntroEffectColors[i])
			break;
		sprintf(tmp, "radio_button_misc_intro_effect_color_%s", GO_IntroEffectColors[i * 3]);
		radio_button_intro_effect_color = lookup_widget(general_options_window, tmp);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_button_intro_effect_color)))
		{
			draw->setIntroEffectColor((unsigned char)i);
			break;
		}
	}
#endif
}
