#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h> 
#endif
#include "admin_connexion.h"
#include "custom_rsa.h" 

int fichier_existant(const char* chemin) {
    struct stat buffer;
    return (stat(chemin, &buffer) == 0);
}

void creer_dossier_secrets() {
    struct stat st = {0};
    if (stat(".secrets", &st) == -1) {
        if (mkdir(".secrets"
#ifndef _WIN32
        , 0700
#endif
        ) != 0) {
            exit(EXIT_FAILURE); 
        }
    }
}

void verifier_ou_generer_cle_locale() {
    creer_dossier_secrets();
    if (!fichier_existant(".secrets/private_key.pem") || !fichier_existant(".secrets/public_key.pem")) {
        if (!custom_rsa_generer_et_sauvegarder_cles(".secrets/public_key.pem", ".secrets/private_key.pem")) {
            exit(EXIT_FAILURE);
        }
    }
}

int main() {
    verifier_ou_generer_cle_locale();

    int choix = -1;
    char buffer[10];

    do {
        printf("\n==============================\n");
        printf("   Bienvenue dans l'application\n");
        printf("==============================\n");
        printf("1 - Panel utilisateur\n");
        printf("2 - Panel admin\n");
        printf("0 - Quitter\n");
        printf("------------------------------\n");
        printf("Votre choix : ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &choix) != 1) {
                choix = -1; 
            }
        }

        switch(choix) {
            case 1:
                printf("Panel utilisateur non implémenté pour le moment.\n");
                break;
            case 2:
                menu_admin_connexion();
                break;
            case 0:
                printf("Au revoir !\n");
                break;
            default:
                printf("Choix invalide. Veuillez recommencer.\n");
        }
    } while (choix != 0);

    return 0;
}