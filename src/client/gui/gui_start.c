#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include "mainpage.h"

// Объявляем переменные для всех виджетов


// Глобальная переменная для отслеживания текущей вкладки
// 0 - Login, 1 - Register

void init_gui(t_chat *chat) {
    GtkBuilder *builder;

    // Завантаження графічного інтерфейсу з Glade-файлу
    builder = gtk_builder_new_from_file("src/client/gui/Glade_structures.glade");
    if (!builder) {
        g_printerr("Помилка: не вдалося завантажити файл Glade.\n");
        exit(EXIT_FAILURE);
    }

    // Завантаження всіх основних віджетів
    load_widgets_from_builder(builder);

    // Ініціалізація головного вікна
    init_mainpage(builder);

    // Перевірка наявності діалогового вікна авторизації
    if (!auth_dialog) {
        g_printerr("Помилка: не вдалося знайти auth_dialog у файлі Glade.\n");
        exit(EXIT_FAILURE);
    }

    // Підключення сигналів до авторизації
    g_signal_connect(button_apply, "clicked", G_CALLBACK(on_button_apply_clicked), chat);
    g_signal_connect(notebook_auth, "switch-page", G_CALLBACK(on_notebook_auth_switch_page), NULL);
    g_signal_connect(btn_auth_cancel, "clicked", G_CALLBACK(on_btn_auth_cancel_clicked), NULL);

    // Відображення діалогового вікна авторизації
    gtk_widget_show_all(auth_dialog);
}


char *build_url(const char *host, const char *port,const char* url_link) {
    // Виділяємо пам'ять для результуючого рядка
    size_t url_length = snprintf(NULL, 0, "http://%s:%s/%s", host, port, url_link) + 1;
    
    char *url = malloc(url_length);
    
    if (url == NULL) {
        fprintf(stderr, "Помилка виділення пам'яті\n");
        return NULL;
    }

    // Формуємо рядок
    snprintf(url, url_length, "http://%s:%s/%s", host, port, url_link);
    printf("URL_1: %s", url);
    return url;
}


// Функция для загрузки всех виджетов из GtkBuilder
void load_widgets_from_builder(GtkBuilder *builder) {
    auth_dialog = GTK_WIDGET(gtk_builder_get_object(builder, "auth_dialog"));
    auth_box = GTK_WIDGET(gtk_builder_get_object(builder, "auth_box"));
    button_apply = GTK_WIDGET(gtk_builder_get_object(builder, "btn_auth_confirm"));
    header_auth = GTK_WIDGET(gtk_builder_get_object(builder, "header_auth"));
    notebook_auth = GTK_WIDGET(gtk_builder_get_object(builder, "notebook_auth"));
    reg_entry_log = GTK_WIDGET(gtk_builder_get_object(builder, "rg_entry_log"));
    reg_entry_pass = GTK_WIDGET(gtk_builder_get_object(builder, "rg_entry_pass"));
    label_reg_auth = GTK_WIDGET(gtk_builder_get_object(builder, "label_reg_auth"));
    log_entry_log = GTK_WIDGET(gtk_builder_get_object(builder, "log_entry_log"));
    log_entry_pass = GTK_WIDGET(gtk_builder_get_object(builder, "log_entry_pass"));
    label_log_auth = GTK_WIDGET(gtk_builder_get_object(builder, "label_log_auth"));
    window_main = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    dialog_addroom = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_addroom"));
    btn_auth_cancel = GTK_WIDGET(gtk_builder_get_object(builder, "btn_auth_cancel"));
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
        char *piptik = build_url(host, port, "auth/login");
        printf("\n%s\n", piptik);
        send_login_request(piptik, login, password);
    } else if (current_page == 0) { // Register
        const gchar *login = gtk_entry_get_text(GTK_ENTRY(reg_entry_log));
        const gchar *password = gtk_entry_get_text(GTK_ENTRY(reg_entry_pass));
        char *piptik = build_url(host, port, "auth/register");
        
        g_print("Register: Login: %s, Password: %s\n", login, password);
      
        // Отправить запрос на сервер
        send_register_request(piptik, login, password);

    }
    clear_entry(reg_entry_log);
    clear_entry(reg_entry_pass);
    clear_entry(log_entry_log);
    clear_entry(log_entry_pass);
}

void on_btn_auth_cancel_clicked(GtkButton *button, gpointer user_data) {
    exit(0);
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
