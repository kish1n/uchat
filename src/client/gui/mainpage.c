#include "mainpage.h"
#include "client.h"

extern char* global_token;
// Реалізація функцій
GtkWidget *btn_add_room = NULL;
GtkWidget *btn_show_roomlist = NULL;
GtkWidget *listbox_global_rooms = NULL;
GtkWidget *msg_entry = NULL;
GtkWidget *btn_send_msg = NULL;
GtkWidget *box_main = NULL;
GtkWidget *knipka_zakrittya = NULL;
GtkWidget *entry_room_name_1 = NULL;
GtkWidget *btn_addroom_apply = NULL;
GtkWidget *group_name = NULL;
GtkWidget *group_members = NULL;
GtkWidget *notebook_auth_1 = NULL;
GtkWidget *listbox = NULL;
GtkListBox *listbox_msg = NULL;
GtkWidget *btn_profile_sett = NULL;
GtkWidget *label_header_roomname = NULL;
GtkWidget *profile_exit_btn = NULL;

gboolean auto_update_messages(gpointer chat_id_ptr) {
    int chat_id = GPOINTER_TO_INT(chat_id_ptr);
    g_print("Автоматичне оновлення для чату ID: %d\n", chat_id);

    fetch_and_display_messages(chat_id); // Оновлюємо повідомлення
    return TRUE;
}
void clear_listbox(GtkWidget *listbox) {
    if (!listbox) return;

    GList *children = gtk_container_get_children(GTK_CONTAINER(listbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

static guint auto_update_timer = 0;

void start_auto_update(int chat_id, guint interval_ms) {
    // Зупиняємо попередній таймер
    if (auto_update_timer > 0) {
        g_source_remove(auto_update_timer);
    }

    // Запускаємо новий таймер
    auto_update_timer = g_timeout_add(interval_ms, auto_update_messages, GINT_TO_POINTER(chat_id));
}



// Функція для створення контекстного меню
GtkWidget *create_context_menu(GtkWidget *message_box) {
    GtkWidget *menu = gtk_menu_new();

    // Опція "Редагувати"
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Редагувати");
    g_signal_connect(edit_item, "activate", G_CALLBACK(on_edit_message), message_box);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), edit_item);

    // Опція "Видалити"
    GtkWidget *delete_item = gtk_menu_item_new_with_label("Видалити");
    g_signal_connect(delete_item, "activate", G_CALLBACK(on_delete_message), message_box);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_item);

    gtk_widget_show_all(menu);
    return menu;
}

// Обробник правої кнопки миші
gboolean on_message_box_button_press(GtkWidget *widget, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) { // Права кнопка миші
        GtkWidget *menu = create_context_menu(widget);
        gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent *)event);
        g_print("ПКМ спрацювала на віджет: %p\n", widget);
        return TRUE; // Подія оброблена
    }
    return FALSE; // Продовжити обробку
}

// Обробник "Редагувати"
void on_edit_message(GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget *message_box = GTK_WIDGET(user_data);
    g_print("Редагувати повідомлення: %p\n", message_box);

    // Додайте тут логіку для відкриття вікна редагування
}

// Обробник "Видалити"
void on_delete_message(GtkMenuItem *menuitem, gpointer user_data) {
    GtkWidget *message_box = GTK_WIDGET(user_data);
    g_print("Видалити повідомлення: %p\n", message_box);

    // Видаляємо повідомлення
    gtk_widget_destroy(message_box);
}

// Автоматичне оновлення

void add_message_to_list(const gchar *message, const gchar *time, const gchar *sender) {

    // Контейнер для повідомлення
    GtkWidget *message_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

    // Перевіряємо, чи це повідомлення від поточного користувача
    gboolean is_own_message = g_strcmp0(sender, get_username_global()) == 0;

    // Вирівнюємо контейнер залежно від відправника
    gtk_widget_set_halign(message_box, is_own_message ? GTK_ALIGN_END : GTK_ALIGN_START);

    // Додаємо лейбл з ім'ям відправника
    GtkWidget *label_sender = gtk_label_new(sender);
    gtk_widget_set_name(label_sender, "message_sender"); // Ім'я для стилізації
    gtk_label_set_xalign(GTK_LABEL(label_sender), is_own_message ? 1.0 : 0.0); // Вирівнювання тексту
    gtk_widget_set_margin_start(label_sender, 10);
    gtk_widget_set_margin_end(label_sender, 10);
    gtk_widget_set_margin_top(label_sender, 5);
    gtk_widget_set_margin_bottom(label_sender, 2);

    // Додаємо лейбл відправника до контейнера повідомлення
    gtk_box_pack_start(GTK_BOX(message_box), label_sender, FALSE, FALSE, 0);

    // Обгортка для "бабла" повідомлення
    GtkWidget *bubble = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_name(bubble, "message_bubble");
    gtk_widget_set_margin_top(bubble, 5);
    gtk_widget_set_margin_bottom(bubble, 2);
    gtk_widget_set_margin_start(bubble, is_own_message ? 50 : 10);
    gtk_widget_set_margin_end(bubble, is_own_message ? 10 : 50);

    g_signal_connect(message_box, "button-press-event", G_CALLBACK(on_message_box_button_press), NULL);

    // Текст повідомлення
    GtkWidget *label_message = gtk_label_new(message);
    gtk_label_set_xalign(GTK_LABEL(label_message), 0.0);
    gtk_label_set_line_wrap(GTK_LABEL(label_message), TRUE);
    gtk_label_set_line_wrap_mode(GTK_LABEL(label_message), PANGO_WRAP_WORD_CHAR);
    gtk_label_set_max_width_chars(GTK_LABEL(label_message), 50);

    gtk_widget_set_margin_start(label_message, 10);
    gtk_widget_set_margin_end(label_message, 10);
    gtk_widget_set_margin_top(label_message, 5);
    gtk_widget_set_margin_bottom(label_message, 5);

    // Додаємо текст у "бабл"
    gtk_box_pack_start(GTK_BOX(bubble), label_message, TRUE, TRUE, 0);

    // Додаємо "бабл" у контейнер повідомлення
    gtk_box_pack_start(GTK_BOX(message_box), bubble, FALSE, FALSE, 0);

    // Додаємо часовий лейбл
    GtkWidget *label_time = gtk_label_new(time);
    gtk_widget_set_halign(label_time, is_own_message ? GTK_ALIGN_END : GTK_ALIGN_START);
    gtk_widget_set_margin_end(label_time, 15);
    gtk_widget_set_margin_bottom(label_time, 5);
    gtk_widget_set_name(label_time, "message_time");

    // Додаємо лейбл часу до контейнера повідомлення
    gtk_box_pack_start(GTK_BOX(message_box), label_time, FALSE, FALSE, 0);

    // Включаємо події для ПКМ
    gtk_widget_set_events(message_box, GDK_BUTTON_PRESS_MASK);

    // Додаємо контейнер у список
    gtk_list_box_insert(GTK_LIST_BOX(listbox_msg), message_box, -1);

    // Оновлюємо інтерфейс
    gtk_widget_show_all(GTK_WIDGET(listbox_msg));
}

void load_css(const gchar *css_path) {
    GtkCssProvider *css_provider = gtk_css_provider_new();
    GError *error = NULL;

    if (!gtk_css_provider_load_from_path(css_provider, css_path, &error)) {
        g_printerr("Помилка завантаження CSS: %s\n", error->message);
        g_clear_error(&error);
        g_object_unref(css_provider);
        return;
    }

    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    g_object_unref(css_provider);
    g_print("CSS завантажено успішно\n");
}


// Функція для додавання елементу до listbox_global_rooms
void add_room_to_list(const char *room_name, int id) {
    // Перевірка існування глобального списку
    if (!listbox) {
        g_printerr("Помилка: listbox_global_rooms не знайдений.\n");
        return;
    }

    // Створюємо новий елемент (рядок)
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Додаємо контейнер для тексту
    GtkWidget *label = gtk_label_new(room_name);

    // Вирівнюємо текст
    gtk_label_set_xalign(GTK_LABEL(label), 0.0); // Текст вирівняний по лівому краю

    // Додаємо мітку до контейнера
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);

    // Додаємо контейнер до рядка
    gtk_container_add(GTK_CONTAINER(row), box);

    // Зберігаємо id у рядку
    g_object_set_data(G_OBJECT(row), "chat_id", GINT_TO_POINTER(id));

    // Додаємо рядок до списку
    gtk_list_box_insert(GTK_LIST_BOX(listbox), row, -1);

    // Відображаємо новий елемент
    gtk_widget_show_all(row);
}



// Профільне вікно
GtkWidget *dialog_profile_sett = NULL;
GtkWidget *btn_edit_profile = NULL;
GtkWidget *btn_logout = NULL;
GtkWidget *entry_user_name = NULL;
GtkWidget *label_profile_login = NULL;

// Обробник кнопки профілю
void on_btn_profile_settings_clicked(GtkButton *button, gpointer user_data) {

    // Показуємо діалогове вікно налаштувань профілю
    if (dialog_profile_sett) {
        // Завантажуємо дані користувача
        gtk_label_set_text(GTK_LABEL(label_profile_login), get_username_global());
        gtk_entry_set_text(GTK_ENTRY(entry_user_name), get_username_global());

        gtk_widget_show_all(dialog_profile_sett);
    } else {
        g_printerr("Помилка: Профільне вікно не знайдено!\n");
    }
}

// Обробник кнопки редагування профілю
void on_btn_edit_profile_clicked(GtkButton *button, gpointer user_data) {
    const gchar *new_username = gtk_entry_get_text(GTK_ENTRY(entry_user_name));
    if (g_strcmp0(new_username, "") != 0) {
        // Реалізуйте логіку для оновлення імені користувача
        g_print("Оновлення імені користувача: %s\n", new_username);
        set_username_global(new_username); // Оновлюємо глобальне ім'я
        gtk_label_set_text(GTK_LABEL(label_profile_login), new_username);
        update_username_request(build_url(host, port, "auth/update_username"), get_global_token(), new_username, get_pass());
    } else {
        g_printerr("Помилка: Ім'я користувача не може бути порожнім!\n");
    }
}


// Обробник кнопки виходу
void on_btn_logout_clicked(GtkButton *button, gpointer user_data) {
    g_print("Натиснуто 'Logout'. Реалізуйте вихід із профілю.\n");
    // Реалізуйте логіку виходу користувача
    gtk_widget_hide(window_main);
    gtk_widget_hide(dialog_profile_sett);
    gtk_widget_show_all(auth_dialog);
    
    
}

void init_mainpage(GtkBuilder *builder) {
    // Ініціалізація основних елементів
    listbox_msg = GTK_LIST_BOX(gtk_builder_get_object(builder, "listbox_found_msgs"));
    listbox = GTK_WIDGET(gtk_builder_get_object(builder, "listbox_global_rooms"));
    btn_add_room = GTK_WIDGET(gtk_builder_get_object(builder, "btn_add_room"));
    listbox_global_rooms = GTK_WIDGET(gtk_builder_get_object(builder, "listbox_global_rooms"));
    msg_entry = GTK_WIDGET(gtk_builder_get_object(builder, "msg_entry"));
    btn_send_msg = GTK_WIDGET(gtk_builder_get_object(builder, "btn_send_msg"));
    box_main = GTK_WIDGET(gtk_builder_get_object(builder, "box_main"));
    knipka_zakrittya = GTK_WIDGET(gtk_builder_get_object(builder, "knipka_zakrittya"));
    entry_room_name_1 = GTK_WIDGET(gtk_builder_get_object(builder, "entry_room_name_1"));
    btn_addroom_apply = GTK_WIDGET(gtk_builder_get_object(builder, "btn_addroom_apply"));
    notebook_auth_1 = GTK_WIDGET(gtk_builder_get_object(builder, "note_dialog_addroom"));
    group_name = GTK_WIDGET(gtk_builder_get_object(builder, "group_name"));
    group_members = GTK_WIDGET(gtk_builder_get_object(builder, "group_members"));
    btn_profile_sett = GTK_WIDGET(gtk_builder_get_object(builder, "btn_profile_sett"));
    label_header_roomname = GTK_WIDGET(gtk_builder_get_object(builder, "label_header_roomname"));
    profile_exit_btn = GTK_WIDGET(gtk_builder_get_object(builder, "profile_exit_btn"));

    // Ініціалізація елементів профільного вікна
    dialog_profile_sett = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_profile_sett"));
    btn_edit_profile = GTK_WIDGET(gtk_builder_get_object(builder, "btn_edit_profile"));
    btn_logout = GTK_WIDGET(gtk_builder_get_object(builder, "btn_logout"));
    entry_user_name = GTK_WIDGET(gtk_builder_get_object(builder, "entry_user_name"));
    label_profile_login = GTK_WIDGET(gtk_builder_get_object(builder, "label_profile_login"));

    if (!btn_add_room || !listbox_global_rooms || !msg_entry || !btn_send_msg) {
        g_printerr("Помилка: не вдалося завантажити основні віджети.\n");
        exit(EXIT_FAILURE);
    }

    // Сигнали для основних кнопок
    g_signal_connect(btn_add_room, "clicked", G_CALLBACK(on_add_room_clicked), NULL);
    g_signal_connect(btn_show_roomlist, "clicked", G_CALLBACK(on_show_roomlist_clicked), NULL);
    g_signal_connect(btn_send_msg, "clicked", G_CALLBACK(on_btn_send_msg_clicked), NULL);
    g_signal_connect(btn_profile_sett, "clicked", G_CALLBACK(on_btn_profile_settings_clicked), NULL);
    g_signal_connect(knipka_zakrittya, "clicked", G_CALLBACK(on_knipka_zakrittya_clicked), NULL);
    g_signal_connect(btn_addroom_apply, "clicked", G_CALLBACK(on_btn_addroom_apply_clicked), NULL);
    g_signal_connect(notebook_auth_1, "switch-page", G_CALLBACK(on_notebook_auth_switch_page_1), NULL);
    g_signal_connect(listbox, "row-activated", G_CALLBACK(on_row_activated), NULL);
    g_signal_connect(profile_exit_btn, "clicked", G_CALLBACK(on_profile_exit_btn_clicked), NULL);

    // Сигнали для профільного вікна
    g_signal_connect(btn_edit_profile, "clicked", G_CALLBACK(on_btn_edit_profile_clicked), NULL);
    g_signal_connect(btn_logout, "clicked", G_CALLBACK(on_btn_logout_clicked), NULL);
}

int current_page_member = 0;

void on_notebook_auth_switch_page_1(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data) {
    current_page_member = page_num; // Обновляем текущую страницу
    g_print("Switched to page: %d\n", page_num);
}

// Обработчик для кнопки "Apply"
void on_btn_addroom_apply_clicked(GtkButton *button, gpointer user_data) {
    t_chat *chat = (t_chat *)user_data;
    if (current_page_member == 0) { // Solo member
        const gchar *input = gtk_entry_get_text(GTK_ENTRY(entry_room_name_1));
        // Отправить запрос на сервер
        printf("\nhost: %s, port: %s\n", host, port);
        char* url_link = "chats/create_private";       
        size_t url_length = snprintf(NULL, 0, "http://%s:%s/%s", host, port, url_link) + 1;
    
        char *url = malloc(url_length);
    
        if (url == NULL) {
            fprintf(stderr, "Помилка виділення пам'яті\n");
            return;
        }
        
        // Формуємо рядок
        snprintf(url, url_length, "http://%s:%s/%s", host, port, url_link);
        const char* piptik = url;
        create_private_chat(piptik, get_global_token(), input);
    } else if (current_page_member == 1) { // Multimembers
        const char *name = gtk_entry_get_text(GTK_ENTRY(group_name));
        const char *members = gtk_entry_get_text(GTK_ENTRY(group_members));



        // Создаем корневой JSON-объект
        cJSON *root = cJSON_CreateObject();

        // Создаем массив JSON
        cJSON *json_array = cJSON_CreateArray();

        // Разделяем строку по запятым и добавляем элементы в массив
        char *substring = strtok(members, ",");
        while (substring != NULL) {
            // Удаляем лишние пробелы
            while (*substring == ' ') substring++;
            cJSON_AddItemToArray(json_array, cJSON_CreateString(substring));
            substring = strtok(NULL, ",");
        }

        // Добавляем массив в объект JSON под ключом "usernames"
        cJSON_AddItemToObject(root, "users", json_array);
        cJSON_AddStringToObject(root, "name", name);
        // Печатаем результат в виде строки
        char *json_string = cJSON_Print(root);
 
        char* url_link = "chats/create_group";       
        size_t url_length = snprintf(NULL, 0, "http://%s:%s/%s", host, port, url_link) + 1;
    
        char *url = malloc(url_length);
    
        if (url == NULL) {
            fprintf(stderr, "Помилка виділення пам'яті\n");
            return;
        }

        // Формуємо рядок
        snprintf(url, url_length, "http://%s:%s/%s", host, port, url_link);

        const char* piptik = url;

        create_group_chat(piptik, get_global_token(), name, members);
        
        
        // Освобождаем память
        cJSON_Delete(root);
        free(json_string);

    }
    gtk_widget_hide(dialog_addroom);
}

void on_btn_send_msg_clicked(GtkButton *button, gpointer user_data) {
    GtkTextBuffer *buffer;
    GtkTextIter start, end;
    gchar *msg_input;

    // Отримуємо буфер тексту з msg_entry
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(msg_entry));
    if (buffer == NULL) {
        g_print("Помилка: буфер тексту є NULL.\n");
        return;
    }

    // Отримуємо текст із буфера
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    msg_input = gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    char* url_link = "messages/send";       
    size_t url_length = snprintf(NULL, 0, "http://%s:%s/%s", host, port, url_link) + 1;
    char *url = malloc(url_length);
    if (url == NULL) {
        fprintf(stderr, "Помилка виділення пам'яті\n");
        return NULL;
    }
        
    // Формуємо рядок
    
    snprintf(url, url_length, "http://%s:%s/%s", host, port, url_link);
    char snum[5];
    snprintf(snum, sizeof(snum), "%d", get_chat_id_global());
    const char *chat_id = snum;
    g_print("ID - %d\n", get_chat_id_global());
    g_print("Before send message - %s\n", chat_id);
    send_message(url, get_global_token(), chat_id, msg_input);
    time_t rawtime;
    struct tm *timeinfo;
    char buffer_time[9]; // Формат "HH:MM:SS" потребує 8 символів + 1 для '\0'

    // Отримуємо поточний час
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer_time, sizeof(buffer_time), "%H:%M:%S", timeinfo);

    //add_message_to_list(msg_input, buffer_time, );

    // Звільняємо виділену пам'ять для тексту
    g_free(msg_input);

    clear_text_view(msg_entry);
}


void on_add_room_clicked(GtkButton *button, gpointer user_data) {


    gtk_widget_show_all(dialog_addroom);
}

void on_knipka_zakrittya_clicked(GtkButton *button, gpointer user_data){
    gtk_widget_hide(dialog_addroom);
}

void on_profile_exit_btn_clicked(GtkButton *button, gpointer user_data){
    gtk_widget_hide(dialog_profile_sett);
}

void on_show_roomlist_clicked(GtkButton *button, gpointer user_data) {
    gboolean is_visible = gtk_widget_get_visible(listbox_global_rooms);
    gtk_widget_set_visible(listbox_global_rooms, !is_visible);
    g_print("Список кімнат %s.\n", is_visible ? "сховано" : "відображено");
}

void on_send_message_clicked(GtkButton *button, gpointer user_data) {
    const gchar *message = gtk_entry_get_text(GTK_ENTRY(msg_entry));
    if (strlen(message) > 0) {
        g_print("Повідомлення відправлено: %s\n", message);
        gtk_entry_set_text(GTK_ENTRY(msg_entry), "");
    } else {
        g_print("Неможливо відправити порожнє повідомлення.\n");
    }
}


void on_row_activated(GtkListBox *box, GtkListBoxRow *row, gpointer user_data) {
    // Отримуємо id з рядка
    int chat_id = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(row), "chat_id"));

    clear_listbox(GTK_WIDGET(listbox_msg));
    // Отримуємо контейнер всередині рядка
    GtkWidget *box_container = gtk_bin_get_child(GTK_BIN(row));
    if (!box_container) {
        g_printerr("Не вдалося отримати контейнер для рядка.\n");
        return;
    }

    // Отримуємо лейбл з іменем чату
    GList *children = gtk_container_get_children(GTK_CONTAINER(box_container));
    if (children && GTK_IS_LABEL(children->data)) {
        const gchar *chat_name = gtk_label_get_text(GTK_LABEL(children->data));
        g_print("Вибраний чат: %s (ID: %d)\n", chat_name, chat_id);
        gtk_label_set_text(GTK_LABEL(label_header_roomname), chat_name);
    }
    
    // Звільняємо пам'ять
    g_list_free(children);

    // Оновлюємо глобальний ID чату
    set_chat_id_global(chat_id);

    // Завантажуємо повідомлення для вибраного чату
    fetch_and_display_messages(chat_id);
    start_auto_update(chat_id, 500);
}



void fetch_and_display_messages(int chat_id) {
    GError *error = NULL;
    GSocketClient *client = g_socket_client_new();
    GSocketConnection *connection = NULL;
    GOutputStream *output_stream = NULL;
    GInputStream *input_stream = NULL;
    GString *response = g_string_new(NULL);
    cJSON *json = NULL;

    // Очищення списку повідомлень
    if (listbox_msg) {
        GList *children = gtk_container_get_children(GTK_CONTAINER(listbox_msg));
        for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
            gtk_widget_destroy(GTK_WIDGET(iter->data));
        }
        g_list_free(children);
    }

    if (listbox_msg) {
        clear_listbox(GTK_WIDGET(listbox_msg));
    }
    
    // Формуємо URL для запиту
    char url[256];
    char *first_part_url =  build_url(host, port,"messages/history/");
    g_print("first part url: %s\n", first_part_url);
    g_print("chat id: %d\n", chat_id);
    char request1[512];

    snprintf(request1, sizeof(request1), "%s%d", first_part_url, chat_id);
    g_print("request: %s\n", request1);

    // Парсинг URL
    GUri *uri = g_uri_parse(request1, G_URI_FLAGS_NONE, &error);
    if (!uri) {
        g_printerr("Помилка парсингу URL: %s\n", error->message);
        goto cleanup;
    }

    // Встановлення з'єднання
    connection = g_socket_client_connect_to_host(client, g_uri_get_host(uri), g_uri_get_port(uri), NULL, &error);
    if (!connection) {
        g_printerr("Помилка підключення: %s\n", error->message);
        goto cleanup;
    }

    output_stream = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    input_stream = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    if (!output_stream || !input_stream) {
        g_printerr("Помилка отримання потоків вводу/виводу.\n");
        goto cleanup;
    }

    // Формуємо HTTP-запит
    char request[512];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "Authorization: Bearer %s\r\n"
             "Connection: close\r\n"
             "\r\n",
             g_uri_get_path(uri), g_uri_get_host(uri), get_global_token());

    // Відправка запиту
    g_output_stream_write_all(output_stream, request, strlen(request), NULL, NULL, &error);
    if (error) {
        g_printerr("Помилка відправки даних: %s\n", error->message);
        goto cleanup;
    }

    // Читання відповіді
    char buffer[1024];
    while (TRUE) {
        gssize bytes_read = g_input_stream_read(input_stream, buffer, sizeof(buffer) - 1, NULL, &error);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            g_string_append(response, buffer);
        } else if (bytes_read == 0) {
            break;
        } else {
            g_printerr("Помилка читання відповіді: %s\n", error->message);
            goto cleanup;
        }
    }

    // Очищуємо JSON із відповіді
    const char *json_data = extract_json_from_response_3(response->str);
    if (!json_data) {
        g_printerr("Помилка обробки HTTP-відповіді.\n");
        goto cleanup;
    }

    // Парсимо JSON
    json = cJSON_Parse(json_data);
    if (!json) {
        g_printerr("Помилка парсингу JSON: %s\n", cJSON_GetErrorPtr());
        goto cleanup;
    }

    // Обробка повідомлень
    cJSON *message_array = json;
    if (!cJSON_IsArray(message_array)) {
        g_printerr("Очікувався масив повідомлень у відповіді.\n");
        goto cleanup;
    }

    cJSON *message = NULL;
    cJSON_ArrayForEach(message, message_array) {
        cJSON *text = cJSON_GetObjectItemCaseSensitive(message, "message");
        cJSON *time = cJSON_GetObjectItemCaseSensitive(message, "sent_at");
        cJSON *sender= cJSON_GetObjectItemCaseSensitive(message, "sender");

        if (cJSON_IsString(text) && cJSON_IsString(time)) {
            add_message_to_list(text->valuestring, time->valuestring, sender->valuestring);
        }
    }
    

cleanup:
    if (uri) g_uri_unref(uri);
    if (connection) {
        g_io_stream_close(G_IO_STREAM(connection), NULL, NULL);
        g_object_unref(connection);
    }
    if (response) g_string_free(response, TRUE);
    if (json) cJSON_Delete(json);
}

const char *extract_json_from_response_3(const char *response) {
    const char *json_start = strchr(response, '['); // Шукаємо початок JSON (масив '[')
    if (json_start == NULL) {
        g_printerr("JSON не знайдено у відповіді сервера.\n");
        return NULL;
    }
    return json_start;
}

