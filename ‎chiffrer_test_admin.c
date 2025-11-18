#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

int main() {
    const char* input_file = "fichier_test_supp.txt";
    const char* pubkey_file = ".secrets/public_key.pem";
    const char* output_file = "fichier_test_chiffre_supp.bin";

    FILE* fin = fopen(input_file, "rb");
    if(!fin) { printf("Impossible d'ouvrir %s\n", input_file); return 1; }
    fseek(fin, 0, SEEK_END); long sz = ftell(fin); fseek(fin, 0, SEEK_SET);
    unsigned char* buf = (unsigned char*)malloc(sz);
    fread(buf,1,sz,fin); fclose(fin);

    FILE* fpub = fopen(pubkey_file, "r");
    if(!fpub) { printf("Impossible d'ouvrir %s\n", pubkey_file); free(buf); return 1; }
    EVP_PKEY* pkey = PEM_read_PUBKEY(fpub, NULL, NULL, NULL); fclose(fpub);
    if(!pkey) { printf("Cle publique invalide\n"); free(buf); return 1; }

    size_t key_size = EVP_PKEY_size(pkey);
    size_t max_plain = key_size - 2*EVP_MD_size(EVP_sha256()) - 2;
    if ((size_t)sz > max_plain) { printf("Fichier trop gros (%ld > %zu)\n", sz, max_plain); EVP_PKEY_free(pkey); free(buf); return 1; }

    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(pkey, NULL);
    if(!ctx) { EVP_PKEY_free(pkey); free(buf); return 1; }
    if (EVP_PKEY_encrypt_init(ctx) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }
    if (EVP_PKEY_CTX_set_rsa_oaep_md(ctx, EVP_sha256()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }
    if (EVP_PKEY_CTX_set_rsa_mgf1_md(ctx, EVP_sha256()) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }

    size_t outlen = 0;
    if (EVP_PKEY_encrypt(ctx, NULL, &outlen, buf, (size_t)sz) <= 0) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }
    unsigned char* out = (unsigned char*)malloc(outlen);
    if (!out) { EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }
    if (EVP_PKEY_encrypt(ctx, out, &outlen, buf, (size_t)sz) <= 0) { free(out); EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf); return 1; }

    FILE* fout = fopen(output_file, "wb");
    fwrite(out,1,outlen,fout); fclose(fout);

    printf("Fichier %s chiffre (OAEP-SHA256) -> %s\n", input_file, output_file);

    free(out); EVP_PKEY_CTX_free(ctx); EVP_PKEY_free(pkey); free(buf);
    return 0;
}