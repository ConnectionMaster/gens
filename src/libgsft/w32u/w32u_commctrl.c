/***************************************************************************
 * libgsft_w32u: Win32 Unicode Translation Layer.                          *
 * w32u_commctrl.c: commctrl.h translation.                                *
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

#include "w32u.h"
#include "w32u_priv.h"
#include "w32u_windows.h"
#include "w32u_commctrl.h"

// C includes.
#include <stdlib.h>


int WINAPI TabCtrl_InsertItemU(HWND hWnd, int iItem, const LPTCITEM pItem)
{
	if (!w32u_is_unicode)
		return pSendMessageU(hWnd, TCM_INSERTITEMA, iItem, (LPARAM)pItem);
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	TCITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = NULL;
	
	if (wItem.pszText)
	{
		pszwText = w32u_UTF8toUTF16(pItem->pszText);
		wItem.pszText = pszwText;
	}
	
	LRESULT lRet = pSendMessageU(hWnd, TCM_INSERTITEMW, iItem, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


/** ListView functions. **/


int WINAPI ListView_GetItemU(HWND hWnd, LVITEM *pItem)
{
	if (!w32u_is_unicode)
		return pSendMessageU(hWnd, LVM_GETITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText || pItem->cchTextMax <= 0)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_GETITEMW, 0, (LPARAM)pItem);
	}
	
	// Copy the LVITEM temporarily.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	// Allocate a buffer for the UTF-16 text.
	wchar_t *wbuf = (wchar_t*)malloc((wItem.cchTextMax + 1) * sizeof(wchar_t));
	wItem.pszText = wbuf;
	
	// Get the LVITEM.
	LRESULT lRet = pSendMessageU(hWnd, LVM_GETITEMW, 0, (LPARAM)&wItem);
	if (!lRet)
	{
		// Error retrieving the item.
		free(wbuf);
		return lRet;
	}
	
	// Convert the text to UTF-8.
	// TODO Verify characters vs. bytes.
	char *utf8_buf = pItem->pszText;
	WideCharToMultiByte(CP_UTF8, 0, wbuf, -1, utf8_buf, pItem->cchTextMax, NULL, NULL);
	
	// Copy the LVITEM struct back to pItem.
	memcpy(pItem, &wItem, sizeof(*pItem));
	pItem->pszText = utf8_buf;
	
	free(wbuf);
	return lRet;
}


int WINAPI ListView_InsertColumnU(HWND hWnd, int iCol, const LV_COLUMN *pCol)
{
	if (!w32u_is_unicode)
		return pSendMessageU(hWnd, LVM_INSERTCOLUMNA, iCol, (LPARAM)pCol);
	
	// Convert pCol->pszText from UTF-8 to UTF-16.
	TCITEMW wCol;
	memcpy(&wCol, pCol, sizeof(wCol));
	
	wchar_t *pszwText = NULL;
	
	if (wCol.pszText)
	{
		pszwText = w32u_UTF8toUTF16(pCol->pszText);
		wCol.pszText = pszwText;
	}
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_INSERTCOLUMNW, iCol, (LPARAM)&wCol);
	free(pszwText);
	return lRet;
}


int WINAPI ListView_InsertItemU(HWND hWnd, const LVITEM *pItem)
{
	if (!w32u_is_unicode)
		return pSendMessageU(hWnd, LVM_INSERTITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_INSERTITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = w32u_UTF8toUTF16(pItem->pszText);
	wItem.pszText = pszwText;
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_INSERTITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}


int WINAPI ListView_SetItemU(HWND hWnd, const LVITEM *pItem)
{
	if (!w32u_is_unicode)
		return pSendMessageU(hWnd, LVM_SETITEMA, 0, (LPARAM)pItem);
	
	if (!(pItem->mask & LVIF_TEXT) || !pItem->pszText)
	{
		// No text. Send the message as-is.
		return pSendMessageU(hWnd, LVM_SETITEMW, 0, (LPARAM)pItem);
	}
	
	// Convert pItem->pszText from UTF-8 to UTF-16.
	LVITEMW wItem;
	memcpy(&wItem, pItem, sizeof(wItem));
	
	wchar_t *pszwText = w32u_UTF8toUTF16(pItem->pszText);
	wItem.pszText = pszwText;
	
	LRESULT lRet = pSendMessageU(hWnd, LVM_SETITEMW, 0, (LPARAM)&wItem);
	free(pszwText);
	return lRet;
}
