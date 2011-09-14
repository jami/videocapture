#ifndef WINDOW_H
#define WINDOW_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <cairo.h>

static GtkWidget *main_window;
static GtkWidget *device_window;

extern void create_main_window();
extern GtkWidget* get_main_menu_widget(GtkWidget*);

#endif

