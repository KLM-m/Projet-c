#include "config.h"
#include "utils.h"
#include "crypto.h"
#include "actions.h"

int main() {
    init_directories();

    MYSQL *conn = mysql_init(NULL);
    if (conn == NULL) {
        fprintf(stderr, "Erreur initialisation MySQL\n");
        return 1;
    }

    // --- FIX ULTIME ANTI-OVERRIDE DE CONFIGURATION ET SSL ---
    // 1. On empêche le client de lire my.ini/my.cnf, ce qui annule toute exigence SSL externe
    mysql_options(conn, MYSQL_READ_DEFAULT_GROUP, ""); 
    
    // 2. On désactive la vérification du certificat serveur (méthode la plus fiable pour contourner l'obligation SSL)
    my_bool verify_server_cert = 0; 
    mysql_options(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &verify_server_cert);
    // ----------------------------------------------------------------------

    // 3. Connexion au serveur
    if (!mysql_real_connect(conn, DB_HOST, DB_USER, DB_PASS, DB_NAME, 0, NULL, 0)) {
        fprintf(stderr, "\n[ERREUR] Impossible de se connecter a la BDD !\n");
        fprintf(stderr, "Détail : %s\n", mysql_error(conn));
        return 1;
    }

    int choix, my_id = -1;

    while (1) {
        if (my_id == -1) {
            printf("\n=== MESSAGARD (Connecte au serveur) ===\n");
            printf("1. Connexion\n");
            printf("2. Inscription\n");
            printf("0. Quitter\n");
            printf("Choix: ");
            
            if (scanf("%d", &choix) != 1) { while(getchar() != '\n'); continue; }
            if (choix == 0) break;
            
            char email[100], pass[100], hash[65];
            printf("Email: "); scanf("%s", email);
            printf("Mot de passe: "); scanf("%s", pass);
            
            hash_password_simple(pass, hash);

            if (choix == 2) { 
                char q[512];
                sprintf(q, "INSERT INTO utilisateur (nom, email, mot_de_passe, admin) VALUES ('User', '%s', '%s', 0)", email, hash);
                
                if (mysql_query(conn, q)) { 
                    printf("Erreur : %s\n", mysql_error(conn)); 
                } else {
                    mysql_query(conn, "SELECT MAX(id) FROM utilisateur");
                    MYSQL_ROW r = mysql_fetch_row(mysql_store_result(conn));
                    my_id = atoi(r[0]);
                    generer_cles_locales(my_id);
                    printf("Succes ! Compte cree avec l'ID : %d\n", my_id);
                }
            } else { 
                char q[512];
                sprintf(q, "SELECT id FROM utilisateur WHERE email='%s' AND mot_de_passe='%s'", email, hash);
                mysql_query(conn, q);
                MYSQL_RES *r = mysql_store_result(conn);
                
                if (r && mysql_num_rows(r) > 0) {
                    my_id = atoi(mysql_fetch_row(r)[0]);
                    printf("Connexion reussie ! Bienvenue ID %d.\n", my_id);
                    
                    FILE *check = fopen(".secrets/private.pem", "r");
                    if(!check) {
                        generer_cles_locales(my_id); 
                    } else {
                        fclose(check);
                    }
                } else {
                    printf("Identifiants incorrects.\n");
                }
            }
        } else {
            // --- MENU MEMBRE ---
            printf("\n--- ESPACE MEMBRE (ID %d) ---\n", my_id);
            printf("1. Envoyer un Message (Securise)\n");
            printf("2. Envoyer un Fichier (Securise)\n");
            printf("3. Actualiser (Recevoir messages/fichiers)\n");
            printf("0. Deconnexion\n");
            printf("Choix : ");
            
            if (scanf("%d", &choix) != 1) { while(getchar() != '\n'); continue; }

            if (choix == 1) envoyer_fichier(conn, my_id, 1);
            else if (choix == 2) envoyer_fichier(conn, my_id, 0);
            else if (choix == 3) verifier_reception(conn, my_id);
            else if (choix == 0) my_id = -1;
        }
    }
    
    mysql_close(conn);
    return 0;
}