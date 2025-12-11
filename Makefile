CC = gcc
CFLAGS = -g -Wall -I"C:/Program Files/MariaDB/MariaDB Connector C 64-bit/include" -I"C:/Program Files/OpenSSL-Win64/include"

LDFLAGS = -L"C:/Program Files/MariaDB/MariaDB Connector C 64-bit/lib" -L"C:/Program Files/OpenSSL-Win64/lib"
LIBS = -lmariadb -lssl -lcrypto

EXEC = messagard
EXEC_TEST = test_chiffreur

# J'ai ajouté ici tous les nouveaux modules clients
SRCS = main.c \
       admin_panel.c \
       admin_connexion.c \
       database.c \
       custom_rsa.c \
       client_auth.c \
       accueil_users.c \
       client_messaging.c \
       client_files.c \
       client_social.c \
       client_security.c

OBJS = $(SRCS:.c=.o)

SRCS_TEST = chiffrer_test_admin.c
OBJS_TEST = $(SRCS_TEST:.c=.o)

# Règle principale
all: $(EXEC)

# Linkage de l'exécutable principal
$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Règle de test
test: $(EXEC_TEST)

# Linkage du test (Attention: si le test utilise custom_rsa, il faut l'ajouter ici aussi)
$(EXEC_TEST): $(OBJS_TEST) custom_rsa.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compilation des fichiers objets (.c -> .o)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage
clean:
	rm -f $(OBJS) $(OBJS_TEST) $(EXEC).exe $(EXEC_TEST).exe *.o