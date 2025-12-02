create database projet_c;
use projet_c;


CREATE TABLE utilisateur (
    id INT PRIMARY KEY AUTO_INCREMENT,
    nom VARCHAR(50) NOT NULL,
    email VARCHAR(100) NOT NULL,
    mot_de_passe VARCHAR(255) NOT NULL,
    admin TINYINT(1) NOT NULL
);

CREATE TABLE fichier (
    id INT PRIMARY KEY AUTO_INCREMENT,
    nom VARCHAR(100) NOT NULL,
    chemin VARCHAR(260) NOT NULL,
    id_utilisateur_source INT NOT NULL,
    id_utilisateur_destinataire INT NOT NULL,
    statut VARCHAR(20) NOT NULL,
    FOREIGN KEY(id_utilisateur_source) REFERENCES utilisateur(id),
    FOREIGN KEY(id_utilisateur_destinataire) REFERENCES utilisateur(id)
);

INSERT INTO utilisateur (nom, email, mot_de_passe, admin)
VALUES ('admin1', 'admin@admin.com', 'f2d81a260dea8a100dd517984e53c56a7523d96942a834bb7fbf2016b4a4583c', 1);

USE projet_c;

INSERT INTO utilisateur (nom, email, mot_de_passe, admin)
VALUES ('client1', 'client1@test.com', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 0);

INSERT INTO utilisateur (nom, email, mot_de_passe, admin)
VALUES ('client2', 'client2@test.com', 'e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855', 0);
