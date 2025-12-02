#ifndef CUSTOM_RSA_H
#define CUSTOM_RSA_H

#include <stdio.h>



typedef unsigned long long ull;

typedef struct {
    ull e; 
    ull n; 
} CustomRsaPublicKey;

typedef struct {
    ull d; 
    ull n; 
} CustomRsaPrivateKey;


int custom_rsa_generer_et_sauvegarder_cles(const char* chemin_pub, const char* chemin_priv);


int custom_rsa_chiffrer_fichier(const char* chemin_entree, const char* chemin_sortie, CustomRsaPublicKey cle_pub);


int custom_rsa_dechiffrer_fichier(const char* chemin_entree, const char* chemin_sortie, CustomRsaPrivateKey cle_priv);


#endif 