#include "crypto.h"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string.h>
#include <stdio.h>

void generate_salt(char *salt, size_t salt_size) {
    if (RAND_bytes((unsigned char *)salt, salt_size) != 1) {
        fprintf(stderr, "Error generating salt\n");
        return;
    }

    for (size_t i = 0; i < salt_size - 1; i++) {
        salt[i] = (salt[i] % 26) + 'a';
    }
    salt[salt_size - 1] = '\0';
}

void hash_password(const char *password, const char *salt, char *hash_output) {
    unsigned char hash_value[EVP_MAX_MD_SIZE];
    unsigned int hash_value_len;

    char salted_password[SALT_SIZE + strlen(password) + 1];
    snprintf(salted_password, sizeof(salted_password), "%s%s", password, salt);

    EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        fprintf(stderr, "Error initializing hash context\n");
        return;
    }

    if (EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL) != 1 ||
        EVP_DigestUpdate(mdctx, salted_password, strlen(salted_password)) != 1 ||
        EVP_DigestFinal_ex(mdctx, hash_value, &hash_value_len) != 1) {
        fprintf(stderr, "Error generating hash\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    EVP_MD_CTX_free(mdctx);

    for (unsigned int i = 0; i < hash_value_len; i++) {
        snprintf(&hash_output[i * 2], 3, "%02x", hash_value[i]);
    }
    hash_output[hash_value_len * 2] = '\0';
}

int verify_password(const char *password, const char *salt, const char *expected_hash) {
    char computed_hash[HASH_SIZE + 1];
    hash_password(password, salt, computed_hash);

    return strcmp(computed_hash, expected_hash) == 0;
}
