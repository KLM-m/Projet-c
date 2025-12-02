#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/evp.h> 
#include <mysql.h>
#include "admin_connexion.h"
#include "admin_panel.h"
#include "database.h"

void hash_password(const char* password, char* hash_output) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned int hash_len;

    md = EVP_get_digestbyname("SHA256");
    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, password, strlen(password));
    EVP_DigestFinal_ex(mdctx, hash, &hash_len);
    EVP_MD_CTX_free(mdctx);

    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hash_output + (i * 2), "%02x", hash[i]);
    }
    hash_output[64] = '\0'; 
}

static void lire_cle_publique_contenu_custom(const char* chemin, char* buffer, size_t maxsize) {
    FILE* f = fopen(chemin, "r");
    if(!f) { buffer[0] = '\0'; return; }
    
    size_t bytes_read = fread(buffer, 1, maxsize - 1, f);
    buffer[bytes_read] = '\0';
    for (size_t i = 0; i < bytes_read; ++i) {
        if (buffer[i] == '\n' || buffer[i] == '\r') buffer[i] = '\0';
    }
    fclose(f);
}

int menu_admin_connexion() {
    char email[100], mot_de_passe[100];
    printf("-- Connexion Admin --\n");
    printf("Email : ");
    scanf("%99s", email);
    printf("Mot de passe : ");
    scanf("%99s", mot_de_passe);

    char hash_saisi[65];
    hash_password(mot_de_passe, hash_saisi);

    MYSQL *conn = get_db_connection();
    if (!conn) {
        return 0;
    }

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
        
        if (strcmp(hash_saisi, hash_bdd) == 0 && admin == 1) {
            char cle_pub_b64[4096] = {0};
            lire_cle_publique_contenu_custom(".secrets/public_key.pem", cle_pub_b64, sizeof(cle_pub_b64));
            if (strlen(cle_pub_b64) > 1) { 
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
