#ifndef UTILS_H
#define UTILS_H

void init_directories();
void bytes_to_hex(unsigned char *bytes, int len, char *hex_string);
void hex_to_bytes(const char *hex, unsigned char *bytes);
void hash_password_simple(const char *password, char *output_hash);

#endif