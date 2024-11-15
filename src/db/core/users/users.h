#ifndef USER_H
#define USER_H

#include <libpq-fe.h>

int create_user(PGconn *conn, const char *username, const char *passhash);
int update_user_password(PGconn *conn, const char *uuid, const char *new_passhash);
int update_user_username(PGconn *conn, const char *uuid, const char *new_username);
int delete_user(PGconn *conn, const char *uuid);

#endif // USER_H
