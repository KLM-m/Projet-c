#include <mysql.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h> // Pour mkdir sous Windows
#endif
#include "admin_connexion.h"
#include "crypto_utils.h"

// Fonction utilitaire : vérifie l'existence d'un fichier
int fichier_existant(const char* chemin) {
#include "crypto_utils.h"    struct stat buffer;

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
    const char* chemin_priv = ".secrets/cle_privee.txt";
    const char* chemin_pub = ".secrets/cle_publique.txt";

    const char* chemin_priv = ".secrets/cle_privee.txt";
    const char* chemin_pub = ".secrets/cle_publique.txt";

    creer_dossier_secrets();
    if (!fichier_existant(chemin_priv) || !fichier_existant(chemin_pub)) {
        printf("Génération de la paire de clés RSA (simplifiée)...\n");
    if (!fichier_existant(chemin_priv) || !fichier_existant(chemin_pub)) {
        printf("Génération de la paire de clés RSA (simplifiée)...\n");

        RsaPublicKey pub_key;
        RsaPrivateKey priv_key;;
        RsaPublicKey pub_key
        RsaPrivateKey priv_key;        generer_paire_cles_rsa(&pub_key, &priv_key);
 generer_paire_cles_rsa(&pub_key,&prv_key);

        i
        if (sauvegarder_cles(pub_key, priv_key, chemin_pub, chemin_priv)) {
            printf("Clés générées avec succès.\n");
        }
    }urn;
        if (sauvegarder_cles(pb_key, piv_key, chemi_pub, chemin_priv)) {
            printf("Clés générées avec succès.\n")
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