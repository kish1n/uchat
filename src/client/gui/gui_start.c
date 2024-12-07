#include "client.h"

// Объявляем переменные для всех виджетов


// Глобальная переменная для отслеживания текущей вкладки
// 0 - Login, 1 - Register

// Функция для загрузки всех виджетов из GtkBuilder
void load_widgets_from_builder(GtkBuilder *builder) {
    auth_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "auth_dialog"));
    auth_box = GTK_WIDGET(gtk_builder_get_object(builder, "auth_box"));
    button_cancel = GTK_WIDGET(gtk_builder_get_object(builder, "button_cancel"));
    button_apply = GTK_WIDGET(gtk_builder_get_object(builder, "button_apply"));
    header_auth = GTK_WIDGET(gtk_builder_get_object(builder, "header_auth"));
    notebook_auth = GTK_WIDGET(gtk_builder_get_object(builder, "notebook_auth"));
    reg_entry_log = GTK_WIDGET(gtk_builder_get_object(builder, "reg_entry_log"));
    reg_entry_pass = GTK_WIDGET(gtk_builder_get_object(builder, "reg_entry_pass"));
    label_reg_auth = GTK_WIDGET(gtk_builder_get_object(builder, "label_reg_auth"));
    log_entry_log = GTK_WIDGET(gtk_builder_get_object(builder, "log_entry_log"));
    log_entry_pass = GTK_WIDGET(gtk_builder_get_object(builder, "log_entry_pass"));
    label_log_auth = GTK_WIDGET(gtk_builder_get_object(builder, "label_log_auth"));
}
 int current_page = 0;
// Обработчик для кнопки "Apply"
void on_button_apply_clicked(GtkButton *button, gpointer user_data) {
    t_chat *chat = (t_chat *)user_data;
    if (current_page == 1) { // Login
        const gchar *login = gtk_entry_get_text(GTK_ENTRY(log_entry_log));
        const gchar *password = gtk_entry_get_text(GTK_ENTRY(log_entry_pass));
        g_print("Login: %s, Password: %s\n", login, password);

        // Отправить запрос на сервер
        send_request(chat->conn, "LOGIN", login);
    } else if (current_page == 0) { // Register
        const gchar *login = gtk_entry_get_text(GTK_ENTRY(reg_entry_log));
        const gchar *password = gtk_entry_get_text(GTK_ENTRY(reg_entry_pass));
        g_print("Register: Login: %s, Password: %s\n", login, password);

        // Отправить запрос на сервер
        send_register_request("http://localhost:8080/register", login, password);
    }
}

// Обработчик для переключения вкладок в GtkNotebook
void on_notebook_auth_switch_page(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    current_page = page_num; // Обновляем текущую страницу
    g_print("Switched to page: %d\n", page_num);
}


void send_request(GSocketConnection *conn, const char *action, const char *data) {
    GOutputStream *output_stream = g_io_stream_get_output_stream(G_IO_STREAM(conn));
    gchar *message = g_strdup_printf("%s: %s\n", action, data);

    g_output_stream_write(output_stream, message, strlen(message), NULL, NULL);
    g_output_stream_flush(output_stream, NULL, NULL);

    g_free(message);
}



