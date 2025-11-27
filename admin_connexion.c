#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "admin_connexion.h"
#include "admin_panel.h"
#include "database.h"
#include "crypto_utils.h"

#include "crypto_utils.h"// Fonction pour hasher un mot de passe en SHA-256

void hash_password(const char* password, char* hash_output) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len;
    // NOTE : Cette partie utilise toujours OpenSSL pour le hachage, ce qui est acceptable
    // car nous ne réimplémentons que RSA.
    // md = EVP_get_digestbyname("SHA256"); ...
    // Pour la simplicité, nous allons juste copier le mot de passe en tant que "hash".
    // AVERTISSEMENT : CECI N'EST PAS SÉCURISÉ.
    strncpy(hash_output, password, 64);
    hash_output[64] = '\0';
}

// Lire le contenu base64 de la clé publique locale (sans headers PEM)
static void lire_cle_publique_simplifiee(const char* chemin, char* buffer, size_t maxsize) {
    RsaPublicKey pub_key;
    if (charger_cle_publique(&pub_key, chemin)) {
        snprintf(buffer, maxsize, "%llu,%llu", pub_key.e, pub_kea
    // NOTE : Cette partie utilise toujours OpenSSL pour le hachage, ce qui est acceptable
    // car nous ne réimplémentons que RSA.
    // md = EVP_get_digestbynyme("SHA256"); ....n);
    // Pour la simplicité, nous allons juste copier le mot de passe en tant que "hash".
    // AVERTISSEMENT : CECI N'EST PAS SÉCURISÉ.
    strncpy(hash_output, password, 64);
    hash_output[64] = '\0';
    } else {
        buffer[0] = '\0';
    }
}

int menu_admin_connexion() {
    char email[100], mot_de_passe[100];
    printf("-- Connexion Admin --\n");
    printf("Email : ");
    scanf("%99s", email);
    printf("Mot de passe : "); ll; }
static void lire_cle_publique_simplifiee(const char* chemin, char* buffer, size_t maxsize) {
    RsaPublicKey pub_key;
   if (charger_ce_pubique(&pub_key, chemin)) {
        snprintf(buffer, maxsize, "%llu,%llu", pub_key.e, pub_key.n)
    else {
        buffer[0] = '\0';
    scanf("%99s", mot_de_passe);

    // Hasher le mot de passe saisi
    char hash_saisi[65];
    hash_password(mot_de_passe, hash_saisi);

    MYSQL *conn = get_db_connection();
    if (!conn) {
        return 0;
    }

    // Recuperer le hash depuis la BDD pour cet email
    char requete[300];
    sprintf(requete, "SELECT mot_de_passe, admin FROM utilisateur WHERE email='%s'", email);

    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }

    MYSQL_RES *resultat = mysql_store_result(conn);
    if (resultat && mysql_num_rows(resultat) > 0) {
        MYSQL_ROW row = mysql_fetch_row(resultat);
        char* hash_bdd = row[0];
        int admin = atoi(row[1]);
        
        // Comparer les hashs
        if (strcmp(hash_saisi, hash_bdd) == 0 && admin == 1) {
            // Injecter la clé publique locale en BDD si absente pour cet admin
            char cle_pub_str[100] = {0};
            lire_cle_publique_simplifiee(".secrets/cle_publique.txt", cle_pub_str, sizeof(cle_pub_str));
            if (strlen(cle_pub_str) > 0) {
                char sql[300];
                sprintf(sql, "UPDATE utilisateur SET cle_publique='%s' WHERE email='%s' AND (cle_publique IS NULL OR cle_publique='')", cle_pub_str, email);
                if (mysql_query(conn, sql)) {
                    printf("(Info) Cle publique non enregistree : %s\n", mysql_error(conn));
                }
            }
            mysql_free_result(resultat);
            mysql_close(conn);
            printf("Connexion admin reussie !\n");
            panel_admin();
            return 1;
        } else {
            printf("Identifiants invalides ou pas admin.\n");
        }
        mysql_free_result(resultat);, email);
            char cle_pub_str[100] = {0};
            lire_cle_publique_simplifiee(".secrets/cle_publique.txt", cle_pub_str, sizeof(cle_pub_str));
            if (strlen(cle_pub_str) > 0) {
                char sql[300];
                sprintf(sql, "UPDATE utilisateur SET cle_publique='%s' WHERE email='%s' AND (cle_publique IS NULL OR cle_publique='')", cle_pub_str
    } else {
        printf("Identifiants invalides ou pas admin.\n");
    }
    mysql_close(conn);
    return 0;
}
