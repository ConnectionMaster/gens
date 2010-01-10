/***************************************************************************
 * Gens: (Win32) Plugin Manager Window.                                    *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008-2010 by David Korth                                  *
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

#include "pmgr_window.hpp"
#include "ui/common/pmgr_window_common.hpp"
#include "gens/gens_window.h"
#include "emulator/g_main.hpp"

// C includes.
#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Win32 includes.
#include "libgsft/w32u/w32u_windows.h"
#include "libgsft/w32u/w32u_windowsx.h"
#include "libgsft/w32u/w32u_commctrl.h"
#include "ui/win32/fonts.h"
#include "ui/win32/resource.h"

// libgsft includes.
#include "libgsft/gsft_win32.h"
#include "libgsft/gsft_szprintf.h"

// Plugin Manager
#include "plugins/pluginmgr.hpp"
#include "plugins/mdp_incompat.hpp"
#include "mdp/mdp_error.h"

// File functions.
#include "util/file/file.hpp"

// C++ includes.
#include <string>
#include <sstream>
#include <list>
#include <vector>
using std::string;
using std::stringstream;
using std::list;
using std::vector;

// Win32 line ending.
// For some reason, MinGW's std::endl always outputs "\n", even on Windows.
// This works on Wine, but not on Windows.
#define WIN32_ENDL "\r\n"


// Window.
HWND pmgr_window = NULL;

// Window class.
static WNDCLASS pmgr_wndclass;

// Window size. (NOTE: THESE ARE IN DIALOG UNITS, and must be converted to pixels using DLU_X() / DLU_Y().)
#define PMGR_WINDOW_WIDTH  (190+5+5)
#define PMGR_WINDOW_HEIGHT (5+125+5+155+5+15+5)

#define PMGR_FRAME_PLUGIN_LIST_WIDTH  (PMGR_WINDOW_WIDTH-5-5)
#define PMGR_FRAME_PLUGIN_LIST_HEIGHT 125

#define PMGR_FRAME_PLUGIN_INFO_WIDTH  (PMGR_WINDOW_WIDTH-5-5)
#define PMGR_FRAME_PLUGIN_INFO_HEIGHT 155

// Window procedure.
static LRESULT CALLBACK pmgr_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Widgets.
static HWND	lblPluginMainInfo;
static HWND	lblPluginSecInfo;
static HWND	lblPluginDesc;

static HWND	tabPluginList;
static HWND	lstPluginList[PMGR_MAX];

// Widget creation functions.
static void WINAPI pmgr_window_create_child_windows(HWND hWnd);
static void WINAPI pmgr_window_create_plugin_list_tab_control(HWND container);
static void WINAPI pmgr_window_create_plugin_list_tab(HWND container, const char* title, int id);
static void WINAPI pmgr_window_create_plugin_info_frame(HWND container);
static void WINAPI pmgr_window_populate_plugin_lists(void);
static void WINAPI pmgr_window_add_plugin_to_list(mdp_t *plugin, int err, const string& filename = "");

// Callbacks.
static void WINAPI pmgr_window_callback_lstPluginList_cursor_changed(int id);

// Hashtable of error codes.
#include "libgsft/gsft_hashtable.hpp"
typedef GSFT_HASHTABLE<mdp_t*, int> mapMdpErrs_t;
static mapMdpErrs_t mapMdpErrs;
typedef std::pair<mdp_t*, int> pairMdpErr_t;

// Plugin icon functions and variables.
#ifdef GENS_PNG
#include "libgsft/gsft_png.h"
#include "libgsft/gsft_png_dll.h"

static HWND		imgPluginIcon;
static HIMAGELIST	imglPluginIcons = NULL;
static vector<HBITMAP>	vectPluginIcons;

static void WINAPI pmgr_window_create_plugin_icon_widget(HWND container);
static HBITMAP WINAPI pmgr_window_create_bitmap_from_png(const uint8_t *icon, const unsigned int iconLength);
#endif


/**
 * pmgr_window_show(): Show the Plugin Manager window.
 */
void pmgr_window_show(void)
{
	if (pmgr_window)
	{
		// Plugin Manager window is already visible. Set focus.
		// TODO: Figure out how to do this.
		ShowWindow(pmgr_window, SW_SHOW);
		return;
	}
	
	if (pmgr_wndclass.lpfnWndProc != pmgr_window_wndproc)
	{
		// Create the window class.
		pmgr_wndclass.style = 0;
		pmgr_wndclass.lpfnWndProc = pmgr_window_wndproc;
		pmgr_wndclass.cbClsExtra = 0;
		pmgr_wndclass.cbWndExtra = 0;
		pmgr_wndclass.hInstance = ghInstance;
		pmgr_wndclass.hIcon = LoadIconA(ghInstance, MAKEINTRESOURCE(IDI_GENS_APP));
		pmgr_wndclass.hCursor = NULL;
		pmgr_wndclass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
		pmgr_wndclass.lpszMenuName = NULL;
		pmgr_wndclass.lpszClassName = "pmgr_window";
		
		pRegisterClassU(&pmgr_wndclass);
	}
	
	// Create the window.
	pmgr_window = pCreateWindowU("pmgr_window", "Plugin Manager",
					WS_DLGFRAME | WS_POPUP | WS_SYSMENU | WS_CAPTION,
					CW_USEDEFAULT, CW_USEDEFAULT,
					DLU_X(PMGR_WINDOW_WIDTH), DLU_Y(PMGR_WINDOW_HEIGHT),
					gens_window, NULL, ghInstance, NULL);
	
	// Set the actual window size.
	gsft_win32_set_actual_window_size(pmgr_window, DLU_X(PMGR_WINDOW_WIDTH), DLU_Y(PMGR_WINDOW_HEIGHT));
	
	// Center the window on the Gens window.
	gsft_win32_center_on_window(pmgr_window, gens_window);
	
	UpdateWindow(pmgr_window);
	ShowWindow(pmgr_window, 1);
}


/**
 * cc_window_create_child_windows(): Create child windows.
 * @param hWnd HWND of the parent window.
 */
static void WINAPI pmgr_window_create_child_windows(HWND hWnd)
{
	// Create the plugin list tab control.
	pmgr_window_create_plugin_list_tab_control(hWnd);
	
	// Create the plugin information frame.
	pmgr_window_create_plugin_info_frame(hWnd);
	
	// Create the "OK" button.
	HWND btnOK = pCreateWindowU(WC_BUTTON, "&OK",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
					DLU_X(PMGR_WINDOW_WIDTH-5-50), DLU_Y(PMGR_WINDOW_HEIGHT-5-14),
					DLU_X(50), DLU_Y(14),
					hWnd, (HMENU)IDOK, ghInstance, NULL);
	SetWindowFontU(btnOK, fntMain, true);
	
	// Populate the plugin lists.
	pmgr_window_populate_plugin_lists();
	
	// Initialize the plugin description frame.
	pmgr_window_callback_lstPluginList_cursor_changed(PMGR_INTERNAL);
}


/**
 * pmgr_window_create_plugin_list_tabs(): Create the plugin list tab control.
 * @param container Container for the tabs.
 */
static void WINAPI pmgr_window_create_plugin_list_tab_control(HWND container)
{
	// Create the plugin list tab control.
	tabPluginList = pCreateWindowU(WC_TABCONTROL, NULL,
					WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
					DLU_X(5), DLU_Y(5),
					DLU_X(PMGR_FRAME_PLUGIN_LIST_WIDTH),
					DLU_Y(PMGR_FRAME_PLUGIN_LIST_HEIGHT),
					container, NULL, ghInstance, NULL);
	SetWindowFontU(tabPluginList, fntMain, true);
	
#ifdef GENS_PNG
	// Create the ImageList.
	imglPluginIcons = ImageList_Create(32, 32, ILC_MASK | ILC_COLOR32,
					   PluginMgr::lstMDP.size(),
					   PluginMgr::lstMDP.size());
	vectPluginIcons.clear();
	vectPluginIcons.reserve(PluginMgr::lstMDP.size());
#endif
	
	// Create the tabs.
	pmgr_window_create_plugin_list_tab(tabPluginList, "&Internal", PMGR_INTERNAL);
	pmgr_window_create_plugin_list_tab(tabPluginList, "&External", PMGR_EXTERNAL);
	pmgr_window_create_plugin_list_tab(tabPluginList, "I&ncompatible", PMGR_INCOMPAT);
}


/**
 * pmgr_window_create_plugin_list_tabs(): Create a plugin list tab.
 * @param container Container for the tab.
 * @param title Title of the tab.
 * @param id Tab ID.
 */
static void WINAPI pmgr_window_create_plugin_list_tab(HWND container, const char* title, int id)
{
	// Insert a tab.
	TCITEM tabItem;
	tabItem.mask = TCIF_TEXT;
	tabItem.pszText = const_cast<char*>(title);
	pTabCtrl_InsertItemU(container, id, &tabItem);
	
	// Create the plugin ListView.
	lstPluginList[id] = pCreateWindowExU(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL,
						WS_CHILD | WS_TABSTOP | WS_BORDER | WS_VSCROLL |
						LVS_REPORT | LVS_NOCOLUMNHEADER | LVS_SINGLESEL | LVS_SHOWSELALWAYS | LVS_SHAREIMAGELISTS,
						DLU_X(5), DLU_Y(10+5+2),
						DLU_X(PMGR_FRAME_PLUGIN_LIST_WIDTH-10),
						DLU_Y(PMGR_FRAME_PLUGIN_LIST_HEIGHT-15-5-2),
						container, (HMENU)(IDC_PMGR_WINDOW_LSTPLUGINLIST + id), ghInstance, NULL);
	SetWindowFontU(lstPluginList[id], fntMain, true);
	ListView_SetExtendedListViewStyleU(lstPluginList[id], LVS_EX_FULLROWSELECT);
	
	if (id == PMGR_INTERNAL)
	{
		// Show the "Internal" ListView initially..
		ShowWindow(lstPluginList[id], SW_SHOW);
	}
	
#ifdef GENS_PNG
	// Set the ListView's ImageList.
	// "Small" is set in addition to "Normal", since LVS_REPORT uses "Small" icons.
	ListView_SetImageListU(lstPluginList[id], imglPluginIcons, LVSIL_NORMAL);
	ListView_SetImageListU(lstPluginList[id], imglPluginIcons, LVSIL_SMALL);
#endif
	
	// Create the ListView columns.
	LV_COLUMN lvCol;
	memset(&lvCol, 0, sizeof(lvCol));
	lvCol.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
	
	// Icon.
	lvCol.pszText = "Icon";
#ifdef GENS_PNG
	lvCol.cx = 32;
#else
	lvCol.cx = 0;
#endif
	pListView_InsertColumnU(lstPluginList[id], 0, &lvCol);
	
	// Plugin name.
	lvCol.pszText = "Plugin Name";
#ifdef GENS_PNG
	lvCol.cx = DLU_X(PMGR_FRAME_PLUGIN_LIST_WIDTH-10-15)-32;
#else
	lvCol.cx = DLU_X(PMGR_FRAME_PLUGIN_LIST_WIDTH-10-15);
#endif
	pListView_InsertColumnU(lstPluginList[id], 1, &lvCol);
}


/**
 * pmgr_window_create_plugin_info_frame(): Create the plugin information frame.
 * @param container Container for the frame.
 */
static void WINAPI pmgr_window_create_plugin_info_frame(HWND container)
{
	const int top = DLU_Y(5+PMGR_FRAME_PLUGIN_LIST_HEIGHT+5);
	
	HWND fraPluginInfo = pCreateWindowU(WC_BUTTON, "Plugin Information",
						WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
						DLU_X(5), top,
						DLU_X(PMGR_FRAME_PLUGIN_INFO_WIDTH),
						DLU_Y(PMGR_FRAME_PLUGIN_INFO_HEIGHT),
						container, NULL, ghInstance, NULL);
	SetWindowFontU(fraPluginInfo, fntMain, true);
	
#ifdef GENS_PNG
	// Create the plugin icon widget.
	pmgr_window_create_plugin_icon_widget(container);
#endif /* GENS_PNG */
	
	// Label for the main plugin info.
#ifdef GENS_PNG
	const int lblPluginMainInfo_Left = DLU_X(5+5+5)+32;
#else /* !GENS_PNG */
	const int lblPluginMainInfo_Left = DLU_X(5+5);
#endif /* GENS_PNG */
	const int lblPluginMainInfo_Height = DLU_Y(60);
	
	lblPluginMainInfo = pCreateWindowU(WC_EDIT, NULL,
						WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
						lblPluginMainInfo_Left, top+DLU_Y(10),
						DLU_X(PMGR_FRAME_PLUGIN_INFO_WIDTH) - lblPluginMainInfo_Left,
						lblPluginMainInfo_Height,
						container, NULL, ghInstance, NULL);
	SetWindowFontU(lblPluginMainInfo, fntMain, true);
	Edit_SetReadOnlyU(lblPluginMainInfo, true);
	
	// Label for secondary plugin info.
	const int lblPluginSecInfo_Height = DLU_Y(25);
	lblPluginSecInfo = pCreateWindowU(WC_EDIT, NULL,
						WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
						DLU_X(5+5), top+lblPluginMainInfo_Height+DLU_Y(10+5),
						DLU_X(PMGR_FRAME_PLUGIN_INFO_WIDTH-5-5),
						lblPluginSecInfo_Height,
						container, NULL, ghInstance, NULL);
	SetWindowFontU(lblPluginSecInfo, fntMain, true);
	Edit_SetReadOnlyU(lblPluginSecInfo, true);
	
	// Label for the plugin description.
	const int lblPluginDesc_Height = DLU_Y(45);
	lblPluginDesc = pCreateWindowU(WC_EDIT, NULL,
					WS_CHILD | WS_VISIBLE | ES_LEFT | ES_MULTILINE,
					DLU_X(5+5), top+lblPluginMainInfo_Height+lblPluginSecInfo_Height+DLU_Y(10+5+5),
					DLU_X(PMGR_FRAME_PLUGIN_INFO_WIDTH-5-5),
					lblPluginDesc_Height,
					container, NULL, ghInstance, NULL);
	SetWindowFontU(lblPluginDesc, fntMain, true);
	Edit_SetReadOnlyU(lblPluginDesc, true);
}


/**
 * pmgr_window_populate_plugin_lists(): Populate the plugin list.
 */
static void WINAPI pmgr_window_populate_plugin_lists(void)
{
	// Clear the plugin lists.
	for (int i = 0; i < PMGR_MAX; i++)
	{
		if (!lstPluginList[i])
			return;
		ListView_DeleteAllItemsU(lstPluginList[i]);
	}
	mapMdpErrs.clear();
	
	// Add all loaded plugins to the ListView.
	for (list<mdp_t*>::iterator curPlugin = PluginMgr::lstMDP.begin();
	     curPlugin != PluginMgr::lstMDP.end(); curPlugin++)
	{
		mdp_t *plugin = (*curPlugin);
		pmgr_window_add_plugin_to_list(plugin, MDP_ERR_OK);
	}
	
	// Add incompatible plugins to PMGR_INCOMPAT.
	for (list<mdp_incompat_t>::iterator curIncompat = PluginMgr::Incompat.begin();
	     curIncompat != PluginMgr::Incompat.end(); curIncompat++)
	{
		const mdp_incompat_t& incompat = (*curIncompat);
		pmgr_window_add_plugin_to_list(incompat.mdp, incompat.err, incompat.filename);
	}
}


/**
 * pmgr_window_add_plugin_to_list(): Add a plugin to the ListViews.
 * @param plugin mdp_t* of the plugin.
 * @param err MDP error code. If not MDP_ERR_OK, the plugin is added to PMGR_INCOMPAT.
 * @param filename Filename of the plugin, if available.
 */
static void WINAPI pmgr_window_add_plugin_to_list(mdp_t *plugin, int err, const string& filename)
{
	if (!plugin)
		return;
	
	string pluginName;
	pmgr_type_t pmType;
	
	// Determine the plugin name.
	if (plugin->desc && plugin->desc->name)
	{
		pluginName = string(plugin->desc->name);
	}
	else
	{
		// No description or name.
		char tmp[64];
		szprintf(tmp, sizeof(tmp), "[No name: 0x%08lX]", (unsigned long)plugin);
		pluginName = string(tmp);
	}
	
	// Check for MDP DLL information.
	if (filename.empty())
	{
		// Specified filename is empty.
		// Check PluginMgr::tblMdpDLL for the plugin information.
		mapMdpDLL::iterator dllIter = PluginMgr::tblMdpDLL.find(plugin);
		if (dllIter != PluginMgr::tblMdpDLL.end())
		{
			// External plugin.
			pmType = PMGR_EXTERNAL;
			
			const mdpDLL_t& dll = (*dllIter).second;
			pluginName += " (" + File::GetNameFromPath(dll.filename) + ")";
		}
		else
		{
			// Internal plugin.
			pmType = PMGR_INTERNAL;
		}
	}
	else
	{
		// Filename was specified. Assume external plugin.
		pmType = PMGR_EXTERNAL;
		pluginName += " (" + File::GetNameFromPath(filename) + ")";
	}
	
	// If err isn't MDP_ERR_OK, add to PMGR_INCOMPAT.
	if (err != 0)
	{
		pmType = PMGR_INCOMPAT;
		
		// Add the error code to the MDP error code map.
		mapMdpErrs.insert(pairMdpErr_t(plugin, err));
	}
	
	LVITEM lviPlugin;
	memset(&lviPlugin, 0x00, sizeof(lviPlugin));
	
#ifdef GENS_PNG
	// Add the plugin's icon to the ImageList.
	lviPlugin.iImage = -1;
	HBITMAP hbmpIcon = NULL;
	if (plugin->desc)
	{
		hbmpIcon = pmgr_window_create_bitmap_from_png(plugin->desc->icon, plugin->desc->iconLength);
	}
	
	if (hbmpIcon)
	{
		lviPlugin.iImage = ImageList_Add(imglPluginIcons, hbmpIcon, NULL);
		vectPluginIcons.push_back(hbmpIcon);
	}
#endif
	
#ifdef GENS_PNG
	lviPlugin.mask = LVIF_IMAGE | LVIF_PARAM;
#else
	lviPlugin.mask = LVIF_PARAM;
#endif
	lviPlugin.cchTextMax = 256;
	lviPlugin.iItem = ListView_GetItemCountU(lstPluginList[pmType]);
	lviPlugin.lParam = (LPARAM)plugin;
	
	// First column: Icon.
	lviPlugin.iSubItem = 0;
	lviPlugin.pszText = NULL;
	pListView_InsertItemU(lstPluginList[pmType], &lviPlugin);
	
	// lParam doesn't need to be set for the subitems.
	lviPlugin.mask = LVIF_TEXT;
	lviPlugin.lParam = NULL;
	
	// Second column: Plugin name.
	lviPlugin.iSubItem = 1;
	lviPlugin.pszText = const_cast<char*>(pluginName.c_str());
	pListView_SetItemU(lstPluginList[pmType], &lviPlugin);
}


/**
 * pmgr_window_close(): Close the Plugin Manager window.
 */
void pmgr_window_close(void)
{
	if (!pmgr_window)
		return;
	
	// Destroy the window.
	DestroyWindow(pmgr_window);
	pmgr_window = NULL;
	
	// Clear the map of MDP errors.
	mapMdpErrs.clear();
	
#ifdef GENS_PNG
	// Make sure the ListViews are destroyed first.
	for (int i = 0; i < PMGR_MAX; i++)
	{
		DestroyWindow(lstPluginList[i]);
		lstPluginList[i] = NULL;
	}
	
	// Destroy the image list.
	if (imglPluginIcons)
	{
		ImageList_Destroy(imglPluginIcons);
		imglPluginIcons = NULL;
	}
	
	// Destroy the vector of HBITMAPs.
	for (int i = vectPluginIcons.size() - 1; i >= 0; i--)
	{
		DeleteBitmap(vectPluginIcons[i]);
	}
	vectPluginIcons.clear();
#endif
}


/**
 * pmgr_window_wndproc(): Window procedure.
 * @param hWnd hWnd of the window.
 * @param message Window message.
 * @param wParam
 * @param lParam
 * @return
 */
static LRESULT CALLBACK pmgr_window_wndproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		case WM_CREATE:
			pmgr_window_create_child_windows(hWnd);
			break;
		
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDOK:
				case IDCANCEL:
					pmgr_window_close();
					break;
				
				default:
					// Unknown command identifier.
					break;
			}			
			break;
		
		case WM_NOTIFY:
		{
			LPNMHDR nmHdr = (LPNMHDR)lParam;
			
			switch (nmHdr->code)
			{
				case LVN_ITEMCHANGED:
				{
					LPNMLISTVIEW nmListView = (LPNMLISTVIEW)lParam;
					
					if ((nmListView->hdr.idFrom & 0xFF00) == IDC_PMGR_WINDOW_LSTPLUGINLIST &&
					    (nmListView->uChanged & LVIF_STATE) &&
					    (!(nmListView->uOldState & LVIS_SELECTED)) &&
					    (nmListView->uNewState & LVIS_SELECTED))
					{
						// Item was newly selected.
						pmgr_window_callback_lstPluginList_cursor_changed(nmListView->hdr.idFrom & 0xFF);
					}
					break;
				}
				case TCN_SELCHANGE:
				{
					if (nmHdr->hwndFrom == tabPluginList)
					{
						// Tab was changed.
						int selTab = TabCtrl_GetCurSelU(nmHdr->hwndFrom);
						for (int i = 0; i < PMGR_MAX; i++)
						{
							ShowWindow(lstPluginList[i], ((i == selTab) ? SW_SHOW : SW_HIDE));
						}
						
						// Activate the "cursor-changed" callback for the page.
						pmgr_window_callback_lstPluginList_cursor_changed(selTab);
					}
				}
			}
			break;
		}
		
		case WM_DESTROY:
			if (hWnd != pmgr_window)
				break;
			
			pmgr_window = NULL;
			
			// Clear the map of MDP errors.
			mapMdpErrs.clear();
			
#ifdef GENS_PNG
			// Make sure the ListViews are destroyed first.
			for (int i = 0; i < PMGR_MAX; i++)
			{
				DestroyWindow(lstPluginList[i]);
				lstPluginList[i] = NULL;
			}
			
			// Destroy the image list.
			if (imglPluginIcons)
			{
				ImageList_Destroy(imglPluginIcons);
				imglPluginIcons = NULL;
			}
			
			// Destroy the vector of HBITMAPs.
			for (int i = vectPluginIcons.size() - 1; i >= 0; i--)
			{
				DeleteBitmap(vectPluginIcons[i]);
			}
			vectPluginIcons.clear();
#endif
			
			break;
	}
	
	return pDefWindowProcU(hWnd, message, wParam, lParam);
}


/**
 * pmgr_window_callback_lstPluginList_cursor_changed(): Cursor position has changed.
 * @param id Plugin list ID.
 */
static void WINAPI pmgr_window_callback_lstPluginList_cursor_changed(int id)
{
	// Check which plugin is clicked.
	int index = ListView_GetNextItemU(lstPluginList[id], -1, LVNI_SELECTED);
	
	if (index == -1)
	{
		// No plugin selected.
		Edit_SetTextU(lblPluginMainInfo, "No plugin selected.");
		Edit_SetTextU(lblPluginSecInfo, NULL);
		Edit_SetTextU(lblPluginDesc, NULL);
#ifdef GENS_PNG
		pSendMessageU(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	// Found a selected plugin.
	
	LVITEM lvItem;
	lvItem.iItem = index;
	lvItem.iSubItem = 0;
#ifdef GENS_PNG
	lvItem.mask = LVIF_IMAGE | LVIF_PARAM;
#else
	lvItem.mask = LVIF_PARAM;
#endif
	int rval = pListView_GetItemU(lstPluginList[id], &lvItem);
	
	mdp_t *plugin = reinterpret_cast<mdp_t*>(lvItem.lParam);
	
	// Get the plugin information.
	if (!rval || !plugin)
	{
		// Invalid plugin.
		Edit_SetTextU(lblPluginMainInfo, "Invalid plugin selected.");
		Edit_SetTextU(lblPluginSecInfo, NULL);
		Edit_SetTextU(lblPluginDesc, NULL);
#ifdef GENS_PNG
		pSendMessageU(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	if (!plugin->desc)
	{
		Edit_SetTextU(lblPluginMainInfo, "This plugin does not have a valid description field.");
		Edit_SetTextU(lblPluginSecInfo, NULL);
		Edit_SetTextU(lblPluginDesc, NULL);
#ifdef GENS_PNG
		pSendMessageU(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
#endif
		return;
	}
	
	// Fill in the descriptions.
	mdp_desc_t *desc = plugin->desc;
	stringstream ssMainInfo;
	
	// Plugin name.
	ssMainInfo << "Name: " << (desc->name ? desc->name : "(none)") << WIN32_ENDL;
	
	// Plugin version.
	ssMainInfo << "Version: " << MDP_VERSION_MAJOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_MINOR(plugin->pluginVersion)
				  << "." << MDP_VERSION_REVISION(plugin->pluginVersion) << WIN32_ENDL;
	
	// Plugin author.
	ssMainInfo << "MDP Author: " + (desc->author_mdp ? string(desc->author_mdp) : "(none)") << WIN32_ENDL;
	
	// Original code author.
	if (desc->author_orig)
	{
		ssMainInfo << "Original Author: " << desc->author_orig << WIN32_ENDL;
	}
	
	// Website.
	if (desc->website)
	{
		ssMainInfo << "Website: " << desc->website << WIN32_ENDL;
	}
	
	// License.
	ssMainInfo << "License: " + ((desc->license && desc->license[0]) ? string(desc->license) : "(none)");
	
	// Set the main plugin information.
	Edit_SetTextU(lblPluginMainInfo, ssMainInfo.str().c_str());
	
	// UUID.
	string sUUID = UUIDtoString(plugin->uuid);
	
	// Secondary plugin information.
	// Includes UUID and CPU flags.
	stringstream ssSecInfo;
	ssSecInfo << "UUID: " << sUUID << WIN32_ENDL
		  << GetCPUFlags_string(plugin->cpuFlagsRequired, plugin->cpuFlagsSupported, false);
	
	// Set the secondary information label.
	Edit_SetTextU(lblPluginSecInfo, ssSecInfo.str().c_str());
	
	// Plugin description.
	string pluginDesc;
	if (desc->description)
		pluginDesc = string("Description:") + WIN32_ENDL + desc->description;
	
	// Check for an MDP error code.
	mapMdpErrs_t::iterator errIter = mapMdpErrs.find(plugin);
	if (errIter != mapMdpErrs.end())
	{
		// Found an MDP error code.
		if (!pluginDesc.empty())
			pluginDesc += "\n\n";
		
		char err_code[32];
		szprintf(err_code, sizeof(err_code), "0x%08X", -(*errIter).second);
		
		pluginDesc += "MDP error code: " + string(err_code);
	}
	
	Edit_SetTextU(lblPluginDesc, pluginDesc.c_str());
	
#ifdef GENS_PNG
	// Set the plugin icon.
	HBITMAP hbmpIcon = NULL;
	
	if (lvItem.iImage >= 0 && lvItem.iImage < vectPluginIcons.size())
		hbmpIcon = vectPluginIcons[lvItem.iImage];
	
	pSendMessageU(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmpIcon);
#endif
}


#ifdef GENS_PNG
/**
 * pmgr_window_create_plugin_icon_widget(): Create the plugin icon widget and bitmap.
 * @param container Container for the plugin icon widget.
 */
static void WINAPI pmgr_window_create_plugin_icon_widget(HWND container)
{
	// Plugin icon widget.
	const int top = DLU_Y(5+PMGR_FRAME_PLUGIN_LIST_HEIGHT+5);
	imgPluginIcon = pCreateWindowU(WC_STATIC, NULL,
					WS_CHILD | WS_VISIBLE | SS_BITMAP,
					DLU_X(5+5), top+DLU_Y(10),
					32, 32,
					container, NULL, ghInstance, NULL);
	
	// Clear the icon.
	pSendMessageU(imgPluginIcon, STM_SETIMAGE, IMAGE_BITMAP, NULL);
}


#if 0
// NOTE: This function isn't being used right now.
// TODO: Implement transparency for systems that don't support it natively:
// - Windows 9x, NT 4.0
// - Wine
/**
 * pmgr_window_get_bg_color() Get the background color.
 * @return Background color.
 */
static inline unsigned int WINAPI pmgr_window_get_bg_color(void)
{
	// Get the background color.
	unsigned int bgColor = GetSysColor(COLOR_3DFACE);
	
	// Byteswap the lower 24 bits.
	bgColor = ((bgColor & 0xFF000000)) |
		  ((bgColor & 0x00FF0000) >> 16) |
		  ((bgColor & 0x0000FF00)) |
		  ((bgColor & 0x000000FF) << 16);
	
	return bgColor;
}
#endif


/**
 * pmgr_window_create_bitmap_from_png(): Create a bitmap from a PNG image.
 * @param icon Icon data. (PNG format)
 * @param iconLength Length of the icon data.
 * @return Bitmap containing the icon, or NULL on error.
 */
static HBITMAP WINAPI pmgr_window_create_bitmap_from_png(const uint8_t *icon, const unsigned int iconLength)
{
	static const unsigned char pngMagicNumber[8] = {0x89, 'P', 'N', 'G',0x0D, 0x0A, 0x1A, 0x0A};
	
	if (!icon || iconLength < sizeof(pngMagicNumber))
		return NULL;
	
	// Check that the icon is in PNG format.
	if (memcmp(icon, pngMagicNumber, sizeof(pngMagicNumber)))
	{
		// Not in PNG format.
		return NULL;
	}

	// Initialize libpng.
	if (gsft_png_dll_init() != 0)
		return NULL;
	
	png_structp png_ptr = ppng_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		return NULL;
	
	png_infop info_ptr = ppng_create_info_struct(png_ptr);
	if (!info_ptr)
	{
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return NULL;
	}
	
	png_infop end_info = ppng_create_info_struct(png_ptr);
	if (!end_info)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}
	
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		// TODO: Is setjmp() really necessary?
		ppng_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Initialize the custom PNG read function.
	gsft_png_mem_t png_mem = {icon, iconLength, 0};
	ppng_set_read_fn(png_ptr, &png_mem, &gsft_png_user_read_data);
	
	void *read_io_ptr = png_get_io_ptr(png_ptr);
	ppng_set_read_fn(png_ptr, read_io_ptr, &gsft_png_user_read_data);
	
	// Get the PNG information.
	ppng_read_info(png_ptr, info_ptr);
	
	// Get the PNG information.
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type, compression_type, filter_method;
	bool has_alpha = false;
	
	ppng_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
			&interlace_type, &compression_type, &filter_method);
	
	if (width != 32 || height != 32)
	{
		// Not 32x32.
		ppng_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Make sure RGB color is used.
	if (color_type == PNG_COLOR_TYPE_PALETTE)
		ppng_set_palette_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_GRAY)
		ppng_set_gray_to_rgb(png_ptr);
	else if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
		has_alpha = true;
	
	// Win32 expects BGRA format.
	ppng_set_bgr(png_ptr);
	
	// Convert tRNS to alpha channel.
	if (ppng_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	{
		ppng_set_tRNS_to_alpha(png_ptr);
		has_alpha = true;
	}
	
	// Convert 16-bit per channel PNG to 8-bit.
	if (bit_depth == 16)
		ppng_set_strip_16(png_ptr);
	
	// Get the new PNG information.
	ppng_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
			&interlace_type, &compression_type, &filter_method);
	
	// Check if the PNG image has an alpha channel.
	// TODO: Windows XP (and probably 2000 and Vista) support alpha transparency directly.
	// Older versions don't.
	if (!has_alpha)
	{
		// No alpha channel specified.
		// Use filler instead.
		ppng_set_filler(png_ptr, 0x00, PNG_FILLER_AFTER);
	}
	
	// Update the PNG info.
	ppng_read_update_info(png_ptr, info_ptr);
	
	// Create the bitmap.
	// Plugin icon bitmap.
	HDC dc = CreateCompatibleDC(NULL);
	BITMAPINFO bmInfo;
	
	bmInfo.bmiHeader.biSize = sizeof(bmInfo.bmiHeader);
	bmInfo.bmiHeader.biWidth = 32;
	bmInfo.bmiHeader.biHeight = -32;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = 32;
	bmInfo.bmiHeader.biCompression = 0;
	bmInfo.bmiHeader.biSizeImage = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biClrImportant = 0;
	
	// TODO: Destroy hbmpPluginIcon later (if the ImageList doesn't do that automatically).
	void *bmpPluginIconData;
	HBITMAP hbmpPluginIcon = CreateDIBSection(dc, &bmInfo, DIB_RGB_COLORS,
						  &bmpPluginIconData, NULL, 0);
	
	if (!hbmpPluginIcon)
	{
		// Could not allocate a bitmap.
		ppng_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
		return NULL;
	}
	
	// Create the row pointers.
	png_bytep row_pointers[32];
	unsigned char *pixels = static_cast<unsigned char*>(bmpPluginIconData);
	for (unsigned int i = 0; i < 32; i++)
	{
		row_pointers[i] = pixels;
		pixels += 32*4;
	}
	
	// Read the image data.
	ppng_read_image(png_ptr, row_pointers);
	
	// Close the PNG image.
	ppng_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
	
	// Return the bitmap.
	return hbmpPluginIcon;
}
#endif /* GENS_PNG */
