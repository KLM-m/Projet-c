#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "custom_rsa.h" // On utilise notre propre librairie

// Fonction pour charger la clé publique depuis notre format "e,n"
static int charger_cle_publique_custom(CustomRsaPublicKey* pub, const char* chemin_pub) {
    FILE* f = fopen(chemin_pub, "r");
    if (!f) return 0;
    if (fscanf(f, "%llu,%llu", &pub->e, &pub->n) != 2) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

int main() {
    const char* input_file = "fichier_test_supp.txt";
    const char* pubkey_file = ".secrets/public_key.pem";
    const char* output_file = "fichier_test_chiffre_supp.bin";

    // Créer un fichier de test s'il n'existe pas
    FILE* f_test = fopen(input_file, "r");
    if (!f_test) {
        f_test = fopen(input_file, "w");
        if(f_test) {
            fprintf(f_test, "Ceci est un fichier de test pour le chiffrement.");
            fclose(f_test);
        }
    } else {
        fclose(f_test);
    }

    CustomRsaPublicKey cle_pub;
    if (!charger_cle_publique_custom(&cle_pub, pubkey_file)) {
        printf("Erreur: Impossible de charger la clé publique depuis %s\n", pubkey_file);
        return 1;
    }

    if (!custom_rsa_chiffrer_fichier(input_file, output_file, cle_pub)) {
        printf("Erreur lors du chiffrement du fichier.\n");
        return 1;
    }

    printf("Fichier '%s' chiffre avec la cle personnalisee -> '%s'\n", input_file, output_file);

    return 0;
}

