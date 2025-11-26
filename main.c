#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h> // Pour mkdir sous Windows
#endif
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h> // Pour la nouvelle API
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
        // mkdir retourne 0 en cas de succès
        if (mkdir(".secrets"
#ifndef _WIN32
        , 0700
#endif
        ) != 0) {
            perror("Impossible de créer le dossier .secrets");
        }
    }
}

// Génération/vérification clés: uniquement local (.secrets), pas d'insertion BDD ici
void verifier_ou_generer_cle_locale() {
    creer_dossier_secrets();
    if (!fichier_existant(".secrets/private_key.pem") || !fichier_existant(".secrets/public_key.pem")) {
        printf("Génération de la paire de clés RSA (2048 bits)...\n");

        EVP_PKEY *pkey = NULL;
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

        if (!ctx || EVP_PKEY_keygen_init(ctx) <= 0 || EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0) {
            fprintf(stderr, "Erreur lors de la génération de la clé RSA.\n");
            ERR_print_errors_fp(stderr);
            if(ctx) EVP_PKEY_CTX_free(ctx);
            return;
        }

        if (EVP_PKEY_generate(ctx, &pkey) <= 0) {
            fprintf(stderr, "Erreur lors de la génération de la clé RSA.\n");
            ERR_print_errors_fp(stderr);
            EVP_PKEY_CTX_free(ctx);
            return;
        }
        EVP_PKEY_CTX_free(ctx);

        // Sauvegarder la clé privée
        FILE *private_key_file = fopen(".secrets/private_key.pem", "wb");
        if (!private_key_file) {
            perror("Impossible d'ouvrir .secrets/private_key.pem pour écriture");
            return;
        }
        PEM_write_PrivateKey(private_key_file, pkey, NULL, NULL, 0, NULL, NULL);
        fclose(private_key_file);

        // Sauvegarder la clé publique
        FILE *public_key_file = fopen(".secrets/public_key.pem", "wb");
        if (!public_key_file) {
            perror("Impossible d'ouvrir .secrets/public_key.pem pour écriture");
        }
        PEM_write_PUBKEY(public_key_file, pkey);
        fclose(public_key_file);

        EVP_PKEY_free(pkey);
        printf("Clés générées avec succès.\n");
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