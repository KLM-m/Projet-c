#ifndef ACCUEIL_USERS_H
#define ACCUEIL_USERS_H

#include "client_types.h"

// La fonction réelle
void afficher_accueil_users(SessionClient* session);

// --- ALIAS ---
// Vous pouvez maintenant utiliser PANEL(session) au lieu du nom long
#define PANEL afficher_accueil_users

#endif