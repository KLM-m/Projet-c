#include "crypto.h"
#include "config.h"
#include "utils.h"
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>

void generer_cles_locales(int user_id) {
    printf("[Info] Generation cles RSA pour ID %d...\n", user_id);
    char cmd[512];
    sprintf(cmd, "openssl genrsa -out .secrets/private.pem 2048 2>NUL");
    system(cmd);
    char pub_path[256];
    sprintf(pub_path, "%s/%d.pub", KEYS_DIR, user_id);
    sprintf(cmd, "openssl rsa -in .secrets/private.pem -pubout -out %s 2>NUL", pub_path);
    system(cmd);
}

RSA* lire_cle_publique(int user_id) {
    char path[256];
    sprintf(path, "%s/%d.pub", KEYS_DIR, user_id);
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    RSA *rsa = PEM_read_RSA_PUBKEY(f, NULL, NULL, NULL);
    fclose(f);
    return rsa;
}

int chiffrer_et_sauvegarder(const char *source_path, const char *dest_path, int dest_id, int admin_id) {
    unsigned char aes_key[AES_KEY_LEN], iv[AES_IV_LEN];
    RAND_bytes(aes_key, AES_KEY_LEN);
    RAND_bytes(iv, AES_IV_LEN);

    RSA *rsa_dest = lire_cle_publique(dest_id);
    RSA *rsa_admin = lire_cle_publique(admin_id);
    
    if (!rsa_dest || !rsa_admin) {
        printf("[Erreur] Clés RSA manquantes (ID %d ou %d). Connectez ces utilisateurs une fois.\n", dest_id, admin_id);
        return 0;
    }

    unsigned char enc_key_dest[512], enc_key_admin[512];
    int len_d = RSA_public_encrypt(AES_KEY_LEN, aes_key, enc_key_dest, rsa_dest, RSA_PKCS1_OAEP_PADDING);
    int len_a = RSA_public_encrypt(AES_KEY_LEN, aes_key, enc_key_admin, rsa_admin, RSA_PKCS1_OAEP_PADDING);

    FILE *f_out = fopen(dest_path, "wb");
    if (!f_out) return 0;

    char hex_iv[65], hex_kd[1024], hex_ka[1024];
    bytes_to_hex(iv, AES_IV_LEN, hex_iv);
    bytes_to_hex(enc_key_dest, len_d, hex_kd);
    bytes_to_hex(enc_key_admin, len_a, hex_ka);
    fprintf(f_out, "%s\n%s\n%s\n", hex_iv, hex_kd, hex_ka);

    FILE *f_in = fopen(source_path, "rb");
    if (!f_in) { fclose(f_out); return 0; }

    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv, 1);
    
    unsigned char inbuf[4096], outbuf[4096 + 16];
    int inlen, outlen;
    while ((inlen = fread(inbuf, 1, 4096, f_in)) > 0) {
        EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen);
        fwrite(outbuf, 1, outlen, f_out);
    }
    EVP_CipherFinal_ex(ctx, outbuf, &outlen);
    fwrite(outbuf, 1, outlen, f_out);

    EVP_CIPHER_CTX_free(ctx);
    fclose(f_in); fclose(f_out);
    RSA_free(rsa_dest); RSA_free(rsa_admin);
    return 1;
}

void dechiffrer_fichier(const char *enc_path, const char *final_path) {
    FILE *f_in = fopen(enc_path, "rb");
    if (!f_in) { printf("Fichier introuvable.\n"); return; }

    char hex_iv[65], hex_kd[1024], hex_ka[1024];
    if (!fgets(hex_iv, 65, f_in)) { fclose(f_in); return; }
    fscanf(f_in, "%s", hex_kd);
    fscanf(f_in, "%s", hex_ka);
    fgetc(f_in); 

    FILE *fk = fopen(".secrets/private.pem", "rb");
    if (!fk) { printf("Erreur: Pas de clé privée.\n"); fclose(f_in); return; }
    RSA *priv = PEM_read_RSAPrivateKey(fk, NULL, NULL, NULL);
    fclose(fk);

    unsigned char enc_key[512], aes_key[AES_KEY_LEN], iv[AES_IV_LEN];
    hex_to_bytes(hex_iv, iv);
    hex_to_bytes(hex_kd, enc_key);

    if (RSA_private_decrypt(RSA_size(priv), enc_key, aes_key, priv, RSA_PKCS1_OAEP_PADDING) == -1) {
        hex_to_bytes(hex_ka, enc_key);
        if (RSA_private_decrypt(RSA_size(priv), enc_key, aes_key, priv, RSA_PKCS1_OAEP_PADDING) == -1) {
            printf("[Erreur] Echec déchiffrement RSA.\n");
            fclose(f_in); return;
        }
    }

    FILE *f_out = fopen(final_path, "wb");
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, aes_key, iv, 0);

    unsigned char inbuf[4096], outbuf[4096 + 16];
    int inlen, outlen;
    long pos = ftell(f_in);
    fseek(f_in, pos+1, SEEK_SET);

    while ((inlen = fread(inbuf, 1, 4096, f_in)) > 0) {
        EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen);
        fwrite(outbuf, 1, outlen, f_out);
    }
    EVP_CipherFinal_ex(ctx, outbuf, &outlen);
    fwrite(outbuf, 1, outlen, f_out);

    printf("✅ Sauvegardé sous : %s\n", final_path);
    EVP_CIPHER_CTX_free(ctx);
    fclose(f_in); fclose(f_out);
}