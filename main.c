#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h> 
#endif

// Vos headers existants
#include "admin_connexion.h"
#include "custom_rsa.h" 

// --- NOUVEAUX HEADERS AJOUTÉS ---
#include "client_auth.h"   // Pour connexion_client / creer_compte
#include "accueil_users.h" // Pour PANEL (le dashboard utilisateur)

// Fonction utilitaire pour vérifier l'existence de fichier
int fichier_existant(const char* chemin) {
    struct stat buffer;
    return (stat(chemin, &buffer) == 0);
}

// Fonction pour créer le dossier .secrets
void creer_dossier_secrets() {
    struct stat st = {0};
    if (stat(".secrets", &st) == -1) {
        if (mkdir(".secrets"
#ifndef _WIN32
        , 0700
#endif
        ) != 0) {
            perror("Erreur création dossier .secrets");
            exit(EXIT_FAILURE); 
        }
    }
}

// Vérification des clés RSA locales
void verifier_ou_generer_cle_locale() {
    creer_dossier_secrets();
    if (!fichier_existant(".secrets/private_key.pem") || !fichier_existant(".secrets/public_key.pem")) {
        printf("Génération des clés RSA locales...\n");
        if (!custom_rsa_generer_et_sauvegarder_cles(".secrets/public_key.pem", ".secrets/private_key.pem")) {
            fprintf(stderr, "Erreur fatale : Impossible de générer les clés RSA.\n");
            exit(EXIT_FAILURE);
        }
    }
}

// --- SOUS-MENU UTILISATEUR ---
void menu_utilisateur_entree() {
    int choix = -1;
    char buffer[10];

    do {
        printf("\n------------------------------\n");
        printf("      ESPACE UTILISATEUR      \n");
        printf("------------------------------\n");
        printf("1 - Se connecter\n");
        printf("2 - Créer un compte\n");
        printf("0 - Retour au menu principal\n");
        printf("------------------------------\n");
        printf("Choix : ");

        if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
            if (sscanf(buffer, "%d", &choix) != 1) choix = -1;
        }

        switch(choix) {
            case 1: {
                // TENTATIVE DE CONNEXION
                SessionClient* session = connexion_client();
                if (session != NULL) {
                    // SI REUSSI -> REDIRECTION VERS ACCUEIL_USERS (PANEL)
                    PANEL(session);
                    
                    // Une fois qu'on quitte le panel, on libère la mémoire
                    free(session);
                }
                break;
            }
            case 2:
                // INSCRIPTION
                creer_compte_client();
                break;
            case 0:
                printf("Retour...\n");
                break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 0);
}

// --- MAIN ---
int main() {
    // Initialisation
    verifier_ou_generer_cle_locale();

    int choix = -1;
    char buffer[10];

    do {
        printf("\n==============================\n");
        printf("   APPLICATION SECURISEE V2   \n");
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
                // APPEL DU SOUS-MENU UTILISATEUR
                menu_utilisateur_entree();
                break;
                
            case 2:
                // APPEL DU MENU ADMIN EXISTANT
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