# Messagard - Système de Modération de Fichiers Sécurisé

Messagard est une application en C développée en ligne de commande qui agit comme un tiers de confiance pour le transfert de fichiers. Elle permet à des administrateurs de recevoir des fichiers chiffrés, de les inspecter, puis de les valider (en les re-chiffrant pour le destinataire final) ou de les rejeter.

Le projet met en œuvre une cryptographie RSA personnalisée à des fins éducatives et s'appuie sur une base de données MariaDB/MySQL pour la gestion des transactions.

## Fonctionnalités

- **Panel Administrateur :** Interface complète pour gérer le flux de modération.
- **Authentification Sécurisée :**
  - Mots de passe hachés en **SHA-256** (via OpenSSL).
  - Saisie du mot de passe masquée dans le terminal pour éviter l'exposition.
- **Chiffrement de Bout en Bout (Logique) :** Les fichiers sont chiffrés pour l'administrateur, puis re-chiffrés pour le destinataire final, garantissant que seul le destinataire prévu puisse lire le contenu validé.
- **Implémentation RSA Personnalisée :**
  - Génération de clés, chiffrement et déchiffrement de fichiers pour comprendre les mécanismes de la cryptographie asymétrique.
  - **AVERTISSEMENT :** Cette implémentation est à but **éducatif** et n'est pas sécurisée pour un usage en production.
- **Transfert de Fichiers via SCP/SSH :** Communication sécurisée avec un serveur de stockage distant pour le téléchargement et l'envoi des fichiers.
- **Base de Données MariaDB/MySQL :** Persistance des informations sur les utilisateurs, les fichiers et leur statut (en_attente, valide, rejete).
- **Journal d'Audit :** Consultation de l'historique complet de toutes les transactions de fichiers.

## Architecture et Flux de Travail

1.  **Soumission :** Un utilisateur (via l'utilitaire de test `test_chiffreur.exe`) chiffre un fichier avec la clé publique de l'administrateur et le dépose sur un serveur.
2.  **Connexion Admin :** L'administrateur se connecte à l'application Messagard. Sa clé publique locale est automatiquement synchronisée avec la base de données lors de la première connexion.
3.  **Modération :** Depuis son panel, l'administrateur peut :
    - Lister les fichiers en attente.
    - Télécharger et déchiffrer un fichier avec sa clé privée locale pour l'inspecter.
4.  **Décision :**
    - **Validation :** Le fichier est re-chiffré avec la clé publique du destinataire (récupérée depuis la BDD) et envoyé sur le serveur dans un dossier de distribution. Le fichier original est archivé.
    - **Rejet :** Le fichier est supprimé du serveur distant et les fichiers locaux temporaires sont nettoyés.

## Prérequis

- Un compilateur C comme `gcc`.
- L'utilitaire `make` (fourni avec MinGW/MSYS2 sous Windows).
- **MariaDB Connector/C :** La bibliothèque cliente pour interagir avec la base de données.
- **OpenSSL :** Uniquement pour la fonction de hachage SHA-256 des mots de passe.

## Compilation

Le projet utilise un `Makefile` pour simplifier la compilation.

1.  **Configurer le `Makefile` :**
    Assurez-vous que les chemins `CFLAGS` (pour les en-têtes) et `LDFLAGS` (pour les bibliothèques) pointent vers les bons dossiers d'installation de **MariaDB Connector/C** et **OpenSSL** sur votre système.

2.  **Compiler l'application principale :**

    ```bash
    make
    ```

    Cette commande génère l'exécutable `messagard.exe`.

3.  **Compiler l'utilitaire de test :**

    ```bash
    make test
    ```

    Cette commande génère l'exécutable `test_chiffreur.exe`, qui permet de simuler l'envoi d'un fichier par un utilisateur.

4.  **Nettoyer les fichiers compilés :**
    ```bash
    make clean
    ```

## Configuration

### Base de Données

Les informations de connexion à la base de données (hôte, utilisateur, mot de passe, nom de la base) sont définies dans `database.c`. Modifiez ces valeurs pour correspondre à votre configuration.

```c
// Extrait de database.c
#define DB_HOST "localhost"
#define DB_USER "votre_utilisateur"
#define DB_PASS "votre_mot_de_passe"
#define DB_NAME "messagard"
```

### Serveur SSH/SCP

Les informations de connexion au serveur de fichiers distant sont codées en dur dans `admin_panel.c`.

- **Adresse IP du serveur :** `192.168.86.128`
- **Utilisateur SSH :** `adix`
- **Dossiers distants :** `/home/adix/fichiers` (pour les fichiers validés) et `/home/adix/archives` (pour les fichiers originaux après validation).

Modifiez ces valeurs si votre configuration de serveur est différente.

## Utilisation

1.  **Lancer l'application principale :**

    ```bash
    ./messagard.exe
    ```

    Au premier lancement, une paire de clés RSA personnalisée sera générée et stockée dans le dossier `.secrets/`.

2.  **Générer un fichier de test chiffré :**
    ```bash
    ./test_chiffreur.exe
    ```
    Cela créera un fichier `fichier_test_supp.txt` et son équivalent chiffré `fichier_test_chiffre_supp.bin`, prêt à être déposé sur le serveur pour modération.

---

_Ce projet a été développé à des fins d'apprentissage du langage C avancé, de la cryptographie et de l'interaction avec des systèmes externes._
