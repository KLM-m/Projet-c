#include "crypto_utils.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * ####################################################################
 * # AVERTISSEMENT : IMPLÉMENTATION RSA PÉDAGOGIQUE ET NON SÉCURISÉE   #
 * ####################################################################
 */

// Fonction pour calculer (base^exp) % mod
// Essentielle pour RSA, elle permet de manipuler de grands nombres sans dépassement.
ull exponentiation_modulaire(ull base, ull exp, ull mod) {
    ull res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1) res = (res * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return res;
}

// Algorithme d'Euclide étendu pour trouver l'inverse modulaire.
// Permet de trouver 'd' tel que (e * d) % phi == 1.
ull inverse_modulaire(ull a, ull m) {
    ull m0 = m, t, q;
    ull x0 = 0, x1 = 1;
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
    return x1;
}

/**
 * @brief Génère une paire de clés RSA (p, q, e sont choisis pour la démo).
 * Pour cette version PÉDAGOGIQUE, nous utilisons des nombres premiers fixes.
 * Dans une vraie implémentation, p et q seraient de très grands nombres premiers
 * générés aléatoirement.
 */
void generer_paire_cles_rsa(RsaPublicKey* pub, RsaPrivateKey* priv) {
    // 1. Choisir deux nombres premiers (très petits pour l'exemple)
    ull p = 61;
    ull q = 53;

    // 2. Calculer n = p * q
    ull n = p * q;

    // 3. Calculer l'indicatrice d'Euler : phi(n) = (p-1) * (q-1)
    ull phi = (p - 1) * (q - 1);

    // 4. Choisir un exposant public 'e' tel que 1 < e < phi et e est premier avec phi.
    // On prend une valeur commune.
    ull e = 17;

    // 5. Calculer l'exposant privé 'd' tel que (d * e) % phi = 1.
    // 'd' est l'inverse modulaire de 'e' modulo 'phi'.
    ull d = inverse_modulaire(e, phi);

    // Assigner les clés
    pub->e = e;
    pub->n = n;
    priv->d = d;
    priv->n = n;

    printf("--- Paire de clés RSA (simplifiée) générée ---\n");
    printf("Nombres premiers (p, q): %llu, %llu\n", p, q);
    printf("Modulus (n): %llu\n", n);
    printf("Phi(n): %llu\n", phi);
    printf("Clé publique (e, n): (%llu, %llu)\n", e, n);
    printf("Clé privée (d, n): (%llu, %llu)\n", d, n);
    printf("---------------------------------------------\n");
}

/**
 * @brief Chiffre un message (un nombre) avec la clé publique.
 * Formule : C = M^e mod n
 */
ull chiffrer_rsa(ull message, RsaPublicKey pub) {
    if (message >= pub.n) {
        fprintf(stderr, "Erreur: Le message est plus grand que le modulus n.\n");
        return 0;
    }
    return exponentiation_modulaire(message, pub.e, pub.n);
}

/**
 * @brief Déchiffre un message (un nombre) avec la clé privée.
 * Formule : M = C^d mod n
 */
ull dechiffrer_rsa(ull message_chiffre, RsaPrivateKey priv) {
    return exponentiation_modulaire(message_chiffre, priv.d, priv.n);
}

int sauvegarder_cles(RsaPublicKey pub, RsaPrivateKey priv, const char* chemin_pub, const char* chemin_priv) {
    FILE* f_pub = fopen(chemin_pub, "w");
    if (!f_pub) {
        perror("Impossible d'ouvrir le fichier de clé publique pour écriture");
        return 0;
    }
    fprintf(f_pub, "%llu,%llu", pub.e, pub.n);
    fclose(f_pub);

    FILE* f_priv = fopen(chemin_priv, "w");
    if (!f_priv) {
        perror("Impossible d'ouvrir le fichier de clé privée pour écriture");
        return 0;
    }
    fprintf(f_priv, "%llu,%llu", priv.d, priv.n);
    fclose(f_priv);

    return 1;
}

int charger_cle_publique(RsaPublicKey* pub, const char* chemin_pub) {
    FILE* f = fopen(chemin_pub, "r");
    if (!f) return 0;
    if (fscanf(f, "%llu,%llu", &pub->e, &pub->n) != 2) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

int charger_cle_privee(RsaPrivateKey* priv, const char* chemin_priv) {
    FILE* f = fopen(chemin_priv, "r");
    if (!f) return 0;
    if (fscanf(f, "%llu,%llu", &priv->d, &priv->n) != 2) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}