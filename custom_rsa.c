#include "custom_rsa.h"
#include <stdlib.h>
#include <string.h>


static ull exponentiation_modulaire(ull base, ull exp, ull mod) {
    ull res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

static ull inverse_modulaire(ull a, ull m) {
    ull m0 = m, t, q;
    long long x0 = 0, x1 = 1; 
    if (m == 1) return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;
    return (ull)x1;
}

int custom_rsa_generer_et_sauvegarder_cles(const char* chemin_pub, const char* chemin_priv) {
    ull p = 257; 
    ull q = 263;

    ull n = p * q;

    ull phi = (p - 1) * (q - 1);

    ull e = 17;

    ull d = inverse_modulaire(e, phi);

    FILE* f_pub = fopen(chemin_pub, "w");
    if (!f_pub) return 0;
    fprintf(f_pub, "%llu,%llu", e, n);
    fclose(f_pub);

    FILE* f_priv = fopen(chemin_priv, "w");
    if (!f_priv) return 0;
    fprintf(f_priv, "%llu,%llu", d, n);
    fclose(f_priv);

    return 1;
}

int custom_rsa_chiffrer_fichier(const char* chemin_entree, const char* chemin_sortie, CustomRsaPublicKey cle_pub) {
    FILE* f_in = fopen(chemin_entree, "rb");
    if (!f_in) return 0;
    FILE* f_out = fopen(chemin_sortie, "wb");
    if (!f_out) { fclose(f_in); return 0; }

    int octet;
    while ((octet = fgetc(f_in)) != EOF) {
        ull message = (ull)octet;
        ull chiffre = exponentiation_modulaire(message, cle_pub.e, cle_pub.n);
        fwrite(&chiffre, sizeof(ull), 1, f_out);
    }

    fclose(f_in);
    fclose(f_out);
    return 1;
}

int custom_rsa_dechiffrer_fichier(const char* chemin_entree, const char* chemin_sortie, CustomRsaPrivateKey cle_priv) {
    FILE* f_in = fopen(chemin_entree, "rb");
    if (!f_in) return 0;
    FILE* f_out = fopen(chemin_sortie, "wb");
    if (!f_out) { fclose(f_in); return 0; }

    ull chiffre;
    while (fread(&chiffre, sizeof(ull), 1, f_in) == 1) {
        ull dechiffre = exponentiation_modulaire(chiffre, cle_priv.d, cle_priv.n);
        unsigned char octet = (unsigned char)dechiffre;
        fputc(octet, f_out);
    }

    fclose(f_in);
    fclose(f_out);
    return 1;
}