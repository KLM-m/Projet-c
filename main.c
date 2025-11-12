#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "admin_connexion.h"

// Fonction utilitaire : vérifie l'existence d'un fichier
int fichier_existant(const char* chemin) {
    struct stat buffer;
    return (stat(chemin, &buffer) == 0);
}

// Création du dossier caché .secrets si besoin
void creer_dossier_secrets() {
    struct stat st = {0};
    if (stat(".secrets", &st) == -1) {
        #ifdef _WIN32
        mkdir(".secrets");
        #else
        mkdir(".secrets", 0700);
        #endif
    }
}

// Génération/vérification clés: uniquement local (.secrets), pas d'insertion BDD ici
void verifier_ou_generer_cle_locale() {
    creer_dossier_secrets();
    if (!fichier_existant(".secrets/private_key.pem") || !fichier_existant(".secrets/public_key.pem")) {
        #ifdef _WIN32
        system("mkdir .secrets");
        #else
        system("mkdir -p .secrets");
        #endif
        system("openssl genrsa -out .secrets/private_key.pem 2048");
        system("openssl rsa -in .secrets/private_key.pem -pubout -out .secrets/public_key.pem");
    }
}

// Prototypes à remplir selon tes fonctions de gestion utilisateur et admin
void menu_utilisateur();
void menu_admin();

int main() {
    int choix;

    // S'assurer que la paire locale existe (sans toucher à la BDD)
    verifier_ou_generer_cle_locale();

    printf("==============================\n");
    printf("   Bienvenue dans l'application\n");
    printf("==============================\n");
    printf("1 - Panel utilisateur (Creer/Se connecter)\n");
    printf("2 - Panel admin (Connexion superviseur)\n");
    printf("0 - Quitter\n");
    printf("------------------------------\n");
    printf("Votre choix : ");
    scanf("%d", &choix);

    switch(choix) {
        case 1:
            printf("Panel utilisateur non implemente pour le moment.\n");
            break;
        case 2:
            menu_admin_connexion();
            break;
        case 0:
            printf("Au revoir !\n");
            exit(0);
        default:
            printf("Choix invalide. Veuillez recommencer.\n");
    }
    return 0;
}

// À compléter plus tard :
void menu_utilisateur() {
    printf("Panneau utilisateur :\n");
    printf("1 - Créer un compte\n");
    printf("2 - Se connecter\n");
    // Ajoute ici les appels à tes fonctions correspondantes
}

void menu_admin() {
    printf("Panneau Admin - Connexion:\n");
    // Demander email/mot de passe ou autre selon ton système
    // Appel à la fonction de connexion admin
}