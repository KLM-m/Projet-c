#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "admin_panel.h"
#include "database.h"
#include "custom_rsa.h" 

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

static int charger_cle_privee_custom(CustomRsaPrivateKey* priv, const char* chemin_priv) {
    FILE* f = fopen(chemin_priv, "r");
    if (!f) return 0;
    if (fscanf(f, "%llu,%llu", &priv->d, &priv->n) != 2) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int charger_cle_publique_custom_depuis_str(CustomRsaPublicKey* pub, const char* cle_str) {
    if (sscanf(cle_str, "%llu,%llu", &pub->e, &pub->n) != 2) {
        return 0;
    }
    return 1;
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
    
    dashboard_fichiers_attente();
    
    printf("\nEntrez l'ID du fichier a telecharger et dechiffrer : ");
    scanf("%d", &id_fichier);
    
    MYSQL *conn = get_db_connection();
    if (!conn) {
        return;
    }
    
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
    
    char nom_fichier_local[300];
    sprintf(nom_fichier_local, "temp_%d_%s", id_fichier, nom_fichier);
    
    char user_ssh[100] = "adix";  
    char ip_serveur[50] = "192.168.86.128";  
    
    printf("\nTelechargement du fichier depuis le serveur...\n");
    printf("Nom du fichier : %s\n", nom_fichier);
    
    char commande_scp[600];
    int result_scp = -1;
    
    sprintf(commande_scp, "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\" -o StrictHostKeyChecking=no %s@%s:%s %s", 
            user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
    result_scp = system(commande_scp);
    
    if (result_scp != 0) {
        sprintf(commande_scp, "scp.exe -o StrictHostKeyChecking=no %s@%s:%s %s", 
                user_ssh, ip_serveur, chemin_fichier_serveur, nom_fichier_local);
        result_scp = system(commande_scp);
    }
    
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
    
    CustomRsaPrivateKey cle_privee_admin;
    if (!charger_cle_privee_custom(&cle_privee_admin, ".secrets/private_key.pem")) {
        printf("Erreur: Impossible de charger la clé privée locale.\n");
        remove(nom_fichier_local);
        return;
    }

    char base_nom[256];
    construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
    char nom_fichier_dechiffre[300];
    snprintf(nom_fichier_dechiffre, sizeof(nom_fichier_dechiffre), "dechiffre_%s.txt", base_nom);

    if (!custom_rsa_dechiffrer_fichier(nom_fichier_local, nom_fichier_dechiffre, cle_privee_admin)) {
        printf("Erreur lors du déchiffrement du fichier.\n");
        remove(nom_fichier_local);
        return;
    }

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

    char base_nom[256];
    construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
    char nom_fichier_dechiffre[300];
    snprintf(nom_fichier_dechiffre, sizeof(nom_fichier_dechiffre), "dechiffre_%s.txt", base_nom);

    FILE* fin = fopen(nom_fichier_dechiffre, "rb");
    if(!fin) {
        printf("Fichier clair introuvable (%s). Veuillez d'abord le telecharger et le dechiffrer.\n", nom_fichier_dechiffre);
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }
    fclose(fin); 

    CustomRsaPublicKey cle_pub_dest;
    if (!charger_cle_publique_custom_depuis_str(&cle_pub_dest, cle_pub_b64)) {
        printf("La clé publique du destinataire est invalide.\n");
        mysql_free_result(res); mysql_close(conn); return;
    }

    char nom_sortie[320];
    snprintf(nom_sortie, sizeof(nom_sortie), "pour_dest_%s.bin", base_nom);

    if (!custom_rsa_chiffrer_fichier(nom_fichier_dechiffre, nom_sortie, cle_pub_dest)) {
        printf("Erreur lors du chiffrement pour le destinataire.\n");
        mysql_free_result(res); mysql_close(conn); return;
    }

    sprintf(requete, "UPDATE fichier SET statut='valide' WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) printf("Statut non mis a jour : %s\n", mysql_error(conn));
    else printf("Fichier valide. Chiffre pour le destinataire sous : %s\n", nom_sortie);

    mysql_free_result(res);
    mysql_close(conn);

    {
        const char* user_ssh = "adix";
        const char* ip_serveur = "192.168.86.128";
        const char* dossier_dist = "/home/adix/fichiers"; 
        char distant_dir[512];
        snprintf(distant_dir, sizeof(distant_dir), "%s", dossier_dist);
        char distant_file[600];
        snprintf(distant_file, sizeof(distant_file), "%s/%s", dossier_dist, nom_sortie);

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

        char commande_scp[1200]; 
        int resscp = -1;
        snprintf(commande_scp, sizeof(commande_scp), "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\" -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
        resscp = system(commande_scp);

        if(resscp != 0) {
            snprintf(commande_scp, sizeof(commande_scp), "scp.exe -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
            resscp = system(commande_scp);
        }
        if(resscp != 0) {
            snprintf(commande_scp, sizeof(commande_scp), "scp -o StrictHostKeyChecking=no \"%s\" %s@%s:\"%s\"", nom_sortie, user_ssh, ip_serveur, distant_file);
            resscp = system(commande_scp);
        }

        if(resscp == 0) {
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
                    snprintf(cmd_ssh_archive, sizeof(cmd_ssh_archive), "ssh %s@%s \"mkdir -p /home/adix/archives && mv %s /home/adix/archives/\"", user_ssh, ip_serveur, chemin_original_distant);
                    system(cmd_ssh_archive);
                }
                mysql_free_result(res_archive);
            }
            mysql_close(conn_archive);
            }

            remove(nom_sortie);
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

    char requete[600];
    sprintf(requete, "SELECT nom, chemin FROM fichier WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) {
        printf("Erreur requete : %s\n", mysql_error(conn));
        mysql_close(conn); return;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if(!res || mysql_num_rows(res)==0) {
        printf("Fichier introuvable.\n");
        if(res) {
            mysql_free_result(res);
        }
        mysql_close(conn); return;
    }
    MYSQL_ROW row = mysql_fetch_row(res);
    const char* nom_fichier = row[0];
    const char* chemin_distant = row[1];

    if (nom_fichier && nom_fichier[0]) {
        char base_nom[256];
        construire_base_nom(nom_fichier, base_nom, sizeof(base_nom));
        char chemin_local[320];
        snprintf(chemin_local, sizeof(chemin_local), "temp_%d_%s", id_fichier, nom_fichier);
        remove(chemin_local);
        snprintf(chemin_local, sizeof(chemin_local), "dechiffre_%s.txt", base_nom);
        remove(chemin_local);
        snprintf(chemin_local, sizeof(chemin_local), "pour_dest_%s.bin", base_nom);
        remove(chemin_local);
    }

    int suppression_serveur_reussie = 0;
    if (chemin_distant && chemin_distant[0]) {
        const char* user_ssh = "adix";
        const char* ip_serveur = "192.168.86.128";
        char cmd_ssh[900]; int rssh = -1;
        snprintf(cmd_ssh, sizeof(cmd_ssh), "\"C:\\Windows\\System32\\OpenSSH\\ssh.exe\" %s@%s \"rm -f %s\"", user_ssh, ip_serveur, chemin_distant);
        rssh = system(cmd_ssh);
        if (rssh != 0) {
            snprintf(cmd_ssh, sizeof(cmd_ssh), "ssh %s@%s \"rm -f %s\"", user_ssh, ip_serveur, chemin_distant);
            rssh = system(cmd_ssh);
        }

        if (rssh == 0) {
            printf("Fichier supprime avec succes du serveur.\n");
            suppression_serveur_reussie = 1;
        } else {
            printf("Avertissement : Echec de la suppression du fichier sur le serveur.\n");
        }
    }

    sprintf(requete, "UPDATE fichier SET statut='rejete' WHERE id=%d", id_fichier);
    if(mysql_query(conn, requete)) {
        printf("Impossible de mettre a jour le statut : %s\n", mysql_error(conn));
    } else if (suppression_serveur_reussie) {
        printf("Le fichier ID %d a ete marque comme 'rejete'.\n", id_fichier);
    }

    mysql_free_result(res);
    mysql_close(conn);
}
