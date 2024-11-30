#include <sodium.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE crypto_pwhash_STRBYTES

char* hash_password(const char *password) {
    char *hash = malloc(HASH_SIZE);
    if (!hash) {
        fprintf(stderr, "Failed to allocate memory for password hash\n");
        return NULL;
    }

    if (crypto_pwhash_str(hash, password, strlen(password),
                          crypto_pwhash_OPSLIMIT_INTERACTIVE,
                          crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        fprintf(stderr, "Password hashing failed (out of memory?)\n");
        free(hash);
        return NULL;
    }

    return hash;
}

int verify_password(const char *password, const char *stored_hash) {
    if (crypto_pwhash_str_verify(stored_hash, password, strlen(password)) == 0) {
        return 1;
    } else {
        return 0;
    }
}
