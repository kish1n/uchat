#include "client.h"
void init_gui(t_chat *chat);

//STARIJ BOG
GtkWidget *auth_dialog = NULL;
GtkWidget *auth_box = NULL;
GtkWidget *button_cancel = NULL;
GtkWidget *button_apply = NULL;
GtkWidget *header_auth = NULL;
GtkWidget *notebook_auth = NULL;
GtkWidget *reg_entry_log = NULL;
GtkWidget *reg_entry_pass = NULL;
GtkWidget *label_reg_auth = NULL;
GtkWidget *log_entry_log = NULL;
GtkWidget *log_entry_pass = NULL;
GtkWidget *label_log_auth = NULL;
GtkWidget *window_main = NULL;
GtkWidget *dialog_addroom = NULL;
GtkWidget *btn_auth_cancel = NULL;

char* host;
char* port;
// Упрощенная функция для создания подключения
static void create_connection(t_chat *chat, const char *host, const char *port) {
    GError *error = NULL;

    chat->client_conn = g_socket_client_new();
    chat->conn = g_socket_client_connect_to_host(chat->client_conn, host, g_ascii_strtoll(port, NULL, 10), NULL, &error);

    if (!chat->conn) {
        g_printerr("Ошибка подключения: %s\n", error->message);
        g_clear_error(&error);
        exit(EXIT_FAILURE);
    }

    // Инициализация потоков ввода/вывода
    chat->out = g_data_output_stream_new(g_io_stream_get_output_stream(G_IO_STREAM(chat->conn)));
    chat->in = g_data_input_stream_new(g_io_stream_get_input_stream(G_IO_STREAM(chat->conn)));

    g_print("Подключение успешно установлено к %s:%s\n", host, port);
}

void apply_css() {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_css_provider_load_from_path(provider, "style_w.css", NULL);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);


    // Додаємо провайдера стилів
    gtk_style_context_add_provider_for_screen(
        screen,
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    g_object_unref(provider);
}
// Основная функция клиента
int main(int argc, char **argv) {
    if (argc != 3) {
        g_printerr("Using: %s <host> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Инициализация GTK
    gtk_init(&argc, &argv);
    // Создание структуры клиента
    t_chat *chat = g_malloc0(sizeof(t_chat));

    // Установка соединения
    create_connection(chat, argv[1], argv[2]);
    host = argv[1];
    port = argv[2];
    // Инициализация и запуск GUI
    init_gui(chat);
    
    apply_css();
    // Запуск основного цикла GTK
    gtk_main();

    // Освобождение ресурсов
    g_object_unref(chat->builder);
    g_object_unref(chat->out);
    g_object_unref(chat->in);
    g_object_unref(chat->conn);
    g_object_unref(chat->client_conn);
    g_free(chat);

    return EXIT_SUCCESS;
}

void clear_entry(GtkWidget *entry_widget) {
    if (GTK_IS_ENTRY(entry_widget)) {
        gtk_entry_set_text(GTK_ENTRY(entry_widget), "");
    } else {
        g_printerr("Помилка: Віджет не є GtkEntry.\n");
    }
}

void clear_text_view(GtkWidget *text_view_widget) {
    if (GTK_IS_TEXT_VIEW(text_view_widget)) {
        GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_view_widget));
        gtk_text_buffer_set_text(buffer, "", -1);
    } else {
        g_printerr("Помилка: Віджет не є GtkTextView.\n");
    }
}



