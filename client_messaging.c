#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "client_messaging.h"
#include "client_files.h" // Utilise le moteur
#include "database.h"
#include "custom_rsa.h"

void envoyer_message_texte(SessionClient* session) {
    int id;
    char msg[1000];
    
    printf("\n=== NOUVEAU MESSAGE ===\n");
    printf("ID Destinataire : "); scanf("%d", &id); getchar();
    printf("Message : "); fgets(msg, 1000, stdin);
    msg[strcspn(msg, "\n")] = 0;
    
    // 1. Création fichier temporaire
    char tmp[50]; sprintf(tmp, "tmp_msg_%d.txt", session->id_utilisateur);
    FILE* f = fopen(tmp, "w");
    if (!f) return;
    fprintf(f, "%s", msg);
    fclose(f);
    
    // 2. Appel du moteur (Fichier -> RSA -> BDD)
    if(moteur_envoi_fichier_securise(session, tmp, id)) {
        printf("Message envoyé.\n");
    }
    
    // 3. Nettoyage
    remove(tmp);
}

void recevoir_messages_texte(SessionClient* session) {
    MYSQL *conn = get_db_connection();
    if (!conn) return;
    
    char req[600];
    sprintf(req, "SELECT f.id, f.nom, u.nom FROM fichier f JOIN utilisateur u ON u.id=f.id_utilisateur_source WHERE f.id_utilisateur_destinataire=%d AND f.statut='valide'", session->id_utilisateur);
    mysql_query(conn, req);
    MYSQL_RES *res = mysql_store_result(conn);
    
    printf("\n=== MESSAGES RECUS ===\n");
    if(!res || mysql_num_rows(res)==0) {
        printf("Aucun message.\n");
    } else {
        MYSQL_ROW row;
        while((row = mysql_fetch_row(res))) {
            printf("[%s] De: %s | Données chiffrées: %s\n", row[0], row[2], row[1]);
        }
        
        int id;
        printf("\nID à déchiffrer (0 pour retour) : ");
        scanf("%d", &id);
        if(id > 0) {
            printf("Simulation: Récupération et déchiffrement RSA avec clé privée...\n");
        }
    }
    if(res) mysql_free_result(res);
    mysql_close(conn);
}