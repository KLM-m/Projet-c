# Messagard - Application de Messagerie Chiffree

Application C de messagerie securisee avec chiffrement des fichiers et supervision administrative.

## Description

Messagard est une application de messagerie chiffree permettant aux utilisateurs d'echanger des fichiers de maniere securisee. L'application inclut un panel d'administration pour superviser, valider ou rejeter les fichiers en attente.

## Fonctionnalites

### Panel Utilisateur (A implementer)

- Creation de compte utilisateur
- Connexion utilisateur
- Envoi de fichiers chiffres

### Panel Admin (F-A1 a F-A6)

- **F-A1** : Connexion securisee (Interface Admin) ✅

  - Authentification avec hashage SHA-256
  - Verification dans la base de donnees MariaDB
- **F-A2** : Dashboard de supervision des fichiers en attente ✅

  - Affichage de tous les fichiers en attente
  - Informations detaillees (ID, nom, source, destinataire, statut)
- **F-A3** : Telecharger et dechiffrer un fichier en attente (A implementer)

  - Selection d'un fichier depuis le dashboard
  - Telechargement et dechiffrement avec la cle privee Admin
- **F-A4** : Fonction 'Valider' un fichier (A implementer)

  - Validation d'un fichier en attente
  - Chiffrement pour le destinataire B
  - Envoi/notification au destinataire
- **F-A5** : Fonction 'Rejeter' un fichier (A implementer)

  - Rejet d'un fichier en attente
  - Suppression du fichier
  - Notification a l'expediteur A
- **F-A6** : Interface d'audit des messages (A implementer)

  - Affichage de tous les messages/fichiers echanges
  - Dechiffrement avec la cle privee Admin
  - Consultation a posteriori

## Pre-requis

### Systeme

- Windows (teste sur Windows 10/11)
- Compilateur GCC (MinGW/MSYS2)
- Serveur MariaDB accessible en reseau

### Bibliotheques

- **MariaDB Connector/C 3.3.8** (64-bit)

  - Telechargement : https://mariadb.com/downloads/connectors/connectors-data/mariadb-connector-c/
  - Installation : Installer le package MSI "mariadb-connector-c-3.3.8-win64.msi"
  - Chemin d'installation par defaut : `C:\Program Files\MariaDB\MariaDB Connector C 64-bit\`
- **OpenSSL** (pour le hashage SHA-256)

  - Inclus dans MSYS2/MinGW64
  - Ou telecharger depuis : https://slproweb.com/products/Win32OpenSSL.html

### Base de donnees

- **MariaDB** (serveur distant)
  - Adresse IP : 192.168.86.128 (a adapter selon votre configuration)
  - Base de donnees : `projet_c`
  - Utilisateur : `testuser` (a adapter)
  - Mot de passe : `Azerty01` (a adapter)

## Installation

### 1. Installer MariaDB Connector/C 3.3.8

1. Telecharger `mariadb-connector-c-3.3.8-win64.msi` depuis le site officiel MariaDB
2. Installer avec l'option "Typical" (inclut Development Components et DLL Libraries)
3. Noter le chemin d'installation (generalement `C:\Program Files\MariaDB\MariaDB Connector C 64-bit\`)

### 2. Preparer la base de donnees

1. Se connecter au serveur MariaDB
2. Executer le script SQL `Code_bdd.sql` pour creer la base et les tables :
   ```sql
   mysql -u root -p < Code_bdd.sql
   ```

   Ou executer manuellement les commandes SQL du fichier.

### 3. Configurer la connexion

Modifier dans `admin_connexion.c` et `admin_panel.c` :

- Adresse IP du serveur MariaDB
- Nom d'utilisateur de la base de donnees
- Mot de passe de la base de donnees
- Nom de la base de donnees

## Compilation

### Commande de compilation

```powershell
gcc main.c admin_connexion.c admin_panel.c -o Messagard -I"C:\Program Files\MariaDB\MariaDB Connector C 64-bit\include" -L"C:\Program Files\MariaDB\MariaDB Connector C 64-bit\lib" -L"C:/msys64/mingw64/lib" -lmariadb -lssl -lcrypto
```

### Explication des flags

- `-I"..."` : Chemin vers les fichiers d'en-tete (headers) MariaDB
- `-L"..."` : Chemin vers les bibliotheques MariaDB et OpenSSL
- `-lmariadb` : Lier la bibliotheque MariaDB
- `-lssl -lcrypto` : Lier les bibliotheques OpenSSL pour le hashage SHA-256

### Note importante

Si vous utilisez une version differente de MariaDB Connector/C ou si l'installation est dans un autre dossier, adaptez les chemins `-I` et `-L` dans la commande de compilation.

## Execution

```powershell
.\Messagard.exe
```

## Structure du projet

```
Projet-c/
├── main.c                  # Point d'entree, menu principal
├── admin_connexion.c       # Gestion de la connexion admin
├── admin_connexion.h       # Headers pour la connexion admin
├── admin_panel.c           # Panel d'administration (dashboard, validation, etc.)
├── admin_panel.h           # Headers pour le panel admin
├── admin.h                 # Headers generaux admin
├── Code_bdd.sql            # Script SQL pour creer la base de donnees
└── README.md               # Ce fichier
```

## Configuration de la base de donnees

### Tables

#### Table `utilisateur`

- `id` : INT PRIMARY KEY AUTO_INCREMENT
- `nom` : VARCHAR(50)
- `email` : VARCHAR(100)
- `mot_de_passe` : VARCHAR(255) - Hash SHA-256 (64 caracteres hexa)
- `admin` : TINYINT(1) - 1 = admin, 0 = client

#### Table `fichier`

- `id` : INT PRIMARY KEY AUTO_INCREMENT
- `nom` : VARCHAR(100)
- `chemin` : VARCHAR(260)
- `id_utilisateur_source` : INT (FOREIGN KEY vers utilisateur)
- `id_utilisateur_destinataire` : INT (FOREIGN KEY vers utilisateur)
- `statut` : VARCHAR(20) - 'en_attente', 'valide', 'rejete', etc.

## Securite

- **Mots de passe** : Hashage SHA-256 avant stockage en base de donnees
- **Connexion SSL** : Desactivee pour le developpement local (a activer en production)
- **Authentification** : Verification email + hash du mot de passe + statut admin

## Notes importantes

- La version **MariaDB Connector/C 3.3.8** a ete choisie pour sa compatibilite et sa simplicite d'utilisation sans forcer SSL automatiquement
- Pour un usage en production, il est recommande d'activer SSL/TLS sur le serveur MariaDB et de configurer la connexion securisee dans le code
- Les chemins dans la commande de compilation doivent etre adaptes selon votre installation

## Auteur

Projet developpe dans le cadre d'un projet academique.

## Licence

A definir selon les besoins du projet.
