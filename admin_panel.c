#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/core_names.h>
#include "admin_panel.h"
#include "database.h"

static void construire_base_nom(const char* nom_fichier, char* base_nom, size_t max) {
    strncpy(base_nom, nom_fichier, max-1);
    base_nom[max-1] = '\0';
    char *dot = strrchr(base_nom, '.');
    if(dot) *dot = '\0';
    const char *suffix = "_chiffre";
    size_t len_base = strlen(base_nom);
    size_t len_suf = strlen(suffix);
    if(len_base > len_suf && strcmp(base_nom + len_base - len_suf, suffix) == 0) {
        base_nom[len_base - len_suf] = '\0';
    }
}

static int evp_rsa_oaep_sha256_decrypt_pem(const char* pem_path, const unsigned char* inbuf, size_t inlen, unsigned char** outbuf, size_t* outlen) {
    int ok = 0;
    FILE* f = fopen(pem_path, "r");
    if(!f) return 0;
    EVP_PKEY* pkey = PEM_read_PrivateKey(f, NULL, NULL, NULL);
    fclose(f);
    if(!pkey) return 0;

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if(!ctx) goto cleanup;
    if (EVP_PKEY_decrypt_init(ctx) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) goto cleanup;

    size_t tmplen = 0;
    if (EVP_PKEY_decrypt(ctx, NULL, &tmplen, inbuf, inlen) <= 0) goto cleanup;
    unsigned char* out = (unsigned char*)malloc(tmplen);
    if (!out) goto cleanup;
    if (EVP_PKEY_decrypt(ctx, out, &tmplen, inbuf, inlen) <= 0) { free(out); goto cleanup; }

    *outbuf = out;
    *outlen = tmplen;
    ok = 1;
cleanup:
    if(ctx) EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return ok;
}

static int evp_rsa_oaep_sha256_encrypt_pem(const char* pub_pem_path, const unsigned char* inbuf, size_t inlen, unsigned char** outbuf, size_t* outlen) {
    int ok = 0;
    FILE* f = fopen(pub_pem_path, "r");
    if(!f) return 0;
    EVP_PKEY* pkey = PEM_read_PUBKEY(f, NULL, NULL, NULL);
    fclose(f);
    if(!pkey) return 0;
    size_t key_size = EVP_PKEY_size(pkey);
    size_t max_plain = key_size - 2*EVP_MD_size(EVP_sha256()) - 2;
    if (inlen > max_plain) { EVP_PKEY_free(pkey); return 0; }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if(!ctx) goto cleanup;
    if (EVP_PKEY_encrypt_init(ctx) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) goto cleanup;
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) goto cleanup;

    size_t tmplen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &tmplen, inbuf, inlen) <= 0) goto cleanup;
    unsigned char* out = (unsigned char*)malloc(tmplen);
    if (!out) goto cleanup;
    if (EVP_PKEY_encrypt(ctx, out, &tmplen, inbuf, inlen) <= 0) { free(out); goto cleanup; }

    *outbuf = out; *outlen = tmplen; ok = 1;
cleanup:
    if(ctx) EVP_PKEY_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    return ok;
}

static int ecrire_pem_pub_temp(const char* base64, const char* chemin) {
    FILE* f = fopen(chemin, "w");
    if(!f) return 0;
    fprintf(f, "-----BEGIN PUBLIC KEY-----\n");
    size_t n = strlen(base64);
    for(size_t i=0;i<n;i+=64) {
        size_t k = (i+64<=n)?64:(n-i);
        fwrite(base64+i, 1, k, f);
        fputc('\n', f);
    }
    fprintf(f, "-----END PUBLIC KEY-----\n");
    fclose(f);
    return 1;
}

static int ecrire_pem_pub_depuis_bdd(const char* valeur, const char* chemin) {
    // Si la valeur contient déjà un en-tête PEM, on l'écrit telle quelle
    if (valeur && strstr(valeur, "BEGIN PUBLIC KEY") != NULL) {
        FILE* f = fopen(chemin, "w");
        if(!f) return 0;
        fputs(valeur, f);
        // S'assure qu'il termine par \n
        size_t L = strlen(valeur);
        if (L==0 || valeur[L-1] != '\n') fputc('\n', f);
        fclose(f);
        return 1;
    }
    // Sinon on recompose un PEM valide à partir du base64 seul
    return ecrire_pem_pub_temp(valeur, chemin);
}

static void strip_ws(char* s) {
    size_t w=0; for(size_t r=0; s[r]; ++r){
        char c=s[r];
        if(c!='\n' && c!='\r' && c!=' ' && c!='\t') s[w++]=c;
    }
    s[w]=0;
}

void panel_admin() {
    int choix;
    
    while(1) {
        printf("\n========================================\n");
        printf("   PANEL ADMIN - Messagard\n");
        printf("========================================\n");
        printf("1 - Dashboard fichiers en attente\n");
        printf("2 - Telecharger et dechiffrer un fichier\n");
        printf("3 - Valider un fichier\n");
        printf("4 - Rejeter un fichier\n");
        printf("5 - Historique des messages\n");
        printf("0 - Deconnexion\n");
        printf("========================================\n");
        printf("Votre choix : ");
        scanf("%d", &choix);
        
        switch(choix) {
            case 1:
                dashboard_fichiers_attente();
                break;
            case 2:
                telecharger_et_dechiffrer_fichier();
                break;
            case 3:
                valider_fichier();
                break;
            case 4:
                rejeter_fichier();
                break;
            case 5:
                audit_messages();
                break;
            case 0:
                printf("Deconnexion...\n");
                return;
            default:
                printf("Choix invalide.\n");
        }
    }
}

void audit_messages() {
    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }

    // Requête pour récupérer tous les fichiers, tous statuts confondus
    char requete[500];
    sprintf(requete, "SELECT f.id, f.nom, u1.nom as nom_source, u2.nom as nom_dest, f.statut FROM fichier f LEFT JOIN utilisateur u1 ON f.id_utilisateur_source = u1.id LEFT JOIN utilisateur u2 ON f.id_utilisateur_destinataire = u2.id ORDER BY f.id DESC");

    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }

    MYSQL_RES *resultat = mysql_store_result(conn);
    if (!resultat) {
        printf("Erreur lors de la recuperation des resultats.\n");
        mysql_close(conn);
        return;
    }

    int num_rows = mysql_num_rows(resultat);

    printf("\n========================================\n");
    printf("   HISTORIQUE DES MESSAGES\n");
    printf("========================================\n");
    printf("Nombre total de transactions : %d\n", num_rows);
    printf("========================================\n");

    if (num_rows > 0) {
        // En-tête du tableau
        printf("%-5s | %-30s | %-15s | %-15s | %-15s\n", "ID", "Nom fichier", "Source", "Destinataire", "Statut");
        printf("----------------------------------------------------------------------------------------\n");

        MYSQL_ROW row;
        while ((row = mysql_fetch_row(resultat))) {
            printf("%-5s | %-30s | %-15s | %-15s | %-15s\n",
                   row[0] ? row[0] : "N/A",   // ID
                   row[1] ? row[1] : "N/A",   // Nom fichier
                   row[2] ? row[2] : "Inconnu", // Nom source
                   row[3] ? row[3] : "Inconnu", // Nom destinataire
                   row[4] ? row[4] : "N/A");  // Statut
        }
    }

    printf("========================================\n");

    mysql_free_result(resultat);
    mysql_close(conn);
}

void dashboard_fichiers_attente() {
    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }
    
    // Requete pour recuperer les fichiers en attente
    char requete[500];
    sprintf(requete, "SELECT f.id, f.nom, f.chemin, f.id_utilisateur_source, f.id_utilisateur_destinataire, f.statut, u1.nom as nom_source, u2.nom as nom_dest FROM fichier f LEFT JOIN utilisateur u1 ON f.id_utilisateur_source = u1.id LEFT JOIN utilisateur u2 ON f.id_utilisateur_destinataire = u2.id WHERE f.statut = 'en_attente' ORDER BY f.id");
    
    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    
    MYSQL_RES *resultat = mysql_store_result(conn);
    if (!resultat) {
        printf("Erreur lors de la recuperation des resultats.\n");
        mysql_close(conn);
        return;
    }
    
    int num_rows = mysql_num_rows(resultat);
    
    printf("\n========================================\n");
    printf("   DASHBOARD - Fichiers en attente\n");
    printf("========================================\n");
    printf("Nombre de fichiers en attente : %d\n", num_rows);
    printf("========================================\n");
    
    if (num_rows == 0) {
        printf("Aucun fichier en attente.\n");
    } else {
        // En-tete du tableau
        printf("%-5s | %-30s | %-15s | %-15s | %-15s\n", "ID", "Nom fichier", "Source", "Destinataire", "Statut");
        printf("----------------------------------------------------------------------------------------\n");
        
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(resultat))) {
            printf("%-5s | %-30s | %-15s | %-15s | %-15s\n", 
                   row[0] ? row[0] : "NULL",      // ID
                   row[1] ? row[1] : "NULL",      // Nom fichier
                   row[6] ? row[6] : "Inconnu",   // Nom source
                   row[7] ? row[7] : "Inconnu",   // Nom destinataire
                   row[5] ? row[5] : "NULL");     // Statut
        }
    }
    
    printf("========================================\n");
    
    mysql_free_result(resultat);
    mysql_close(conn);
}

void telecharger_et_dechiffrer_fichier() {
    int id_fichier;
    
    // Afficher d'abord la liste des fichiers en attente
    dashboard_fichiers_attente();
    
    printf("\nEntrez l'ID du fichier a telecharger et dechiffrer : ");
    scanf("%d", &id_fichier);
    
    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }
    
    // Recuperer les informations du fichier
    char requete[500];
    sprintf(requete, "SELECT id, nom, chemin FROM fichier WHERE id = %d AND statut = 'en_attente'", id_fichier);
    
    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    
    MYSQL_RES *resultat = mysql_store_result(conn);
    if (!resultat || mysql_num_rows(resultat) == 0) {
        printf("Fichier introuvable ou deja traite.\n");
        if(resultat) mysql_free_result(resultat);
        mysql_close(conn);
        return;
    }
    
    MYSQL_ROW row = mysql_fetch_row(resultat);
    char* nom_fichier = row[1];
    char* chemin_fichier_serveur = row[2];
    
    mysql_free_result(resultat);
    mysql_close(conn);
    
    // Telecharger le fichier depuis le serveur
    char nom_fichier_local[300];
    sprintf(nom_fichier_local, "temp_%d_%s", id_fichier, nom_fichier);
    
    // Configuration SSH (a adapter selon votre configuration)
    char user_ssh[100] = "adix";  // Utilisateur SSH sur le serveur
    char ip_serveur[50] = "192.168.86.128";  // IP du serveur
    
    printf("\nTelechargement du fichier depuis le serveur...\n");
    printf("Nom du fichier : %s\n", nom_fichier);
    
    // Essayer differentes commandes SCP selon l'environnement Windows
    char commande_scp[600];
    int result_scp = -1;
    
    // Methode 1 : Essayer avec le chemin complet Windows OpenSSH (prioritaire car on sait qu'il existe)
    // Note : BatchMode=no permet la saisie interactive du mot de passe
    sprintf(commande_scp, "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\" -o StrictHostKeyChecking=no %s@%s:%s %s", 
            user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
    // (suppression du message inutile sur la saisie du mot de passe)
    result_scp = system(commande_scp);
    
    // Methode 2 : Si le chemin complet echoue, essayer scp.exe (si dans PATH)
    if (result_scp != 0) {
        sprintf(commande_scp, "scp.exe -o StrictHostKeyChecking=no %s@%s:%s %s", 
                user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
        result_scp = system(commande_scp);
    }
    
    // Methode 3 : Essayer scp (sans .exe) si dans PATH
    if (result_scp != 0) {
        sprintf(commande_scp, "scp -o StrictHostKeyChecking=no %s@%s:%s %s", 
                user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
        result_scp = system(commande_scp);
    }
    
    if (result_scp != 0) {
        printf("\nErreur : Impossible de telecharger le fichier depuis le serveur.\n");
        printf("\nSolutions possibles :\n");
        printf("1. Activez OpenSSH Client sur Windows :\n");
        printf("   Parametres > Applications > Fonctionnalites optionnelles > Ajouter OpenSSH Client\n");
        printf("2. Ou installez Git Bash (qui inclut SCP)\n");
        printf("3. Ou copiez manuellement le fichier avec cette commande :\n");
        printf("   scp %s@%s:%s %s\n", user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
        printf("\nApres avoir copie le fichier, relancez cette fonction.\n");
        return;
    }
    
    printf("Fichier '%s' telecharge avec succes.\n", nom_fichier);
    
    // Lire le fichier chiffre local
    FILE *fichier_chiffre = fopen(nom_fichier_local, "rb");
    if (!fichier_chiffre) {
        printf("Erreur : Impossible d'ouvrir le fichier telecharge %s\n", nom_fichier_local);
        return;
    }
    fseek(fichier_chiffre, 0, SEEK_END);
    long taille_fichier = ftell(fichier_chiffre);
    fseek(fichier_chiffre, 0, SEEK_SET);
    unsigned char *contenu_chiffre = (unsigned char*)malloc(taille_fichier);
    if (!contenu_chiffre) { printf("Erreur : Memoire insuffisante.\n"); fclose(fichier_chiffre); return; }
    fread(contenu_chiffre, 1, taille_fichier, fichier_chiffre);
    fclose(fichier_chiffre);

    // Verifier la taille attendue vs taille de cle
    FILE* fkeysz = fopen(".secrets/private_key.pem", "r");
    if(!fkeysz){ printf("Cle privee introuvable (.secrets/private_key.pem).\n"); free(contenu_chiffre); return; }
    EVP_PKEY* pkeysz = PEM_read_PrivateKey(fkeysz, NULL, NULL, NULL); fclose(fkeysz);
    if(!pkeysz){ printf("Cle privee invalide.\n"); free(contenu_chiffre); return; }
    size_t rsa_block = EVP_PKEY_size(pkeysz);
    EVP_PKEY_free(pkeysz);
    if ((size_t)taille_fichier != rsa_block) {
        printf("Fichier binaire inattendu (%ld octets). Attendu: %zu octets pour RSA.\n", taille_fichier, rsa_block);
        printf("Regenerez le .bin avec l'utilitaire OAEP-SHA256 et la cle publique correspondante.\n");
        free(contenu_chiffre);
        return;
    }

    // Dechiffrement OAEP-SHA256 (une seule brique RSA attendue)
    unsigned char* contenu_dechiffre = NULL; size_t taille_dechiffree = 0;
    if (!evp_rsa_oaep_sha256_decrypt_pem(".secrets/private_key.pem", contenu_chiffre, (size_t)taille_fichier, &contenu_dechiffre, &taille_dechiffree)) {
        printf("Erreur lors du dechiffrement (OAEP-SHA256). Details OpenSSL:\n");
        ERR_print_errors_fp(stderr);
        free(contenu_chiffre);
        return;
    }
    free(contenu_chiffre);

    // Sauvegarder le fichier dechiffre en .txt lisible
    char base_nom[256];
    construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
    char nom_fichier_dechiffre[300];
    snprintf(nom_fichier_dechiffre, sizeof(nom_fichier_dechiffre), "dechiffre_%s.txt", base_nom);
    FILE *fichier_dechiffre = fopen(nom_fichier_dechiffre, "wb");
    if (!fichier_dechiffre) { printf("Erreur : Impossible de creer le fichier dechiffre.\n"); free(contenu_dechiffre); return; }
    fwrite(contenu_dechiffre, 1, taille_dechiffree, fichier_dechiffre);
    fclose(fichier_dechiffre);
    free(contenu_dechiffre);
    remove(nom_fichier_local);
    printf("\nFichier dechiffre avec succes !\n");
    printf("Fichier sauvegarde sous : %s\n", nom_fichier_dechiffre);
}

void valider_fichier() {
    int id_fichier;
    printf("\nEntrez l'ID du fichier a valider : ");
    scanf("%d", &id_fichier);

    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }

    // Recuperer nom + destinataire + cle publique destinataire
    char requete[600];
    sprintf(requete, "SELECT f.nom, f.id_utilisateur_destinataire, u.cle_publique FROM fichier f JOIN utilisateur u ON u.id=f.id_utilisateur_destinataire WHERE f.id=%d", id_fichier);
    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn);
        return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if(!res || mysql_num_rows(res)==0) {
        printf("Fichier introuvable.\n");
        if(res) mysql_free_result(res);
        mysql_close(conn);
        return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    const char* nom_fichier = row[0];
    const char* cle_pub_b64 = row[2];

    if(!cle_pub_b64 || strlen(cle_pub_b64)==0) {
        printf("Le destinataire n'a pas de cle publique en BDD.\n");
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }

    // Copie et assainit la base64 (supprime espaces/retours)
    char cle_copy[8192];
    strncpy(cle_copy, cle_pub_b64, sizeof(cle_copy)-1);
    cle_copy[sizeof(cle_copy)-1]='\0';
    strip_ws(cle_copy);

    // Construire nom du fichier clair deja cree
    char base_nom[256];
    construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
    char nom_fichier_dechiffre[300];
    snprintf(nom_fichier_dechiffre, sizeof(nom_fichier_dechiffre), "dechiffre_%s.txt", base_nom);

    // Lire clair
    FILE* fin = fopen(nom_fichier_dechiffre, "rb");
    if(!fin) {
        printf("Fichier clair introuvable (%s). Veuillez d'abord le telecharger et le dechiffrer.\n", nom_fichier_dechiffre);
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }
    fseek(fin, 0, SEEK_END); long sz = ftell(fin); fseek(fin, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(sz);
    fread(buf,1,sz,fin); fclose(fin);

    // Ecrire la cle publique destinataire en PEM temporaire
    char chemin_pub_tmp[64] = ".secrets/tmp_dest_pub.pem";
    if (!ecrire_pem_pub_depuis_bdd(cle_copy, chemin_pub_tmp)) {
        printf("Impossible d'ecrire la cle publique destinataire.\n");
        free(buf); mysql_free_result(res); mysql_close(conn); return;
    }

    // Verifier limite OAEP pour cette cle
    FILE* fpubsz = fopen(chemin_pub_tmp, "r");
    if(!fpubsz){ printf("Cle publique destinataire inaccessible.\n"); free(buf); remove(chemin_pub_tmp); mysql_free_result(res); mysql_close(conn); return; }
    EVP_PKEY* pksz = PEM_read_PUBKEY(fpubsz, NULL, NULL, NULL); fclose(fpubsz);
    if(!pksz){ printf("Cle publique destinataire invalide (PEM).\n"); free(buf); remove(chemin_pub_tmp); mysql_free_result(res); mysql_close(conn); return; }
    size_t key_size = EVP_PKEY_size(pksz);
    size_t max_plain = key_size - 2*EVP_MD_size(EVP_sha256()) - 2;
    EVP_PKEY_free(pksz);
    if((size_t)sz > max_plain){
        printf("Contenu trop grand pour RSA-OAEP (%ld octets > max %zu). Utilisez un fichier plus petit ou un chiffrement hybride.\n", sz, max_plain);
        remove(chemin_pub_tmp); free(buf); mysql_free_result(res); mysql_close(conn); return;
    }

    // Chiffrement OAEP-SHA256 (fichier court)
    unsigned char* out = NULL; size_t outlen = 0;
    if (!evp_rsa_oaep_sha256_encrypt_pem(chemin_pub_tmp, buf, (size_t)sz, &out, &outlen)) {
        printf("Echec chiffrement pour destinataire (OAEP-SHA256). Details OpenSSL:\n");
        ERR_print_errors_fp(stderr);
        remove(chemin_pub_tmp);
        free(buf); mysql_free_result(res); mysql_close(conn); return;
    }
    remove(chemin_pub_tmp);

    char nom_sortie[320];
    snprintf(nom_sortie, sizeof(nom_sortie), "pour_dest_%s.bin", base_nom);
    FILE* fout = fopen(nom_sortie, "wb"); fwrite(out,1,outlen,fout); fclose(fout);
    free(buf); free(out);

    // Mettre a jour le statut
    sprintf(requete, "UPDATE fichier SET statut='valide' WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) printf("Statut non mis a jour : %s\n", mysql_error(conn));
    else printf("Fichier valide. Chiffre pour le destinataire sous : %s\n", nom_sortie);

    mysql_free_result(res);
    mysql_close(conn);

    // Upload du .bin vers le serveur puis suppression locale
    {
        const char* user_ssh = "adix";
        const char* ip_serveur = "192.168.86.128";
        const char* dossier_dist = "/home/adix/fichiers"; // sans / final pour construire proprement
        char distant_dir[512];
        snprintf(distant_dir, sizeof(distant_dir), "%s", dossier_dist);
        char distant_file[600];
        snprintf(distant_file, sizeof(distant_file), "%s/%s", dossier_dist, nom_sortie);

        // 0) S'assurer que le dossier distant existe (ssh mkdir -p)
        {
            char cmd_ssh[800]; int rssh = -1;
            snprintf(cmd_ssh, sizeof(cmd_ssh), "\"C:\\Windows\\System32\\OpenSSH\\ssh.exe\" %s@%s \"mkdir -p %s\"", user_ssh, ip_serveur, distant_dir);
            rssh = system(cmd_ssh);
            if (rssh != 0) {
                snprintf(cmd_ssh, sizeof(cmd_ssh), "ssh %s@%s \"mkdir -p %s\"", user_ssh, ip_serveur, distant_dir);
                rssh = system(cmd_ssh);
            }
            if (rssh != 0) {
                printf("Avertissement: creation du dossier distant %s a echoue.\n", distant_dir);
            }
        }

        // 1) scp.exe chemin complet (quotes corrigees)
        char commande_scp[900];
        int resscp = -1;
        snprintf(commande_scp, sizeof(commande_scp), "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\" -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
        resscp = system(commande_scp);

        // 2) scp.exe dans PATH
        if(resscp != 0) {
            snprintf(commande_scp, sizeof(commande_scp), "scp.exe -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
            resscp = system(commande_scp);
        }
        // 3) scp sans .exe
        if(resscp != 0) {
            snprintf(commande_scp, sizeof(commande_scp), "scp -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
            resscp = system(commande_scp);
        }

        if(resscp == 0) {
            // ARCHIVAGE (F-A6) : On déplace le fichier original chiffré pour l'admin
            // au lieu de le supprimer.
            char cmd_ssh_archive[1200];
            char chemin_original_distant[512];
            MYSQL* conn_archive = get_db_connection();
            if(conn_archive) {
            sprintf(cmd_ssh_archive, "SELECT chemin FROM fichier WHERE id=%d", id_fichier);
            if (mysql_query(conn_archive, cmd_ssh_archive) == 0) {
                MYSQL_RES* res_archive = mysql_store_result(conn_archive);
                MYSQL_ROW row_archive = mysql_fetch_row(res_archive);
                if (row_archive && row_archive[0]) {
                    snprintf(chemin_original_distant, sizeof(chemin_original_distant), "%s", row_archive[0]);
                    // Commande pour créer le dossier d'archives et y déplacer le fichier
                    snprintf(cmd_ssh_archive, sizeof(cmd_ssh_archive), "ssh %s@%s \"mkdir -p /home/adix/archives && mv %s /home/adix/archives/\"", user_ssh, ip_serveur, chemin_original_distant);
                    system(cmd_ssh_archive);
                }
                mysql_free_result(res_archive);
            }
            mysql_close(conn_archive);
            }

            // suppression locale du .bin
            remove(nom_sortie);
            // suppression locale du clair
            remove(nom_fichier_dechiffre);
        } else {
            printf("Avertissement: envoi SCP echoue. Conservez localement %s.\n", nom_sortie);
            printf("Vous pouvez l'envoyer manuellement avec: scp \"%s\" %s@%s:\"%s\"\n", nom_sortie, user_ssh, ip_serveur, distant_file);
        }
    }
}

void rejeter_fichier() {
    int id_fichier;
    printf("\nEntrez l'ID du fichier a rejeter : ");
    scanf("%d", &id_fichier);

    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }

    // Recuperer infos fichier (nom et chemin)
    char requete[600];
    sprintf(requete, "SELECT nom, chemin FROM fichier WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn); return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if(!res || mysql_num_rows(res)==0) {
        printf("Fichier introuvable.\n");
        if(res) mysql_free_result(res); mysql_close(conn); return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    const char* nom_fichier = row[0];
    const char* chemin_distant = row[1];

    // Mettre statut = rejete
    sprintf(requete, "UPDATE fichier SET statut='rejete' WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) {
        printf("Impossible de mettre a jour le statut : %s\n", mysql_error(conn));
    }

    mysql_free_result(res);
    mysql_close(conn);

    // Nettoyer fichiers locaux potentiels
    if (nom_fichier && nom_fichier[0]) {
        char base_nom[256];
        construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
        char chemin_local[320];
        // temp_*
        snprintf(chemin_local, sizeof(chemin_local), "temp_%d_%s", id_fichier, nom_fichier);
        remove(chemin_local);
        // dechiffre_*.txt
        snprintf(chemin_local, sizeof(chemin_local), "dechiffre_%s.txt", base_nom);
        remove(chemin_local);
        // pour_dest_*.bin
        snprintf(chemin_local, sizeof(chemin_local), "pour_dest_%s.bin", base_nom);
        remove(chemin_local);
    }

    // Supprimer le fichier distant via SSH
    if (chemin_distant && chemin_distant[0]) {
        const char* user_ssh = "adix";
        const char* ip_serveur = "192.168.86.128";
        char cmd_ssh[900]; int rssh = -1;
        // ssh "rm -f <chemin>"
        snprintf(cmd_ssh, sizeof(cmd_ssh), "\"C:\\Windows\\System32\\OpenSSH\\ssh.exe\" %s@%s \"rm -f %s\"", user_ssh, ip_serveur, chemin_distant);
        rssh = system(cmd_ssh);
        if (rssh != 0) {
            snprintf(cmd_ssh, sizeof(cmd_ssh), "ssh %s@%s \"rm -f %s\"", user_ssh, ip_serveur, chemin_distant);
            rssh = system(cmd_ssh);
        }
        // pas de message intrusif si ok; on reste silencieux
    }
}
