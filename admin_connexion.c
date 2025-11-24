#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <mysql.h>
#include "admin_connexion.h"
#include "admin_panel.h"
#include "database.h"

// Fonction pour hasher un mot de passe en SHA-256
void hash_password(const char* password, char* hash_output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, password, strlen(password));
    SHA256_Final(hash, &ctx);
    
    // Convertir le hash binaire en string hexadecimale
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }
    hash_output[64] = '\0'; // SHA-256 produit 64 caracteres hexa
}

// Lire le contenu base64 de la clé publique locale (sans headers PEM)
static void lire_cle_publique_contenu(const char* chemin, char* buffer, size_t maxsize) {
    FILE* f = fopen(chemin, "r");
    if(!f) { buffer[0] = '\0'; return; }
    char ligne[200]; size_t pos = 0;
    while(fgets(ligne, sizeof(ligne), f)) {
        if(strncmp(ligne, "-----", 5) == 0) continue;
        size_t ll = strlen(ligne);
        if (pos + ll < maxsize-1) { memcpy(buffer+pos, ligne, ll); pos += ll; }
    }
    buffer[pos] = 0;
    // trim fin
    while(pos>0 && (buffer[pos-1]=='\n' || buffer[pos-1]=='\r')) { buffer[--pos]=0; }
    fclose(f);
}

int menu_admin_connexion() {
    char email[100], mot_de_passe[100];
    printf("-- Connexion Admin --\n");
    printf("Email : ");
    scanf("%99s", email);
    printf("Mot de passe : ");
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
            char cle_pub_b64[4096] = {0};
            lire_cle_publique_contenu(".secrets/public_key.pem", cle_pub_b64, sizeof(cle_pub_b64));
            if (strlen(cle_pub_b64) > 0) {
                char sql[4800];
                sprintf(sql, "UPDATE utilisateur SET cle_publique='%s' WHERE email='%s' AND (cle_publique IS NULL OR cle_publique='')", cle_pub_b64, email);
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
        mysql_free_result(resultat);
    } else {
        printf("Identifiants invalides ou pas admin.\n");
    }
    mysql_close(conn);
    return 0;
}
