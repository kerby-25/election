#ifndef ELECTION_H
#define ELECTION_H

/* ================================================================
   SYSTEME DE GESTION ELECTORALE - UNIVERSITE D'ETAT D'HAITI
   Projet : Algorithmes et Structures de Donnees
   Fichier : election.h
   Role : types, constantes et prototypes communs au projet.
   ================================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define FICHIER_PP      "p_pol.dat"
#define FICHIER_BV      "B_votes.dat"
#define FICHIER_ELECT   "elect.dat"
#define FICHIER_CAND    "candid.dat"
#define FICHIER_VOTE    "votes.dat"

#define TAILLE_ID       9       /* 8 caracteres + '\0' */
#define TAILLE_NINU     11      /* 10 chiffres + '\0' */
#define TAILLE_NOM      80
#define TAILLE_PRENOM   80
#define TAILLE_RUE      100
#define TAILLE_DEPT     60
#define TAILLE_EMAIL    120
#define TAILLE_TEL      20
#define TAILLE_RESP     100
#define TAILLE_TYPE     20
#define TAILLE_NOM_BV   120

#define CODE_INDEPENDANT "0"
#define VOTE_BLANC       "BLANC"

/* ---------------------------- DATE ----------------------------- */
typedef struct {
    int jour;
    int mois;
    int annee;
} Date;

/* -------------------------- ADRESSE ---------------------------- */
typedef struct {
    int noRue;
    char rue[TAILLE_RUE];
    char departement[TAILLE_DEPT];
} Adresse;

/* --------------------- PARTI POLITIQUE ------------------------- */
typedef struct {
    char idPP[TAILLE_ID];
    char nom[TAILLE_NOM];
    Date dateCreation;
    Date dateInscription;
    Adresse adresse;
    char responsable[TAILLE_RESP];
    char telephone[TAILLE_TEL];
    char email[TAILLE_EMAIL];
} Parti;

typedef struct MaillonParti {
    Parti information;
    struct MaillonParti *suivant;
} MaillonParti;

/* ------------------------ BUREAU DE VOTE ----------------------- */
typedef struct {
    char idBV[TAILLE_ID];
    Adresse adresse;
    char idPresident[TAILLE_NINU];
    char idVicePresident[TAILLE_NINU];
    char idSecretaire[TAILLE_NINU];
    int membresAffectes; /* 0 avant affectation, 1 apres affectation */
} BureauVote;

typedef struct MaillonBV {
    BureauVote information;
    struct MaillonBV *suivant;
} MaillonBV;

/* --------------------------- ELECTEUR -------------------------- */
typedef struct {
    char ninu[TAILLE_NINU];
    char nom[TAILLE_NOM];
    char prenom[TAILLE_PRENOM];
    Adresse adresse;
    char idBV[TAILLE_ID];
    Date dateNaissance;
} Electeur;

typedef struct MaillonElecteur {
    Electeur information;
    struct MaillonElecteur *suivant;
} MaillonElecteur;

/* --------------------------- CANDIDAT -------------------------- */
typedef struct {
    char idCandidat[TAILLE_ID];
    char ninu[TAILLE_NINU];
    char nom[TAILLE_NOM];
    char prenom[TAILLE_PRENOM];
    Date dateNaissance;
    Adresse adresse;
    char email[TAILLE_EMAIL];
    char telephone[TAILLE_TEL];
    char type[TAILLE_TYPE]; /* "0" ou Id_PP */
    Date dateInscription;
} Candidat;

typedef struct MaillonCandidat {
    Candidat information;
    struct MaillonCandidat *suivant;
} MaillonCandidat;

/* ---------------------------- VOTE ----------------------------- */
typedef struct {
    char idVote[TAILLE_ID];
    Date dateVote;
    char idBV[TAILLE_ID];
    char ninu[TAILLE_NINU];
    char idCandidat[TAILLE_ID]; /* VOTE_BLANC si vote blanc */
} Vote;

typedef struct MaillonVote {
    Vote information;
    struct MaillonVote *suivant;
} MaillonVote;

/* ====================== OUTILS COMMUNS ========================= */
void vider_ecran(void);
void pause_console(void);
void vider_buffer(void);
void nettoyer_chaine(char *s);
void saisir_chaine_obligatoire(const char *invite, char *destination, size_t taille);
int chaine_est_numerique(const char *s);
int chaine_est_lettres(const char *s);
int valider_id_format(const char *id);
int valider_ninu_format(const char *ninu);
int valider_email_gmail(const char *email);
int egale_sans_casse(const char *a, const char *b);
int valider_telephone(const char *telephone);
int valider_numero_rue_1_3_chiffres(const char *numero);
int valider_nom_rue(const char *rue);
int date_valide(Date d);
int comparer_dates(Date a, Date b);
Date date_systeme(void);
int date_egale_systeme(Date d);
int age_aujourd_hui(Date naissance);
int date_saisir(const char *invite, Date *d, int dateNeDoitPasEtreFuture);
int date_saisir_age_minimum(const char *invite, Date *d, int ageMinimum);
void saisir_adresse(Adresse *adresse);
int confirmer(const char *question);
void afficher_date(Date d);
void afficher_adresse(const Adresse *a);

/* ===================== LISTES / PARTIS ========================= */
MaillonParti *inserer_parti(MaillonParti *debut, Parti p);
MaillonParti *rechercher_parti(MaillonParti *debut, const char *id);
MaillonParti *supprimer_parti_liste(MaillonParti *debut, const char *id, int *ok);
void liberer_partis(MaillonParti **debut);
int liste_partis_vide(MaillonParti *debut);
int charger_partis(MaillonParti **debut);
int sauvegarder_partis(MaillonParti *debut);
void afficher_parti(const Parti *p);
void afficher_partis(MaillonParti *debut);
void ins_PP(MaillonParti **debut);
void modif_PP(MaillonParti *debut);
void suppr_PP(MaillonParti **debut, MaillonCandidat *listeCandidats);

/* ======================== BUREAUX ============================== */
MaillonBV *inserer_bv(MaillonBV *debut, BureauVote bv);
MaillonBV *rechercher_bv(MaillonBV *debut, const char *id);
MaillonBV *supprimer_bv_liste(MaillonBV *debut, const char *id, int *ok);
void liberer_bv(MaillonBV **debut);
int charger_bv(MaillonBV **debut);
int sauvegarder_bv(MaillonBV *debut);
void afficher_bv(const BureauVote *bv);
void afficher_bvs(MaillonBV *debut);
void ajout_BV(MaillonBV **debut, MaillonElecteur **listeElecteurs);
void affecter_membres_BV(MaillonBV *debut, MaillonElecteur *listeElecteurs);
void modif_BV(MaillonBV *debut, MaillonElecteur *listeElecteurs);
void suppr_BV(MaillonBV **debut, MaillonElecteur *listeElecteurs);

/* ======================== ELECTEURS ============================ */
MaillonElecteur *inserer_electeur(MaillonElecteur *debut, Electeur e);
MaillonElecteur *rechercher_electeur(MaillonElecteur *debut, const char *ninu);
MaillonElecteur *supprimer_electeur_liste(MaillonElecteur *debut, const char *ninu, int *ok);
void liberer_electeurs(MaillonElecteur **debut);
int charger_electeurs(MaillonElecteur **debut);
int sauvegarder_electeurs(MaillonElecteur *debut);
void afficher_electeur(const Electeur *e);
void afficher_electeurs(MaillonElecteur *debut);
void ins_elect(MaillonElecteur **debut, MaillonBV *listeBV);
void modif_elect(MaillonElecteur *debut, MaillonBV *listeBV);
void suppr_elect(MaillonElecteur **debut, MaillonBV *listeBV, MaillonVote *listeVotes);

/* ======================== CANDIDATS ============================ */
MaillonCandidat *inserer_candidat(MaillonCandidat *debut, Candidat c);
MaillonCandidat *rechercher_candidat_id(MaillonCandidat *debut, const char *id);
MaillonCandidat *rechercher_candidat_ninu(MaillonCandidat *debut, const char *ninu);
MaillonCandidat *supprimer_candidat_liste(MaillonCandidat *debut, const char *id, int *ok);
void liberer_candidats(MaillonCandidat **debut);
int charger_candidats(MaillonCandidat **debut);
int sauvegarder_candidats(MaillonCandidat *debut);
void afficher_candidat(const Candidat *c);
void afficher_candidats(MaillonCandidat *debut);
void ins_cand(MaillonCandidat **debut, MaillonParti *listePartis);
void modif_cand(MaillonCandidat *debut, MaillonParti *listePartis);
void suppr_cand(MaillonCandidat **debut, MaillonVote *listeVotes);
void afficher_candidats_par_nom(MaillonCandidat *debut, const char *nom);

/* =========================== VOTES ============================= */
MaillonVote *inserer_vote(MaillonVote *debut, Vote v);
MaillonVote *rechercher_vote_id(MaillonVote *debut, const char *id);
MaillonVote *rechercher_vote_electeur(MaillonVote *debut, const char *ninu);
void liberer_votes(MaillonVote **debut);
int charger_votes(MaillonVote **debut);
int sauvegarder_votes(MaillonVote *debut);
void afficher_vote(const Vote *v);
void afficher_votes(MaillonVote *debut);
void enregistrer_vote(MaillonVote **listeVotes, MaillonElecteur *listeElecteurs,
                      MaillonBV *listeBV, MaillonCandidat *listeCandidats);

/* ====================== RAPPORTS / RESULTATS =================== */
void List_Electeurs(MaillonElecteur *listeElecteurs, MaillonBV *listeBV);
void List_Electeurs_Par_Departement_BV(MaillonElecteur *listeElecteurs, MaillonBV *listeBV);
void List_Candid(MaillonCandidat *listeCandidats);
void List_BV(MaillonBV *listeBV, MaillonElecteur *listeElecteurs);
void List_BV_Par_Departement(MaillonBV *listeBV, MaillonElecteur *listeElecteurs);
void Resultats(MaillonBV *listeBV, MaillonCandidat *listeCandidats,
               MaillonVote *listeVotes);

/* ========================= AFFICHAGE =========================== */
void menu_principal(MaillonParti **partis, MaillonBV **bvs,
                    MaillonElecteur **electeurs, MaillonCandidat **candidats,
                    MaillonVote **votes);

#endif /* ELECTION_H */
