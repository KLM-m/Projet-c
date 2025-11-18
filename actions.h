#ifndef ACTIONS_H
#define ACTIONS_H
#include "config.h"

void envoyer_fichier(MYSQL *conn, int mon_id, int is_message);
void verifier_reception(MYSQL *conn, int mon_id);

#endif