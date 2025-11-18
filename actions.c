#include "actions.h"
#include "crypto.h"
#include <time.h>

int get_admin_id(MYSQL *conn) {
    mysql_query(conn, "SELECT id FROM utilisateur WHERE admin=1 LIMIT 1");
    MYSQL_RES *res = mysql_store_result(conn);
    if (!res || mysql_num_rows(res) == 0) return -1;
    MYSQL_ROW row = mysql_fetch_row(res);
    int id = atoi(row[0]);
    mysql_free_result(res);
    return id;
}

void envoyer_fichier(MYSQL *conn, int mon_id, int is_message) {
    int dest_id;
    char chemin_source[260], nom_fichier[100];
    
    mysql_query(conn, "SELECT id, nom FROM utilisateur");
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row;
    printf("\n--- UTILISATEURS ---\n");
    while ((row = mysql_fetch_row(res))) { 
        if(atoi(row[0]) != mon_id) printf("ID [%s] : %s\n", row[0], row[1]); 
    }
    printf("Destinataire ID : "); scanf("%d", &dest_id);
    
    if (is_message) {
        printf("Message : ");
        char msg[1024]; getchar(); fgets(msg, 1024, stdin);
        msg[strcspn(msg, "\n")] = 0;
        sprintf(chemin_source, "temp_msg.txt");
        FILE *f = fopen(chemin_source, "w"); fprintf(f, "%s", msg); fclose(f);
        strcpy(nom_fichier, "message.txt");
    } else {
        printf("Fichier local : "); scanf("%s", chemin_source);
        strcpy(nom_fichier, chemin_source);
    }

    char chemin_serveur[300];
    sprintf(chemin_serveur, "%s/%ld_%d.enc", FILES_DIR, time(NULL), mon_id);
    
    int admin_id = get_admin_id(conn);
    if (admin_id == -1) { printf("Erreur: Pas d'admin.\n"); return; }

    printf("Chiffrement en cours...\n");
    if (chiffrer_et_sauvegarder(chemin_source, chemin_serveur, dest_id, admin_id)) {
        char q[1024];
        sprintf(q, "INSERT INTO fichier (nom, chemin, id_utilisateur_source, id_utilisateur_destinataire, statut) VALUES ('%s', '%s', %d, %d, 'en_attente')",
            nom_fichier, chemin_serveur, mon_id, dest_id);
        
        if (!mysql_query(conn, q)) printf("✅ Envoyé ! (En attente validation Admin)\n");
        else printf("❌ Erreur SQL : %s\n", mysql_error(conn));
    } else {
        printf("❌ Echec chiffrement. Clés manquantes ?\n");
    }
    if (is_message) remove("temp_msg.txt");
}

void verifier_reception(MYSQL *conn, int mon_id) {
    char q[512];
    sprintf(q, "SELECT id, nom, chemin FROM fichier WHERE id_utilisateur_destinataire=%d AND statut='valide'", mon_id);
    mysql_query(conn, q);
    MYSQL_RES *res = mysql_store_result(conn);
    
    if (!res || mysql_num_rows(res) == 0) {
        printf("Aucun nouveau fichier validé.\n");
        if(res) mysql_free_result(res);
        return;
    }

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(res))) {
        printf("📥 Réception : %s ...\n", row[1]);
        char out[300];
        sprintf(out, "recu_%s_%s", row[0], row[1]);
        dechiffrer_fichier(row[2], out);
        
        char update[256];
        sprintf(update, "UPDATE fichier SET statut='telecharge' WHERE id=%s", row[0]);
        mysql_query(conn, update);
    }
    mysql_free_result(res);
}