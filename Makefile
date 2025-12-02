CC = gcc
CFLAGS = -g -Wall -I"C:/Program Files/MariaDB/MariaDB Connector C 64-bit/include" -I"C:/Program Files/OpenSSL-Win64/include"

LDFLAGS = -L"C:/Program Files/MariaDB/MariaDB Connector C 64-bit/lib" -L"C:/Program Files/OpenSSL-Win64/lib"
LIBS = -lmariadb -lssl -lcrypto

EXEC = messagard
EXEC_TEST = test_chiffreur

SRCS = main.c admin_panel.c admin_connexion.c database.c custom_rsa.c
OBJS = $(SRCS:.c=.o)

SRCS_TEST = chiffrer_test_admin.c
OBJS_TEST = $(SRCS_TEST:.c=.o)


all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

test: $(EXEC_TEST)

$(EXEC_TEST): $(OBJS_TEST)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS_TEST) $(EXEC) $(EXEC_TEST) *.exe

.PHONY: all test clean
