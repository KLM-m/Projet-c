#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include "client_social.h"
#include "database.h"

void verifier_nouveaux_messages(SessionClient* session) {
    MYSQL *conn = get_db_connection();
    if (!conn) return;
    
    char req[300];
    sprintf(req, "SELECT COUNT(*) FROM fichier WHERE id_utilisateur_destinataire=%d AND statut='valide'", session->id_utilisateur);
    
    if(mysql_query(conn, req)) { mysql_close(conn); return; }
    
    MYSQL_RES *res = mysql_store_result(conn);
    MYSQL_ROW row = mysql_fetch_row(res);
    int nb = atoi(row[0]);
    
    printf("\n----------------------------------\n");
    if(nb > 0) printf("🔔 OUI ! Vous avez %d message(s).\n", nb);
    else printf("🔕 NON. Rien pour le moment.\n");
    printf("----------------------------------\n");
    
    mysql_free_result(res);
    mysql_close(conn);
}