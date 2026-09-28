INTERFACE WEB — SYSTEME DE GESTION ELECTORALE UEH
====================================================

FICHIERS WEB
------------
index.html     : interface principale compatible GitHub Pages
styles.css     : design responsive, mode clair/sombre et composants UI
app.js         : logique de l'interface, donnees locales et rapports
favicon.svg    : icone du site

UTILISATION
-----------
Ouvrir index.html dans un navigateur moderne.
Aucun serveur n'est necessaire pour la demonstration locale.
Les donnees saisies dans la version web sont conservees dans localStorage du navigateur.

DEPLOIEMENT GITHUB PAGES
------------------------
Le fichier index.html est volontairement place a la racine du projet afin de permettre
un deploiement direct par GitHub Pages.

FONCTIONNALITES WEB
-------------------
- Tableau de bord avec indicateurs, repartition des electeurs et derniers votes.
- Creation d'un BV avec inscription immediate du president, du vice-president et du secretaire
  comme electeurs du BV.
- Liste des BV avec filtre par departement.
- Liste des electeurs avec filtre par departement et par BV.
- Gestion des candidats, partis et votes.
- Vote blanc et controle d'un seul vote par electeur.
- Rapport « BV par departement ».
- Rapport « Electeurs par departement et par BV ».
- Resultats nationaux et lecture par departement.
- Export / import JSON pour sauvegarder une demonstration.
- Mode sombre, recherche globale et affichage responsive mobile/tablette/ordinateur.
- Mode demonstration avec des donnees explicitement fictives.

NOTE SUR LES FICHIERS .DAT
---------------------------
Les fichiers .dat du projet C sont des fichiers binaires et ne sont pas lus directement
par JavaScript dans cette interface statique. L'interface web possede son propre stockage
local navigateur et un export/import JSON.

PRINCIPES DE CONCEPTION
-----------------------
L'interface utilise une navigation laterale, des cartes d'indicateurs, des tableaux sans
barres verticales inutiles, un contraste lisible, des etats de focus visibles et une adaptation
aux petits ecrans. Les icones sont dessinees en SVG/CSS pour eviter une dependance externe.
