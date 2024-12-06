#ifndef USERS_H
#define USERS_H

#include <sqlite3.h>

typedef struct {
    char *id;
    char *username;
    char *passhash;
    char *created_at;
} User;

User* get_user_by_uuid(sqlite3 *db, const char *uuid);
User* get_user_by_username(sqlite3 *db, const char *username);
void free_user(User *user);

int create_user(sqlite3 *db, const char *username, const char *passhash);
int update_user_password(sqlite3 *db, const char *uuid, const char *new_passhash);
int update_user_username(sqlite3 *db, const char *uuid, const char *new_username);
int delete_user(sqlite3 *db, const char *uuid);
int check_user_credentials(sqlite3 *db, const char *username, const char *password);

#endif // USERS_H
