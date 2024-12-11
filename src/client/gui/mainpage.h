#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <gtk/gtk.h>

// Оголошення функцій
void init_mainpage(GtkBuilder *builder);
void on_add_room_clicked(GtkButton *button, gpointer user_data);
void on_show_roomlist_clicked(GtkButton *button, gpointer user_data);
void on_send_message_clicked(GtkButton *button, gpointer user_data);
void on_knipka_zakrittya_clicked(GtkButton *button, gpointer user_data);
void on_btn_addroom_apply_clicked(GtkButton *button, gpointer user_data);
void on_notebook_auth_switch_page_1(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
void create_group_chat(const char *url, const char *token, const char *group_name, const char *members);
void on_btn_send_msg_clicked(GtkButton *button, gpointer user_data);
void send_message(const char *url, const char *token, const char *chat_id, const char *content);
void add_message_to_list(const gchar *message, const gchar *time, const gchar *sender);
void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data);
void on_edit_message(GtkMenuItem *menuitem, gpointer user_data);
void on_delete_message(GtkMenuItem *menuitem, gpointer user_data);
GtkWidget *create_context_menu(GtkWidget *message_box);
gboolean on_message_box_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data);
void create_private_chat(const char *url, const char *token, const char *input);
void update_username_request(const char *url, const char *token, const char *new_username, const char *password);




#endif // MAINPAGE_H

