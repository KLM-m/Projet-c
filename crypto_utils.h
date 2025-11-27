#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

/*
 * ####################################################################
 * # AVERTISSEMENT : IMPLÉMENTATION RSA PÉDAGOGIQUE ET NON SÉCURISÉE   #
 * ####################################################################
 * # Ce code est une simplification extrême de RSA à des fins         #
 * # éducatives uniquement. Il est mathématiquement correct pour de   #
 * # très petits nombres mais ne fournit AUCUNE sécurité réelle.      #
 * # - Utilise des entiers 64-bit (facilement cassables) au lieu de   #
 * #   clés de 2048/4096 bits.                                        #
 * # - N'utilise pas de padding (ex: OAEP), ce qui est critique.      #
 * # - La génération de nombres premiers est basique.                 #
 * #                                                                  #
 * # NE PAS UTILISER POUR PROTÉGER DES DONNÉES RÉELLES.               #
 * ####################################################################
 */

typedef unsigned long long ull;

// Structure pour une clé publique simplifiée
typedef struct {
    ull e; // Exposant
    ull n; // Modulus
} RsaPublicKey;

// Structure pour une clé privée simplifiée
typedef struct {
    ull d; // Exposant
    ull n; // Modulus
} RsaPrivateKey;

/**
 * @brief Génère une paire de clés RSA (p, q, e sont choisis pour la démo).
 * @param pub Pointeur vers la clé publique qui sera remplie.
 * @param priv Pointeur vers la clé privée qui sera remplie.
 */
void generer_paire_cles_rsa(RsaPublicKey* pub, RsaPrivateKey* priv);

/**
 * @brief Chiffre un message (un nombre) avec la clé publique.
 * @param message Le nombre à chiffrer.
 * @param pub La clé publique.
 * @return Le message chiffré.
 */
ull chiffrer_rsa(ull message, RsaPublicKey pub);

/**
 * @brief Déchiffre un message (un nombre) avec la clé privée.
 * @param message_chiffre Le nombre à déchiffrer.
 * @param priv La clé privée.
 * @return Le message original.
 */
ull dechiffrer_rsa(ull message_chiffre, RsaPrivateKey priv);

/**
 * @brief Sauvegarde une paire de clés dans des fichiers texte.
 * @param pub La clé publique à sauvegarder.
 * @param priv La clé privée à sauvegarder.
 * @param chemin_pub Chemin du fichier pour la clé publique.
 * @param chemin_priv Chemin du fichier pour la clé privée.
 * @return int 1 en cas de succès, 0 en cas d'échec.
 */
int sauvegarder_cles(RsaPublicKey pub, RsaPrivateKey priv, const char* chemin_pub, const char* chemin_priv);

/**
 * @brief Charge une clé publique depuis un fichier.
 * @param pub Pointeur vers la structure où charger la clé.
 * @param chemin_pub Chemin du fichier de la clé publique.
 * @return int 1 en cas de succès, 0 en cas d'échec.
 */
int charger_cle_publique(RsaPublicKey* pub, const char* chemin_pub);

/**
 * @brief Charge une clé privée depuis un fichier.
 * @param priv Pointeur vers la structure où charger la clé.
 * @param chemin_priv Chemin du fichier de la clé privée.
 * @return int 1 en cas de succès, 0 en cas d'échec.
 */
int charger_cle_privee(RsaPrivateKey* priv, const char* chemin_priv);

#endif // CRYPTO_UTILS_H