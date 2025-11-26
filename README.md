# Messagard - Projet C

Application de messagerie sécurisée développée en C, utilisant OpenSSL pour la génération de clés et le hachage, et MariaDB pour la gestion des utilisateurs.

## Prérequis

Avant de compiler, assurez-vous d'avoir installé les dépendances suivantes :

1.  **Compilateur GCC (MinGW-w64)** : Le moyen le plus simple est de l'installer via [MSYS2](https://www.msys2.org/).
2.  **Bibliothèque OpenSSL (développement)** : À installer via le gestionnaire de paquets de MSYS2.
    ```bash
    pacman -S mingw-w64-x86_64-openssl
    ```
3.  **MariaDB Connector/C** : Installez le connecteur C pour Windows depuis le [site officiel de MariaDB](https://mariadb.com/downloads/connectors/connectors-c/). Assurez-vous de noter le chemin d'installation (par ex: `C:\Program Files\MariaDB\MariaDB Connector C 64-bit`).

## Compilation

Ouvrez un terminal (PowerShell ou CMD) et placez-vous à la racine de ce projet (`Projet-c`).

Copiez et collez la commande suivante pour compiler l'application. Cette commande inclut les chemins vers les bibliothèques OpenSSL (via MSYS2) et MariaDB.

```powershell
gcc main.c admin_connexion.c admin_panel.c database.c -o Messagard.exe -I"C:/msys64/mingw64/include" -I"C:\Program Files\MariaDB\MariaDB Connector C 64-bit\include" -L"C:/msys64/mingw64/lib" -L"C:\Program Files\MariaDB\MariaDB Connector C 64-bit\lib" -lssl -lcrypto -lmariadb -lws2_32 -lcrypt32 -lgdi32
```

## Exécution

Après une compilation réussie, un fichier `Messagard.exe` sera créé.

**Important :** Pour que l'exécutable fonctionne, il doit pouvoir trouver les fichiers DLL nécessaires au moment de l'exécution. Le plus simple est de copier les fichiers suivants dans le même dossier que `Messagard.exe` :

- Depuis `C:\msys64\mingw64\bin` : `libcrypto-3-x64.dll`, `libssl-3-x64.dll` (les noms peuvent varier légèrement).
- Depuis `C:\Program Files\MariaDB\MariaDB Connector C 64-bit\bin` : `libmariadb.dll`.
