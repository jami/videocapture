#include "window.h"

/* menu click handler*/
static void menu_item_onclick(gchar *string)
{
    printf("%s\n", string);
}
/* getter */
GtkWidget *get_main_window() {
	return main_window;
}

GtkWidget* get_main_menu_widget(GtkWidget *window) {
    GtkWidget* menu = gtk_menu_new();
    /* create the menu items */
    GtkWidget* open_capture_item = gtk_menu_item_new_with_label ("Open");
    /* Add them to the menu */
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_capture_item);
    /* Attach the callback functions to the activate signal */
    g_signal_connect_swapped(open_capture_item, "activate", 
        G_CALLBACK(menu_item_onclick), (gpointer) "open.device");
    /* We do need to show menu items */
    gtk_widget_show(open_capture_item);


    GtkWidget* menu_bar = gtk_menu_bar_new();
    gtk_container_add(GTK_CONTAINER(window), menu_bar);
    gtk_widget_show(menu_bar);

    GtkWidget* file_item = gtk_menu_item_new_with_label("Device");
    gtk_widget_show(file_item);

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), menu);
    gtk_menu_bar_append(GTK_MENU_BAR(menu_bar), file_item);

    return menu_bar;
}
/* close the application */
static void on_destroy(GtkWidget* w, gpointer data) {
	gtk_main_quit();
}
/* create the main window */ 
void create_main_window() {
	main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* connect the destroy handler */
    g_signal_connect(G_OBJECT(main_window), "destroy", G_CALLBACK(on_destroy), NULL);
    /* set window size */
    gtk_widget_set_usize(GTK_WIDGET(main_window), 800, 600);
    
    GtkWidget* main_vbox = gtk_vbox_new(FALSE, 1);
    gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
    gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
    gtk_widget_show(main_vbox);
    
    GtkWidget* menubar = get_main_menu_widget(main_window); 
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    gtk_widget_show(menubar);
    
    gtk_widget_show(main_window);
}
