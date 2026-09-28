SYSTEME DE GESTION ELECTORALE - PRESIDENTIELLE
Universite d'Etat d'Haiti - Faculte des Sciences
Cours : Algorithmes et Structures de Donnees

FICHIERS SOURCE
---------------
election.h     : structures, nouveaux types, constantes et prototypes
ge_parti.c      : gestion des partis politiques
ge_BV.c         : gestion des bureaux de vote
ge_elect.c      : gestion des electeurs
ge_candi.c      : gestion des candidats
ge_votes.c      : gestion des votes
rapp.c          : rapports et resultats
velectro.c      : outils communs, menus et main

FICHIERS DE DONNEES
-------------------
p_pol.dat
B_votes.dat
elect.dat
candid.dat
votes.dat

REGLES IMPLEMENTEES
-------------------
- ID : exactement 8 caracteres, 2 lettres + 6 chiffres.
- NINU : exactement 10 chiffres.
- Aucun champ obligatoire ne peut etre vide.
- Dates reelles et controlees ; les dates futures sont refusees pour les dates d'inscription,
  de creation et de naissance. Les dates de naissance sont controlees par age.
- Electeur : au moins 18 ans.
- Candidat : au moins 25 ans.
- Email : format Gmail (nom@gmail.com).
- Telephone : 8 chiffres. Le sujet mentionne un format specifique mais n'en donne pas
  le masque numerique dans le document fourni ; ce projet adopte 8 chiffres pour le contexte haitien.
- Numero de rue : 1 a 3 chiffres maximum.
- Nom de rue : lettres, espaces et tirets.
- Nombre d'electeurs dynamique : listes chainees + malloc/free.
- Recherche, insertion, suppression et parcours realises avec des pointeurs et listes chainees.
- Modification par champ : l'utilisateur choisit uniquement l'information a modifier.
- Suppression des enregistrements avec controles d'integrite simples.
- Un electeur ne peut voter qu'une fois.
- Vote blanc pris en compte dans les resultats.
- Choix d'un candidat par NINU ou par nom.
- Date du vote obtenue automatiquement a partir du systeme.
- Nettoyage de l'ecran au retour au menu principal.
- Creation d'un BV et inscription immediate du president, du vice-president et du secretaire comme electeurs du BV.
- L'affectation supplementaire n'est donc plus necessaire apres la creation ; l'option de reaffectation reste disponible en maintenance.
- Affichages des listes sous forme verticale, sans bordures de tableau, pour une lecture claire dans la console.
- Gestion robuste des champs qui atteignent exactement leur longueur maximale (notamment le NINU de 10 chiffres).

COMPILATION GCC / CODE::BLOCKS
------------------------------
Commande :
    gcc -std=c11 -Wall -Wextra -pedantic -o election velectro.c ge_parti.c ge_BV.c ge_elect.c ge_candi.c ge_votes.c rapp.c

Sous Windows avec MinGW :
    gcc -std=c11 -Wall -Wextra -pedantic -o election.exe velectro.c ge_parti.c ge_BV.c ge_elect.c ge_candi.c ge_votes.c rapp.c

Pour Code::Blocks :
1. Creer un projet "Console application" en C.
2. Ajouter les 7 fichiers source/header du dossier.
3. Compiler et executer.

ORDRE CONSEILLE POUR LA DEMONSTRATION
-------------------------------------
1. Inscrire les partis politiques.
2. Creer chaque bureau de vote et inscrire directement ses trois responsables comme electeurs.
3. Inscrire les autres electeurs et les affecter a un BV existant.
4. Inscrire les candidats, dont au moins 1 independant.
5. Enregistrer des votes normaux et quelques votes blancs.
6. Afficher les listes, les BV par departement et les electeurs par departement/BV.
7. Modifier puis supprimer un enregistrement non reference.
8. Afficher les resultats.

NOTE SUR LES RAPPORTS
----------------------
Le module Resultats regroupe les votes par departement en utilisant le departement stocke
sur l'adresse de l'electeur, car la structure minimale du sujet ne met pas le departement
comme champ direct du vote. Le fichier rapp.c conserve un tri par insertion sur une liste
chainee de resultats afin de rester coherent avec le chapitre sur les listes chainees fourni.


INTERFACE WEB
-------------
index.html / styles.css / app.js : interface web moderne du projet, directement compatible
avec GitHub Pages. La version web permet notamment :
- creation d un BV avec inscription immediate de ses trois responsables comme electeurs ;
- affichage des BV avec filtre par departement ;
- affichage des electeurs avec filtre par departement et par BV ;
- rapports BV par departement et electeurs par departement/BV ;
- resultats nationaux et par departement ;
- recherche, mode sombre et export/import JSON.

La version web est une interface de demonstration autonome dans le navigateur. Les fichiers .dat
binaires du programme C ne sont pas lus directement par JavaScript ; l interface web utilise son
propre stockage navigateur et un format JSON pour l import/export.
