#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>

// --- CONFIGURATION BDD (A adapter si besoin) ---
#define DB_HOST "192.168.238.142"
#define DB_USER "testuser"
#define DB_PASS "Azerty01" 
#define DB_NAME "projet_c"

// --- DOSSIERS ---
#define STORAGE_DIR "messagard_storage"
#define KEYS_DIR "messagard_storage/keys"
#define FILES_DIR "messagard_storage/files"

// --- CRYPTO ---
#define AES_KEY_LEN 32
#define AES_IV_LEN 16

#endif