#ifndef UTILISATEUR_H
#define UTILISATEUR_H

// Structure Utilisateur
typedef struct {
    int id;
    char nom[50];
    char email[100];
    char mot_de_passe[100]; // Hashé dans le vrai projet
} Utilisateur;

#endif // UTILISATEUR_H
