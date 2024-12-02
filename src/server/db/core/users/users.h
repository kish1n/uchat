#ifndef USER_H
#define USER_H

#include <libpq-fe.h>

typedef struct {
    char *id;
    char *username;
    char *passhash;
    char *created_at;
} User;

User* get_user_by_uuid(PGconn *conn, const char *uuid);
User* get_user_by_username(PGconn *conn, const char *username);
void free_user(User *user);

int create_user(PGconn *conn, const char *username, const char *passhash);
int update_user_password(PGconn *conn, const char *uuid, const char *new_passhash);
int update_user_username(PGconn *conn, const char *uuid, const char *new_username);
int delete_user(PGconn *conn, const char *uuid);
int check_user_credentials(PGconn *conn, const char *username, const char *password);

#endif // USER_H
