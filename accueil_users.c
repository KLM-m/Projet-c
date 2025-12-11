#include <stdio.h>
#include <stdlib.h>
#include "accueil_users.h"
#include "client_messaging.h"
#include "client_social.h"
#include "client_security.h"

// Ici on garde le vrai nom pour la définition
void afficher_accueil_users(SessionClient* session) {
    if (!etablir_canal_securise(session)) {
        printf("Erreur critique : Session non sécurisée.\n");
        return;
    }

    int choix;
    do {
        printf("\n=======================================\n");
        printf("   TABLEAU DE BORD : %s (ID: %d)\n", session->nom, session->id_utilisateur);
        printf("=======================================\n");
        printf("1. 📨 Envoyer un Message (Sécurisé RSA)\n");
        printf("2. 📬 Boîte de réception\n");
        printf("3. 🔔 Vérifier les notifications\n");
        printf("0. 🚪 Se déconnecter\n");
        printf("=======================================\n");
        printf("Votre choix : ");
        
        if (scanf("%d", &choix) != 1) {
            while(getchar() != '\n');
            choix = -1;
        }
        
        switch (choix) {
            case 1: envoyer_message_texte(session); break;
            case 2: recevoir_messages_texte(session); break;
            case 3: verifier_nouveaux_messages(session); break;
            case 0: printf("\nDéconnexion...\n"); break;
            default: printf("\n❌ Choix invalide.\n");
        }
    } while (choix != 0);
}