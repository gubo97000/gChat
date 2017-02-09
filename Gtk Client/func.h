#define _GNU_SOURCE

#include <stdio.h>
#include <stdio_ext.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <ctype.h>
#include <pthread.h>
#include <signal.h>

#include <gtk/gtk.h>

//Dimension
#define S_BUFF 1024
#define S_NICK 11
#define S_HOST 20
#define AL "-"
#define SE "+"
#define RO "#"
#define NI "@"

//Layout widget
GtkWidget *window;
GtkWidget *hbox_entry, *hbox_top_info, *vbox, *scrolledwindow;
GtkWidget *message_entry;
GtkWidget *ok_button;
GtkWidget *view;
GtkWidget *nick, *room;

GdkPixbuf *pixbuf;
GtkTextBuffer *buffer, *alert;
GtkTextMark *mark;
GtkTextTag *tag;
GtkTextIter iter;

//Struct for conf.txt

typedef struct i {
    char host[S_HOST];
    char nick[S_NICK];
} conf_t;

//File conf.txt initialize 
int fconf_init();
//Load conf.txt
int sconf_up(FILE*f_add, conf_t*conf);

//Check if string is empty 
int is_empty(const char *s);
//Check if buf is suitable for server
int genuine(const char*buf);

//Check if a command is empty
int check(char buf[S_BUFF]);

//Visualizer function for a thread 
void*visualizer();

//Log in the server
void login();

//Interface

//Close app when destroy
void closeApp(GtkWidget *window, gpointer data);
//Function for "send"
void button_clicked(GtkWidget *button, gpointer data);
//Create the interface
int interface(int argc, char *argv[]);
//Function for /n
void press_enter(GtkWidget *message_entry, gpointer data);
//Aut scroll
void auto_scroll(GtkWidget *buffer, gpointer data);

static gboolean scroll_to_bottom(GtkTextView *textview, GtkTextIter iter);