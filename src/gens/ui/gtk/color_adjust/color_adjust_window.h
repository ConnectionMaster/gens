/**
 * GENS: (GTK+) Color Adjust Window.
 */

#ifndef COLOR_ADJUST_WINDOW_H
#define COLOR_ADJUST_WINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include "ui-common.h"

GtkWidget* create_color_adjust_window(void); 
extern GtkWidget *color_adjust_window;

#ifdef __cplusplus
}
#endif

#endif
