#include "window.h"
#include <string.h>
#include "video.h"

enum {
  COL_NAME = 0,
  COL_VENDOR,
  COL_ACTIVE,
  NUM_COLS
};

void on_device_activate(GtkWidget *widget, gpointer data) {
    g_print("Activate device\n");

    open_video_device(capture_device_instance.selected_device_str);
}

void on_devicelist_changed(GtkWidget *widget, gpointer data) {
    g_print("clicked\n");

    GtkTreeIter iter;
    GtkTreeModel* model;
    char *value;

    if (gtk_tree_selection_get_selected(
        GTK_TREE_SELECTION(widget), &model, &iter)) {
        gtk_tree_model_get(model, &iter, COL_NAME, &value,  -1);
        strncpy(capture_device_instance.selected_device_str, value, 255);
        g_free(value);
    }
}

GtkTreeModel* create_and_fill_model(void) {
    GtkListStore  *store;
    GtkTreeIter    iter;

    store = gtk_list_store_new(NUM_COLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN);

    int num_items = 0;
    video_capture_device_t* capture_device_list;

    capture_device_list = create_capture_list();
    
    if (capture_device_list == NULL) {
        return GTK_TREE_MODEL(store);
    }
    
    num_items = sizeof(*capture_device_list) / sizeof(video_capture_device_t);
    
    int i;
    for (i = 0; i < num_items; i++) {
        /* Append a row and fill in some data */
        gtk_list_store_append (store, &iter);
        gtk_list_store_set(store, &iter,
            COL_NAME, (char*)capture_device_list[i].device_str,
            COL_VENDOR, (char*)capture_device_list[i].card_str,
            COL_ACTIVE, FALSE, -1);
    } 

    free(capture_device_list);
    
    return GTK_TREE_MODEL(store);
}

GtkWidget* create_devicelist_view_and_model(void) {
    GtkCellRenderer* renderer;
    GtkTreeModel*    model;
    GtkWidget*       view;

    view = gtk_tree_view_new();

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                               -1,      
                                               "Device",  
                                               renderer,
                                               "text", COL_NAME,
                                               NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                               -1,      
                                               "Description",  
                                               renderer,
                                               "text", COL_VENDOR,
                                               NULL);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_insert_column_with_attributes(GTK_TREE_VIEW(view),
                                               -1,      
                                               "Active",  
                                               renderer,
                                               "text", COL_ACTIVE,
                                               NULL);
    
    model = create_and_fill_model();
    gtk_tree_view_set_model (GTK_TREE_VIEW (view), model);
    g_object_unref(model);

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
    g_signal_connect(selection, "changed", 
        G_CALLBACK(on_devicelist_changed), NULL);
    
    return view;
}

/* device window */
void show_device_dialog() {
	device_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    /* set window size */
    gtk_widget_set_usize(GTK_WIDGET(device_window), 800, 600);
    gtk_window_set_position(GTK_WINDOW(device_window), GTK_WIN_POS_CENTER);
    gtk_window_set_resizable(GTK_WINDOW(device_window), TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(device_window), 15);

    GtkWidget* table = gtk_table_new(8, 4, FALSE);
    gtk_table_set_col_spacings(GTK_TABLE(table), 3);

    GtkWidget* title = gtk_label_new("Detected camera devices");
    GtkWidget* halign = gtk_alignment_new(0, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(halign), title);
    gtk_table_attach(GTK_TABLE(table), halign, 0, 1, 0, 1, 
      GTK_FILL, GTK_FILL, 0, 0);

    GtkWidget* listview = create_devicelist_view_and_model();
    gtk_table_attach(GTK_TABLE(table), listview, 0, 2, 1, 3,
        GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_EXPAND, 1, 1);

    GtkWidget* halign2 = gtk_alignment_new(0, 1, 0, 0);
    GtkWidget* activate = gtk_button_new_with_label("Activate");
    gtk_container_add(GTK_CONTAINER(halign2), activate);
    gtk_widget_set_size_request(activate, 70, 30);
    gtk_table_set_row_spacing(GTK_TABLE(table), 3, 6);
    gtk_table_attach(GTK_TABLE(table), halign2, 0, 1, 4, 5, 
        GTK_FILL, GTK_FILL, 0, 0);
    g_signal_connect(activate, "clicked", 
        G_CALLBACK(on_device_activate), NULL);
    
    GtkWidget* ok = gtk_button_new_with_label("OK");
    gtk_widget_set_size_request(ok, 70, 30);
    gtk_table_attach(GTK_TABLE(table), ok, 3, 4, 4, 5, 
        GTK_FILL, GTK_FILL, 0, 0);

    
    gtk_container_add(GTK_CONTAINER(device_window), table);    
    gtk_widget_show_all(GTK_WIDGET(device_window));
}
/* menu click handler*/
static void menu_item_onclick(gchar *sender) {
    printf("%s\n", sender);
    
    if (!strcmp(sender, "open.device")) {
        show_device_dialog();
    }
}

gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data) {
    gdk_draw_arc(widget->window,
        widget->style->fg_gc[gtk_widget_get_state(widget)],
        TRUE,
        0, 0, widget->allocation.width, widget->allocation.height,
        0, 64 * 360
    );
    return TRUE;
}


gboolean update_source_stream(GtkWidget *widget) {
    if (widget->window == NULL) 
        return FALSE;

    if (capture_device_instance.active == 0)
        return TRUE;
    
    // read frame from input source
    frameRead();
    
    gtk_widget_queue_draw(widget);
    return TRUE;
}

/* getter */
GtkWidget *get_main_window() {
	return main_window;
}

GtkWidget* get_main_menu_widget(GtkWidget *window) {
    GtkWidget* menu = gtk_menu_new();
    /* create the menu items */
    GtkWidget* open_capture_item = gtk_menu_item_new_with_label("Open");
    /* Add them to the menu */
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), open_capture_item);
    /* Attach the callback functions to the activate signal */
    g_signal_connect_swapped(open_capture_item, "activate", 
        G_CALLBACK(menu_item_onclick), (gpointer) "open.device");
    /* We do need to show menu items */
    gtk_widget_show(open_capture_item);

    GtkWidget* menu_bar = gtk_menu_bar_new();
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
    //gtk_widget_show(main_vbox);
    
    GtkWidget* menubar = get_main_menu_widget(main_window); 
    gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
    //gtk_widget_show(menubar);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 700, 500);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event",
        G_CALLBACK(expose_event_callback), NULL);

    gtk_box_pack_start(GTK_BOX(main_vbox), drawing_area, FALSE, TRUE, 0);

    g_timeout_add(1000, (GSourceFunc)update_source_stream, (gpointer)main_window);
    
    gtk_widget_show_all(main_window);
}