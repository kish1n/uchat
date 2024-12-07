#ifndef CLIENT_H
#define CLIENT_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include <glib.h>
#include "cJSON.h"

extern GtkWidget *auth_dialog;
extern GtkWidget *auth_box;
extern GtkWidget *button_cancel;
extern GtkWidget *button_apply;
extern GtkWidget *header_auth;
extern GtkWidget *notebook_auth;
extern GtkWidget *reg_entry_log;
extern GtkWidget *reg_entry_pass;
extern GtkWidget *label_reg_auth;
extern GtkWidget *log_entry_log;
extern GtkWidget *log_entry_pass;
extern GtkWidget *label_log_auth;
extern int current_page;

typedef struct s_chat {
    GDataOutputStream *out;
    GDataInputStream *in;
    GSocketConnection *conn;
    GSocketClient *client_conn;
    char *auth_token;
    char *login;
    GtkBuilder *builder;
} t_chat;


void init_gui(t_chat *chat);
void send_request(GSocketConnection *conn, const char *action, const char *data);
void load_widgets_from_builder(GtkBuilder *builder);
void on_button_apply_clicked(GtkButton *button, gpointer user_data);
void on_notebook_auth_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
void send_register_request(const char *url, const char *username, const char *password);

#endif
