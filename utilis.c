#include "utils.h"
#include "config.h"
#include <openssl/sha.h>
#include <sys/stat.h>

void init_directories() {
#ifdef _WIN32
    mkdir(STORAGE_DIR);
    mkdir(KEYS_DIR);
    mkdir(FILES_DIR);
    mkdir(".secrets");
#else
    mkdir(STORAGE_DIR, 0777);
    mkdir(KEYS_DIR, 0777);
    mkdir(FILES_DIR, 0777);
    mkdir(".secrets", 0700);
#endif
}

void bytes_to_hex(unsigned char *bytes, int len, char *hex_string) {
    for (int i = 0; i < len; i++) sprintf(hex_string + (i * 2), "%02x", bytes[i]);
    hex_string[len * 2] = 0;
}

void hex_to_bytes(const char *hex, unsigned char *bytes) {
    for (unsigned int i = 0; i < strlen(hex) / 2; i++) sscanf(hex + 2 * i, "%02hhx", &bytes[i]);
}

void hash_password_simple(const char *password, char *output_hash) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    bytes_to_hex(hash, SHA256_DIGEST_LENGTH, output_hash);
}