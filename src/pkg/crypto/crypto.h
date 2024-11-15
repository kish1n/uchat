#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>

#define SALT_SIZE 16
#define HASH_SIZE 64

void generate_salt(char *salt, size_t salt_size);

void hash_password(const char *password, const char *salt, char *hash_output);

int verify_password(const char *password, const char *salt, const char *expected_hash);

#endif // CRYPTO_H
