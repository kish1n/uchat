#ifndef CLIENT_H
#define CLIENT_H

#include <gtk/gtk.h>
#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gio/gio.h>
#include <time.h>
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
extern GtkWidget *window_main;
extern GtkWidget *dialog_addroom;
extern GtkWidget *btn_auth_cancel;
extern int current_page;
extern char* host;
extern char* port;
extern char* global_token;
extern int chat_id_global;
extern char* username_global;

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
void send_login_request(const char *url, const char *username, const char *password);
const char *get_global_token();
void set_global_token(const char *token);
void set_pass(const char *pass);
const char *get_pass();
void add_room_to_list(const char *room_name, int id);
void load_css(const gchar *css_path);
void set_chat_id_global(int chat_id);
int get_chat_id_global();
void fetch_and_load_chats(const char *url);
char *extract_other_username(const char *input, const char *my_username);
const char *get_username_global();
char *build_url(const char *host, const char *port,const char* url_link);
void fetch_and_display_messages(int chat_id);
const char *extract_json_from_response_3(const char *response);
gboolean is_scrolled_to_bottom(GtkScrolledWindow *scrolled_window);
void scroll_to_bottom(GtkScrolledWindow *scrolled_window);
void on_btn_auth_cancel_clicked(GtkButton *button, gpointer user_data);
void clear_entry(GtkWidget *entry_widget);
void clear_text_view(GtkWidget *text_view_widget);
void on_profile_exit_btn_clicked(GtkButton *button, gpointer user_data);

#endif
