#ifndef CRYPTO_H
#define CRYPTO_H

#define HASH_SIZE crypto_pwhash_STRBYTES

char* hash_password(const char *password);
int verify_password(const char *password, const char *stored_hash);

#endif // CRYPTO_H
