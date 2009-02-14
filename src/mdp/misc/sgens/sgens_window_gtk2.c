/***************************************************************************
 * Gens: [MDP] Sonic Gens. (Window Code) (GTK+)                            *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * SGens Copyright (c) 2002 by LOst                                        *
 * MDP port Copyright (c) 2008-2009 by David Korth                         *
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

#include <stdio.h>

#include "sgens_window.h"
#include "sgens_plugin.h"
#include "sgens.hpp"

// MDP error codes.
#include "mdp/mdp_error.h"

// GTK includes.
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

// Window.
static GtkWidget *sgens_window = NULL;

// Widgets.
static GtkWidget *lblZone_Info;
static GtkWidget *lblAct_Info;
static GtkWidget *lblScore_Info;
static GtkWidget *lblTime_Info;
static GtkWidget *lblRings_Info;

// Widget creation functions.
static void	sgens_window_create_level_info_frame(GtkWidget *container);

// Callbacks.
static gboolean	sgens_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data);
static void	sgens_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data);


/**
 * sgens_window_show(): Show the VDP Layer Options window.
 * @param parent Parent window.
 */
void sgens_window_show(void *parent)
{
	if (sgens_window)
	{
		// Sonic Gens window is already visible.
		// Set focus.
		gtk_widget_grab_focus(sgens_window);
		return;
	}
	
	// Create the window.
	sgens_window = gtk_dialog_new();
	gtk_widget_set_name(sgens_window, "sgens_window");
	gtk_container_set_border_width(GTK_CONTAINER(sgens_window), 4);
	gtk_window_set_title(GTK_WINDOW(sgens_window), "Sonic Gens");
	gtk_window_set_position(GTK_WINDOW(sgens_window), GTK_WIN_POS_CENTER);
	gtk_window_set_resizable(GTK_WINDOW(sgens_window), FALSE);
	gtk_window_set_type_hint(GTK_WINDOW(sgens_window), GDK_WINDOW_TYPE_HINT_DIALOG);
	gtk_dialog_set_has_separator(GTK_DIALOG(sgens_window), FALSE);
	g_object_set_data(G_OBJECT(sgens_window), "sgens_window", sgens_window);
	
	// Callbacks for if the window is closed.
	g_signal_connect((gpointer)sgens_window, "delete_event",
			 G_CALLBACK(sgens_window_callback_close), NULL);
	g_signal_connect((gpointer)sgens_window, "destroy_event",
			 G_CALLBACK(sgens_window_callback_close), NULL);
	
	// Dialog response callback.
	g_signal_connect((gpointer)sgens_window, "response",
			 G_CALLBACK(sgens_window_callback_response), NULL);
	
	// Get the dialog VBox.
	GtkWidget *vboxDialog = GTK_DIALOG(sgens_window)->vbox;
	gtk_widget_set_name(vboxDialog, "vboxDialog");
	gtk_widget_show(vboxDialog);
	g_object_set_data_full(G_OBJECT(sgens_window), "vboxDialog",
			       g_object_ref(vboxDialog), (GDestroyNotify)g_object_unref);
	
	// Create the "Level Information" frame.
	sgens_window_create_level_info_frame(vboxDialog);
	
	// Create the dialog buttons.
	gtk_dialog_add_buttons(GTK_DIALOG(sgens_window),
			       "gtk-close", GTK_RESPONSE_CLOSE,
			       NULL);
	
	// Set the window as modal to the main application window.
	if (parent)
		gtk_window_set_transient_for(GTK_WINDOW(sgens_window), GTK_WINDOW(parent));
	
	// Show the window.
	gtk_widget_show_all(sgens_window);
	
	// Register the window with MDP Host Services.
	sgens_host_srv->window_register(&mdp, sgens_window);
}


/**
 * sgens_window_create_level_info_frame(): Create the "Level Information" frame.
 * @param container Container for the frame.
 */
static void sgens_window_create_level_info_frame(GtkWidget *container)
{
	// "Level Information" frame.
	GtkWidget *fraLevelInfo = gtk_frame_new("Level Information");
	gtk_widget_set_name(fraLevelInfo, "fraLevelInfo");
	gtk_widget_show(fraLevelInfo);
	gtk_box_pack_start(GTK_BOX(container), fraLevelInfo, TRUE, TRUE, 0);
	gtk_frame_set_shadow_type(GTK_FRAME(fraLevelInfo), GTK_SHADOW_ETCHED_IN);
	g_object_set_data_full(G_OBJECT(container), "fraLevelInfo",
			       g_object_ref(fraLevelInfo), (GDestroyNotify)g_object_unref);
	
	// Create a VBox for the frame.
	GtkWidget *vboxLevelInfo = gtk_vbox_new(FALSE, 4);
	gtk_widget_set_name(vboxLevelInfo, "vboxLevelInfo");
	gtk_container_set_border_width(GTK_CONTAINER(vboxLevelInfo), 0);
	gtk_widget_show(vboxLevelInfo);
	gtk_container_add(GTK_CONTAINER(fraLevelInfo), vboxLevelInfo);
	g_object_set_data_full(G_OBJECT(container), "vboxLevelInfo",
			       g_object_ref(vboxLevelInfo), (GDestroyNotify)g_object_unref);
	
	// "Zone" information label.
	lblZone_Info = gtk_label_new("Zone");
	gtk_widget_set_name(lblZone_Info, "lblZone_Info");
	gtk_misc_set_alignment(GTK_MISC(lblZone_Info), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblZone_Info), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblZone_Info);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblZone_Info, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "lblZone_Info",
			       g_object_ref(lblZone_Info), (GDestroyNotify)g_object_unref);
	
	// "Act" information label.
	lblAct_Info = gtk_label_new("Act");
	gtk_widget_set_name(lblAct_Info, "lblAct_Info");
	gtk_misc_set_alignment(GTK_MISC(lblAct_Info), 0.5f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblAct_Info), GTK_JUSTIFY_CENTER);
	gtk_widget_show(lblAct_Info);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), lblAct_Info, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "lblAct_Info",
			       g_object_ref(lblAct_Info), (GDestroyNotify)g_object_unref);
	
	// Table for various information.
	GtkWidget *tblLevelInfo = gtk_table_new(5, 4, FALSE);
	gtk_widget_set_name(tblLevelInfo, "tblLevelInfo");
	gtk_container_set_border_width(GTK_CONTAINER(tblLevelInfo), 8);
	gtk_table_set_col_spacings(GTK_TABLE(tblLevelInfo), 16);
	gtk_table_set_col_spacing(GTK_TABLE(tblLevelInfo), 3, 8);
	gtk_widget_show(tblLevelInfo);
	gtk_box_pack_start(GTK_BOX(vboxLevelInfo), tblLevelInfo, TRUE, TRUE, 0);
	g_object_set_data_full(G_OBJECT(container), "tblLevelInfo",
			       g_object_ref(tblLevelInfo), (GDestroyNotify)g_object_unref);
	
	// "Score" label.
	GtkWidget *lblScore = gtk_label_new("Score:");
	gtk_widget_set_name(lblScore, "lblScore");
	gtk_misc_set_alignment(GTK_MISC(lblScore), 0.0f, 0.5f);
	gtk_widget_show(lblScore);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblScore,
			 0, 1, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblScore",
			       g_object_ref(lblScore), (GDestroyNotify)g_object_unref);
	
	// "Score" information label.
	lblScore_Info = gtk_label_new("0");
	gtk_widget_set_name(lblScore_Info, "lblScore_Info");
	gtk_misc_set_alignment(GTK_MISC(lblScore_Info), 1.0f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblScore_Info), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(lblScore_Info);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblScore_Info,
			 1, 2, 0, 1,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblScore_Info",
			       g_object_ref(lblScore_Info), (GDestroyNotify)g_object_unref);
	
	// "Time" label.
	GtkWidget *lblTime = gtk_label_new("Time:");
	gtk_widget_set_name(lblTime, "lblTime");
	gtk_misc_set_alignment(GTK_MISC(lblTime), 0.0f, 0.5f);
	gtk_widget_show(lblTime);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblTime,
			 0, 1, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblTime",
			       g_object_ref(lblTime), (GDestroyNotify)g_object_unref);
	
	// "Time" information label.
	lblTime_Info = gtk_label_new("00:00:00");
	gtk_widget_set_name(lblTime_Info, "lblTime_Info");
	gtk_misc_set_alignment(GTK_MISC(lblTime_Info), 1.0f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblTime_Info), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(lblTime_Info);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblTime_Info,
			 1, 2, 1, 2,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblTime_Info",
			       g_object_ref(lblTime_Info), (GDestroyNotify)g_object_unref);
	
	// "Rings" label.
	GtkWidget *lblRings = gtk_label_new("Rings:");
	gtk_widget_set_name(lblRings, "lblRings");
	gtk_misc_set_alignment(GTK_MISC(lblRings), 0.0f, 0.5f);
	gtk_widget_show(lblRings);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblRings,
			 0, 1, 2, 3,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblRings",
			       g_object_ref(lblRings), (GDestroyNotify)g_object_unref);
	
	// "Rings" information label.
	lblRings_Info = gtk_label_new("0");
	gtk_widget_set_name(lblRings_Info, "lblRings_Info");
	gtk_misc_set_alignment(GTK_MISC(lblRings_Info), 1.0f, 0.5f);
	gtk_label_set_justify(GTK_LABEL(lblRings_Info), GTK_JUSTIFY_RIGHT);
	gtk_widget_show(lblRings_Info);
	gtk_table_attach(GTK_TABLE(tblLevelInfo), lblRings_Info,
			 1, 2, 2, 3,
			 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
			 (GtkAttachOptions)(GTK_FILL), 0, 0);
	g_object_set_data_full(G_OBJECT(container), "lblRings_Info",
			       g_object_ref(lblRings_Info), (GDestroyNotify)g_object_unref);
}

/**
 * sgens_window_close(): Close the VDP Layer Options window.
 */
void sgens_window_close(void)
{
	if (!sgens_window)
		return;
	
	// Unregister the window from MDP Host Services.
	sgens_host_srv->window_unregister(&mdp, sgens_window);
	
	// Destroy the window.
	gtk_widget_destroy(sgens_window);
	sgens_window = NULL;
}


/**
 * sgens_window_callback_close(): Close Window callback.
 * @param widget
 * @param event
 * @param user_data
 * @return FALSE to continue processing events; TRUE to stop processing events.
 */
static gboolean sgens_window_callback_close(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(widget);
	MDP_UNUSED_PARAMETER(event);
	MDP_UNUSED_PARAMETER(user_data);
	
	sgens_window_close();
	return FALSE;
}


/**
 * sgens_window_callback_response(): Dialog Response callback.
 * @param dialog
 * @param response_id
 * @param user_data
 */
static void sgens_window_callback_response(GtkDialog *dialog, gint response_id, gpointer user_data)
{
	MDP_UNUSED_PARAMETER(dialog);
	MDP_UNUSED_PARAMETER(user_data);
	
	int rval;
	
	switch (response_id)
	{
		case GTK_RESPONSE_CLOSE:
			// Close.
			sgens_window_close();
			break;
		
		case GTK_RESPONSE_DELETE_EVENT:
		default:
			// Other response ID. Don't do anything.
			// Also, don't do anything when the dialog is deleted.
			break;
	}
}
