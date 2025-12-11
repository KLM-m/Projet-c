#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include "database.h"

// Vos identifiants
#define DB_HOST "192.168.86.128"
#define DB_USER "testuser"
#define DB_PASS "Azerty01"
#define DB_NAME "projet_c"

MYSQL* get_db_connection() {
    // CORRECTION : Déclaration correcte du pointeur 'conn'
    MYSQL *conn = mysql_init(NULL); 

    // Test de l'initialisation du pointeur
    if (conn == NULL) {
        fprintf(stderr, "Erreur mysql_init() : Impossible d'initialiser l'objet MySQL.\n");
        return NULL;
    }

    // --- FIX ANTI-OVERRIDE DE CONFIGURATION ET SSL ---

    // 1. On empêche le client de lire my.ini/my.cnf
    mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, ""); 

    // 2. On désactive la vérification du certificat serveur (pour contourner l'obligation SSL)
    my_bool verify_server_cert = 0; 
    mysql_options(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify_server_cert);

    // Connexion au serveur
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "\n[ERREUR] Impossible de se connecter a la BDD !\n");
        fprintf(stderr, "Détail : %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL; 
    }

    return conn;
}