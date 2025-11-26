#include <stdio.h>
#include "database.h"

// Constantes pour la connexion à la base de données
#define DB_HOST "192.168.153.130"
#define DB_USER "testuser"
#define DB_PASS "Azerty01"
#define DB_NAME "projet_c"

MYSQL* get_db_connection() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Erreur mysql_init() : Impossible d'initialiser l'objet MySQL.\n");
        return NULL;
    }

    // Activer et exiger le mode SSL/TLS pour la connexion.
    // La connexion échouera si le serveur ne supporte pas SSL.
    enum mysql_ssl_mode ssl_mode = SSL_MODE_REQUIRED;
    if (mysql_options(conn, MYSQL_OPT_SSL_MODE, &ssl_mode)) {
        fprintf(stderr, "Erreur mysql_options(SSL_MODE_REQUIRED) : %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Erreur de connexion a la BDD : %s\n", mysql_error(conn));
        mysql_close(conn); // Nettoyer l'objet conn initialisé en cas d'échec
        return NULL;
    }

    return conn;
}