#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include "client_auth.h"
#include "database.h"
#include "custom_rsa.h"

int creer_compte_client() {
    char nom[50], email[100], mot_de_passe[100], confirmation[100];
    
    printf("\n=== INSCRIPTION ===\n");
    printf("Nom : "); scanf("%49s", nom);
    printf("Email : "); scanf("%99s", email);
    printf("Mot de passe : "); scanf("%99s", mot_de_passe);
    printf("Confirmer : "); scanf("%99s", confirmation);
    
    if (strcmp(mot_de_passe, confirmation) != 0) {
        printf("Les mots de passe ne correspondent pas.\n");
        return 0;
    }
    
    // Salage et Hachage
    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    char salt_hex[33];
    for(int i=0; i<16; i++) sprintf(salt_hex + (i*2), "%02x", salt[i]);
    
    char mdp_salt[200];
    snprintf(mdp_salt, sizeof(mdp_salt), "%s%s", mot_de_passe, salt_hex);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)mdp_salt, strlen(mdp_salt), hash);
    char hash_hex[65];
    for(int i=0; i<SHA256_DIGEST_LENGTH; i++) sprintf(hash_hex + (i*2), "%02x", hash[i]);
    
    // Génération RSA
    char chemin_pub[200], chemin_priv[200];
    snprintf(chemin_pub, sizeof(chemin_pub), ".secrets/%s_public_key.pem", nom);
    snprintf(chemin_priv, sizeof(chemin_priv), ".secrets/%s_private_key.pem", nom);
    
    if (!custom_rsa_generer_et_sauvegarder_cles(chemin_pub, chemin_priv)) {
        printf("Erreur lors de la génération des clés RSA.\n");
        return 0;
    }
    
    // Stockage BDD
    FILE* f_pub = fopen(chemin_pub, "r");
    char cle_pub_str[4096];
    size_t n = fread(cle_pub_str, 1, 4095, f_pub);
    cle_pub_str[n] = 0;
    fclose(f_pub);
    
    MYSQL *conn = get_db_connection();
    if (!conn) return 0;
    
    char req[5000];
    sprintf(req, "INSERT INTO utilisateur (nom, email, mot_de_passe, admin, salt, cle_publique) VALUES ('%s', '%s', '%s', 0, '%s', '%s')", nom, email, hash_hex, salt_hex, cle_pub_str);
    
    if(mysql_query(conn, req)) {
        printf("Erreur BDD: %s\n", mysql_error(conn));
        mysql_close(conn);
        return 0;
    }
    
    printf("Compte créé avec succès.\n");
    mysql_close(conn);
    return 1;
}

SessionClient* connexion_client() {
    char email[100], mot_de_passe[100];
    printf("\n=== CONNEXION ===\n");
    printf("Email : "); scanf("%99s", email);
    printf("Mot de passe : "); scanf("%99s", mot_de_passe);
    
    MYSQL *conn = get_db_connection();
    if (!conn) return NULL;
    
    char req[300];
    sprintf(req, "SELECT id, nom, mot_de_passe, salt FROM utilisateur WHERE email='%s'", email);
    mysql_query(conn, req);
    MYSQL_RES *res = mysql_store_result(conn);
    
    if (!res || mysql_num_rows(res) == 0) {
        printf("Identifiants incorrects.\n");
        if(res) mysql_free_result(res);
        mysql_close(conn);
        return NULL;
    }
    
    MYSQL_ROW row = mysql_fetch_row(res);
    char* hash_bdd = row[2];
    char* salt = row[3];
    
    char mdp_salt[200];
    snprintf(mdp_salt, sizeof(mdp_salt), "%s%s", mot_de_passe, salt);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)mdp_salt, strlen(mdp_salt), hash);
    char hash_calc[65];
    for(int i=0; i<SHA256_DIGEST_LENGTH; i++) sprintf(hash_calc + (i*2), "%02x", hash[i]);
    
    if (strcmp(hash_calc, hash_bdd) != 0) {
        printf("Identifiants incorrects.\n");
        mysql_free_result(res);
        mysql_close(conn);
        return NULL;
    }
    
    SessionClient* s = malloc(sizeof(SessionClient));
    s->id_utilisateur = atoi(row[0]);
    strcpy(s->nom, row[1]);
    strcpy(s->email, email);
    
    printf("Connexion réussie !\n");
    mysql_free_result(res);
    mysql_close(conn);
    return s;
}