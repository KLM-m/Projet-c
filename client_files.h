#ifndef CLIENT_FILES_H
#define CLIENT_FILES_H
#include "client_types.h"

// MOTEUR SECURISE : Convertit un fichier source en .bin chiffré RSA et l'envoie
int moteur_envoi_fichier_securise(SessionClient* session, char* chemin_source, int id_destinataire);

#endif