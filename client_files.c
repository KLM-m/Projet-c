#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql.h>
#include <unistd.h>   // Pour remove()
#include "client_files.h" 
#include "database.h"     
#include "custom_rsa.h"   

// NOTE: Les constantes #define ont été REMPLACÉES par des variables locales 
// dans la fonction moteur_envoi_fichier_securise, sauf pour le chemin distant.
// Cependant, il est plus sûr d'utiliser une variable locale pour le chemin distant aussi.
// Je retire les #define du code précédent.

int moteur_envoi_fichier_securise(SessionClient* session, char* chemin_source, int id_destinataire) {
    
    // --- NOUVELLES VARIABLES DE CONNEXION LOCALES ---
    char user_ssh[100] = "adix"; // Utilisateur SSH
    char ip_serveur[50] = "192.168.86.128"; // Adresse IP du serveur
    // Chemin cible sur la VM Linux (doit exister et avoir les droits d'écriture pour l'utilisateur adix)
    const char* chemin_distant_linux = "/home/adix/messages/"; 
    // --------------------------------------------------
    
    MYSQL *conn = get_db_connection();
    if (!conn) return 0;
    
    // 1. Récupération Clé Admin
    if(mysql_query(conn, "SELECT cle_publique FROM utilisateur WHERE admin=1 LIMIT 1")) {
        mysql_close(conn); return 0;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if(!res || mysql_num_rows(res) == 0) {
        printf("Erreur : Clé Admin introuvable.\n");
        mysql_free_result(res);
        mysql_close(conn); return 0;
    }
    CustomRsaPublicKey pub;
    if (sscanf(mysql_fetch_row(res)[0], "%llu,%llu", &pub.e, &pub.n) != 2) {
        printf("Erreur : Format de clé publique invalide.\n");
        mysql_free_result(res);
        mysql_close(conn); return 0;
    }
    mysql_free_result(res);
    
    // 2. Chiffrement RSA
    char nom_bin[300];
    sprintf(nom_bin, "msg_data_%d_%lld.bin", session->id_utilisateur, (long long)time(NULL)); 
    
    printf("   [Securité] Chiffrement RSA en cours...\n");
    if(!custom_rsa_chiffrer_fichier(chemin_source, nom_bin, pub)) {
        printf("   [Erreur] Echec du chiffrement.\n");
        mysql_close(conn); return 0;
    }
    
    // --- ÉTAPE DE TRANSFERT RÉSEAU SCP ---
    
    char commande_scp[600];
    int result_scp = -1;

    printf("   [Réseau] Tentative de transfert vers %s@%s:%s en cours...\n", user_ssh, ip_serveur, chemin_distant_linux);
    
    // Tentative 1: Chemin absolu OpenSSH Windows
    sprintf(commande_scp, "\"C:\\Windows\\System32\\OpenSSH\\scp.exe\" -o StrictHostKeyChecking=no %s %s@%s:%s", 
            nom_bin, user_ssh, ip_serveur, chemin_distant_linux);
    result_scp = system(commande_scp);

    // Tentative 2: scp.exe dans le PATH
    if (result_scp != 0) {
        sprintf(commande_scp, "scp.exe -o StrictHostKeyChecking=no %s %s@%s:%s", 
                nom_bin, user_ssh, ip_serveur, chemin_distant_linux);
        result_scp = system(commande_scp);
    }

    // Tentative 3: scp dans le PATH (pour Git Bash/Cygwin)
    if (result_scp != 0) {
        sprintf(commande_scp, "scp -o StrictHostKeyChecking=no %s %s@%s:%s", 
                nom_bin, user_ssh, ip_serveur, chemin_distant_linux);
        result_scp = system(commande_scp);
    }
    
    // Vérification finale du résultat SCP
    if (result_scp != 0) {
        printf("\n   [Erreur] Echec du transfert sécurisé (SCP). L'échec est souvent dû à :\n");
        printf("   1. L'authentification par mot de passe (clés SSH recommandées pour system()).\n");
        printf("   2. L'outil SCP non trouvé (vérifiez le PATH).\n");
        printf("   3. Permissions d'écriture insuffisantes sur la VM Linux (chemin : %s).\n", chemin_distant_linux);
        remove(nom_bin); // Nettoyage local
        mysql_close(conn);
        return 0;
    }
    
    printf("   [Réseau] Transfert réussi.\n");

    // Nettoyage du fichier chiffré local
    remove(nom_bin); 
    
    // 3. Insertion BDD
    char chemin_cible_bdd[300];
    // Le chemin inséré en BDD est le chemin LINUX où le fichier a été envoyé.
    sprintf(chemin_cible_bdd, "%s%s", chemin_distant_linux, nom_bin); 
    
    char req[1000];
    sprintf(req, "INSERT INTO fichier (nom, chemin, id_utilisateur_source, id_utilisateur_destinataire, statut) VALUES ('%s', '%s', %d, %d, 'en_attente')", nom_bin, chemin_cible_bdd, session->id_utilisateur, id_destinataire);
    
    if(mysql_query(conn, req)) {
        printf("   [Erreur] SQL : %s\n", mysql_error(conn));
        mysql_close(conn); 
        return 0;
    }
    
    printf("   [Succès] Données sécurisées envoyées et BDD mise à jour.\n");
    mysql_close(conn);
    return 1;
}