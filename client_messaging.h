#ifndef CLIENT_MESSAGING_H
#define CLIENT_MESSAGING_H
#include "client_types.h"

void envoyer_message_texte(SessionClient* session);
void recevoir_messages_texte(SessionClient* session);

#endif