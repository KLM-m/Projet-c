#ifndef DATABASE_H
#define DATABASE_H

#include <mysql.h>

// Établit et retourne une connexion à la BDD.
MYSQL* get_db_connection();

#endif // DATABASE_H