#ifndef CRYPTO_H
#define CRYPTO_H

void generer_cles_locales(int user_id);
int chiffrer_et_sauvegarder(const char *source_path, const char *dest_path, int dest_id, int admin_id);
void dechiffrer_fichier(const char *enc_path, const char *final_path);

#endif