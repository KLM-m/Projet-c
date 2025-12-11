#ifndef CLIENT_TYPES_H
#define CLIENT_TYPES_H

typedef struct {
    int id_utilisateur;
    char nom[50];
    char email[100];
    unsigned char cle_session[32]; 
} SessionClient;

#endif