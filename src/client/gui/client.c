#include "client.h"

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

// Упрощенная функция запуска GUI
void init_gui(t_chat *chat) {
    GtkBuilder *builder;

    // Загружаем интерфейс из файла Glade
    builder = gtk_builder_new_from_file("Glade_structures.glade");
    if (!builder) {
        g_printerr("Ошибка: не удалось загрузить файл интерфейса.\n");
        exit(EXIT_FAILURE);
    }

    // Загружаем все виджеты
    load_widgets_from_builder(builder);

    // Проверяем наличие основного окна (auth_dialog)
    if (!auth_dialog) {
        g_printerr("Ошибка: не найдено главное окно (auth_dialog) в файле Glade.\n");
        exit(EXIT_FAILURE);
    }

    // Подключаем сигналы
    g_signal_connect(button_apply, "clicked", G_CALLBACK(on_button_apply_clicked), chat);
    g_signal_connect(notebook_auth, "switch-page", G_CALLBACK(on_notebook_auth_switch_page), NULL);

    // Отображаем окно
    gtk_widget_show_all(auth_dialog);
}

// Основная функция клиента
int main(int argc, char **argv) {
    if (argc != 3) {
        g_printerr("Использование: %s <host> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Инициализация GTK
    gtk_init(&argc, &argv);

    // Создание структуры клиента
    t_chat *chat = g_malloc0(sizeof(t_chat));

    // Установка соединения
    create_connection(chat, argv[1], argv[2]);

    // Инициализация и запуск GUI
    init_gui(chat);

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


