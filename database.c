#include <stdio.h>
#include "database.h"

#define DB_HOST "192.168.86.128"
#define DB_USER "testuser"
#define DB_PASS "Azerty01"
#define DB_NAME "projet_c"

MYSQL* get_db_connection() {
    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Erreur mysql_init() : Impossible d'initialiser l'objet MySQL.\n");
        return NULL;
    }

    
    if (mysql_ssl_set(conn, NULL, NULL, NULL, NULL, NULL) != 0) {
        fprintf(stderr, "Erreur mysql_ssl_set() : %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "Erreur de connexion à la BDD : %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    return conn;
}