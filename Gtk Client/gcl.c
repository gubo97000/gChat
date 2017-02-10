#include "func.h"
#include <stdio.h>
#include <stdlib.h> /* for exit() */

void closeApp(GtkWidget *window, gpointer data) {
    gtk_main_quit();
}

int interface(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    //Initialize widget
    message_entry = gtk_entry_new();
    ok_button = gtk_button_new_with_label("Send");
    view = gtk_text_view_new();
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(view));
    nick = gtk_label_new("Nick:");
    room = gtk_label_new("Room:");
    pixbuf = gdk_pixbuf_new_from_file("icon.ico", NULL);

    scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
    hbox_entry = gtk_hbox_new(FALSE, 10);
    hbox_top_info = gtk_hbox_new(FALSE, 10);
    vbox = gtk_vbox_new(FALSE, 10);

    //Window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gtk gClient");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_window_set_icon(GTK_WINDOW(window), pixbuf);

    //Set iter
    gtk_text_buffer_get_iter_at_offset(buffer, &iter, 0);
    //Set Mark
    mark = gtk_text_buffer_create_mark(buffer, "scroll", &iter, TRUE);
    //Set tag
    gtk_text_buffer_create_tag(buffer, "alert", "foreground", "red", NULL);
    gtk_text_buffer_create_tag(buffer, "service", "foreground", "green", NULL);
    //Setting for TextView
    gtk_text_view_set_editable((GtkTextView*) view, FALSE);
    gtk_text_view_set_cursor_visible((GtkTextView*) view, FALSE);

    //Layout
    gtk_box_pack_start(GTK_BOX(hbox_top_info), nick, FALSE, FALSE, 0.5);
    gtk_box_pack_start(GTK_BOX(hbox_top_info), room, FALSE, FALSE, 0.5);

    gtk_container_add(GTK_CONTAINER(scrolledwindow), view);

    gtk_box_pack_start(GTK_BOX(hbox_entry), message_entry, TRUE, TRUE, 8);
    gtk_box_pack_start(GTK_BOX(hbox_entry), ok_button, FALSE, FALSE, 1);

    gtk_box_pack_start(GTK_BOX(vbox), hbox_top_info, FALSE, TRUE, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), scrolledwindow, TRUE, TRUE, 8);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_entry, FALSE, FALSE, 1);

    gtk_container_add(GTK_CONTAINER(window), vbox);

    //Connect the function
    //Close window
    g_signal_connect(GTK_OBJECT(window), "destroy",
            GTK_SIGNAL_FUNC(closeApp), NULL);
    //Button click
    g_signal_connect(GTK_OBJECT(ok_button), "clicked",
            GTK_SIGNAL_FUNC(button_clicked), message_entry);
    //Enter on text entry
    g_signal_connect(GTK_OBJECT(message_entry), "activate",
            GTK_SIGNAL_FUNC(press_enter), message_entry);
    //autoscroll
    //g_signal_connect(buffer, "changed",GTK_SIGNAL_FUNC(auto_scroll), NULL);

    //Show
    gtk_widget_show_all(window);
    return 0;
}

void auto_scroll(GtkWidget *buffer, gpointer data) {
    //gtk_text_iter_set_line_offset(&iter, 0);

    gtk_text_buffer_move_mark((GtkTextBuffer*) buffer, mark, &iter);
    gtk_text_view_scroll_mark_onscreen((GtkTextView*) view, mark);
}

