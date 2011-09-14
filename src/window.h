#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>

GtkWidget *main_window;

extern void create_main_window();
extern GtkWidget* get_main_menu_widget(GtkWidget*);

#endif

