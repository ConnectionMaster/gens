/***************************************************************************
 * Gens: (GTK+) Zip File Selection Dialog.                                 *
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

#include "zip_select_dialog.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>

// GENS GTK+ miscellaneous functions
#include "gtk-misc.h"

//GtkAccelGroup *accel_group;


/**
 * create_zip_select_dialog(): Create the Zip File Selection Dialog.
 * @return Zip File Selection Dialog.
 */
GtkWidget* create_zip_select_dialog(void)
{
	GdkPixbuf *zip_select_dialog_icon_pixbuf;
	GtkWidget *zip_select_dialog;
	GtkWidget *frame_zip, *label_frame_zip;
	GtkWidget *treeview_zip_list;
	GtkWidget *button_Zip_Cancel, *button_Zip_OK;
	
//	accel_group = gtk_accel_group_new();
	
	// Create the Zip File Selection window.
	zip_select_dialog = gtk_dialog_new();
	gtk_widget_set_name(zip_select_dialog, "zip_select_dialog");
	gtk_container_set_border_width(GTK_CONTAINER(zip_select_dialog), 5);
	gtk_window_set_title(GTK_WINDOW(zip_select_dialog), "Archive File Selection");
	gtk_window_set_position(GTK_WINDOW(zip_select_dialog), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(zip_select_dialog), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(zip_select_dialog), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(zip_select_dialog), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(zip_select_dialog), FALSE);
	GLADE_HOOKUP_OBJECT_NO_REF(zip_select_dialog, zip_select_dialog, "zip_select_dialog");
	
	// Load the Gens icon.
	zip_select_dialog_icon_pixbuf = create_pixbuf("Gens2.ico");
	if (zip_select_dialog_icon_pixbuf)
	{
		gtk_window_set_icon(GTK_WINDOW(zip_select_dialog), zip_select_dialog_icon_pixbuf);
		gdk_pixbuf_unref(zip_select_dialog_icon_pixbuf);
	}
	
	// Add a frame for zip code selection.
	frame_zip = gtk_frame_new(NULL);
	gtk_widget_set_name(frame_zip, "frame_zip");
	gtk_container_set_border_width(GTK_CONTAINER(frame_zip), 5);
	gtk_frame_set_shadow_type(GTK_FRAME(frame_zip), GTK_SHADOW_NONE);
	gtk_widget_show(frame_zip);
	gtk_container_add(GTK_CONTAINER(GTK_DIALOG(zip_select_dialog)->vbox), frame_zip);
	GLADE_HOOKUP_OBJECT(zip_select_dialog, frame_zip, "frame_zip");
	
	// Add a label to the zip code selection frame.
	label_frame_zip = gtk_label_new("This archive contains multiple files.\nSelect which file you want to load.");
	gtk_widget_set_name(label_frame_zip, "label_frame_zip");
	gtk_label_set_use_markup(GTK_LABEL(label_frame_zip), TRUE);
	gtk_widget_show(label_frame_zip);
	gtk_frame_set_label_widget(GTK_FRAME(frame_zip), label_frame_zip);
	GLADE_HOOKUP_OBJECT(zip_select_dialog, label_frame_zip, "label_frame_zip");
	
	// Tree view containing the files in the archive.
	treeview_zip_list = gtk_tree_view_new();
	gtk_widget_set_name(treeview_zip_list, "treeview_zip_list");
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW(treeview_zip_list), TRUE);
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview_zip_list), FALSE);
	gtk_widget_show(treeview_zip_list);
	gtk_container_add(GTK_CONTAINER(frame_zip), treeview_zip_list);
	GLADE_HOOKUP_OBJECT(zip_select_dialog, treeview_zip_list, "treeview_zip_list");
	
	// Dialog buttons
	
	// Cancel
	button_Zip_Cancel = gtk_dialog_add_button(GTK_DIALOG(zip_select_dialog),
						  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
	GLADE_HOOKUP_OBJECT(zip_select_dialog, button_Zip_Cancel, "button_Zip_Cancel");
	
	// OK
	button_Zip_OK = gtk_dialog_add_button(GTK_DIALOG(zip_select_dialog),
						  GTK_STOCK_OK, GTK_RESPONSE_OK);
	GLADE_HOOKUP_OBJECT(zip_select_dialog, button_Zip_Cancel, "button_Zip_Cancel");
	
	// Add the accel group.
//	gtk_window_add_accel_group(GTK_WINDOW(zip_select_dialog), accel_group);
	
	return zip_select_dialog;
}
