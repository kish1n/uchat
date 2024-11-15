#include <stdio.h>
#include "src/pkg/json_parser/json_parser.h"

int main() {
    const char *json_request = "{\"username\": \"testuser\", \"password\": \"securepass\", \"email\": \"user@example.com\"}";

    char username[64];
    char password[64];
    char email[64];

    // Извлечение данных из JSON
    if (get_json_value(json_request, "username", username, sizeof(username)) == 0) {
        printf("Username: %s\n", username);
    } else {
        printf("Failed to get 'username'\n");
    }

    if (get_json_value(json_request, "password", password, sizeof(password)) == 0) {
        printf("Password: %s\n", password);
    } else {
        printf("Failed to get 'password'\n");
    }

    if (get_json_value(json_request, "email", email, sizeof(email)) == 0) {
        printf("Email: %s\n", email);
    } else {
        printf("Failed to get 'email'\n");
    }

    return 0;
}
