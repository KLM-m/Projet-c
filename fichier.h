#ifndef FICHIER_H
#define FICHIER_H

#include "utilisateur.h" // Pour relier le fichier à un utilisateur

// Structure Fichier
typedef struct {
    int id;
    char nom[100];
    char chemin[260]; // Chemin complet dans le système
    int id_utilisateur_source;
    int id_utilisateur_destinataire;
    char statut[20]; // exemple : 'en_attente', 'valide', 'rejete'
} Fichier;

#endif // FICHIER_H
