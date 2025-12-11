#include <openssl/rand.h>
#include "client_security.h"
int etablir_canal_securise(SessionClient* session) {
    return RAND_bytes(session->cle_session, 32);
}