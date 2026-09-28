/* ================================================================
   MODULE BUREAUX DE VOTE
   Fichier : ge_BV.c
   ================================================================ */
#include "election.h"

#define MEMBRE_NON_AFFECTE "NON_ATTR"

MaillonBV *inserer_bv(MaillonBV *debut, BureauVote bv) {
    MaillonBV *nouveau = (MaillonBV *)malloc(sizeof(MaillonBV));
    MaillonBV *p;
    if (nouveau == NULL) return NULL;
    nouveau->information = bv;
    nouveau->suivant = NULL;
    if (debut == NULL) return nouveau;
    p = debut;
    while (p->suivant != NULL) p = p->suivant;
    p->suivant = nouveau;
    return debut;
}

MaillonBV *rechercher_bv(MaillonBV *debut, const char *id) {
    while (debut != NULL) {
        if (strcmp(debut->information.idBV, id) == 0) return debut;
        debut = debut->suivant;
    }
    return NULL;
}

MaillonBV *supprimer_bv_liste(MaillonBV *debut, const char *id, int *ok) {
    MaillonBV *courant = debut, *precedent = NULL;
    *ok = 0;
    while (courant != NULL && strcmp(courant->information.idBV, id) != 0) {
        precedent = courant;
        courant = courant->suivant;
    }
    if (courant == NULL) return debut;
    if (precedent == NULL) debut = courant->suivant;
    else precedent->suivant = courant->suivant;
    free(courant);
    *ok = 1;
    return debut;
}

void liberer_bv(MaillonBV **debut) {
    MaillonBV *p = *debut, *tmp;
    while (p != NULL) {
        tmp = p;
        p = p->suivant;
        free(tmp);
    }
    *debut = NULL;
}

int charger_bv(MaillonBV **debut) {
    FILE *f = fopen(FICHIER_BV, "rb");
    BureauVote bv;
    MaillonBV *liste = NULL, *tmp;
    int n = 0;
    if (f == NULL) return 0;
    while (fread(&bv, sizeof(BureauVote), 1, f) == 1) {
        /* Compatibilite avec les anciennes donnees : si le champ n'existe
           pas logiquement, on considere le BV comme non affecte. */
        if (bv.membresAffectes != 1) {
            bv.membresAffectes = 0;
            strcpy(bv.idPresident, MEMBRE_NON_AFFECTE);
            strcpy(bv.idVicePresident, MEMBRE_NON_AFFECTE);
            strcpy(bv.idSecretaire, MEMBRE_NON_AFFECTE);
        }
        tmp = inserer_bv(liste, bv);
        if (tmp == NULL) {
            liberer_bv(&liste);
            fclose(f);
            return -1;
        }
        liste = tmp;
        n++;
    }
    fclose(f);
    liberer_bv(debut);
    *debut = liste;
    return n;
}

int sauvegarder_bv(MaillonBV *debut) {
    FILE *f = fopen(FICHIER_BV, "wb");
    if (f == NULL) return 0;
    while (debut != NULL) {
        if (fwrite(&debut->information, sizeof(BureauVote), 1, f) != 1) {
            fclose(f);
            return 0;
        }
        debut = debut->suivant;
    }
    fclose(f);
    return 1;
}

void afficher_bv(const BureauVote *bv) {
    printf("ID BV           : %s\n", bv->idBV);
    printf("Adresse         : ");
    afficher_adresse(&bv->adresse);
    printf("\n");
    printf("President       : %s\n", bv->membresAffectes ? bv->idPresident : "NON AFFECTE");
    printf("Vice-president  : %s\n", bv->membresAffectes ? bv->idVicePresident : "NON AFFECTE");
    printf("Secretaire      : %s\n", bv->membresAffectes ? bv->idSecretaire : "NON AFFECTE");
}

/* Affichage vertical : aucune barre de tableau, chaque BV est separe du suivant. */
void afficher_bvs(MaillonBV *debut) {
    if (debut == NULL) {
        printf("Aucun bureau de vote.\n");
        return;
    }

    printf("\n================ LISTE DES BUREAUX DE VOTE ================\n\n");
    printf("%-10s %-32s %-16s %-16s %-16s\n",
           "ID_BV", "ADRESSE", "PRESIDENT", "VICE-PRESIDENT", "SECRETAIRE");

    while (debut != NULL) {
        char adresse[180];
        const char *president = debut->information.membresAffectes ? debut->information.idPresident : "NON AFFECTE";
        const char *vice = debut->information.membresAffectes ? debut->information.idVicePresident : "NON AFFECTE";
        const char *secretaire = debut->information.membresAffectes ? debut->information.idSecretaire : "NON AFFECTE";

        snprintf(adresse, sizeof(adresse), "%d, %s, %s",
                 debut->information.adresse.noRue,
                 debut->information.adresse.rue,
                 debut->information.adresse.departement);

        printf("%-10s %-32.32s %-16s %-16s %-16s\n",
               debut->information.idBV, adresse, president, vice, secretaire);
        debut = debut->suivant;
    }
}

static int saisir_id_bv_unique(MaillonBV *liste, char *id) {
    for (;;) {
        saisir_chaine_obligatoire("ID_BV (2 lettres + 6 chiffres) : ", id, TAILLE_ID);
        for (size_t i = 0; i < strlen(id); ++i)
            id[i] = (char)toupper((unsigned char)id[i]);
        if (!valider_id_format(id)) {
            printf("ERREUR : format ID invalide.\n");
        } else if (rechercher_bv(liste, id) != NULL) {
            printf("ERREUR : ID deja utilise.\n");
        } else {
            return 1;
        }
    }
}

static int valider_membre_bv(MaillonElecteur *electeurs, const char *ninu) {
    return valider_ninu_format(ninu) && rechercher_electeur(electeurs, ninu) != NULL;
}

static int membre_deja_utilise(const char *ninu, const char *a, const char *b) {
    return strcmp(ninu, a) == 0 || strcmp(ninu, b) == 0;
}

/* Saisit et prepare un responsable comme nouvel electeur du BV en cours de creation. */
static int saisir_responsable_electeur(Electeur *e,
                                       MaillonElecteur *listeElecteurs,
                                       const char *idBV,
                                       const char *role,
                                       const char *ninu1,
                                       const char *ninu2) {
    char ninu[TAILLE_NINU];

    printf("\n--- INSCRIPTION DU %s ---\n", role);
    do {
        saisir_chaine_obligatoire("NINU (exactement 10 chiffres) : ", ninu, sizeof(ninu));
        if (!valider_ninu_format(ninu)) {
            printf("ERREUR : le NINU doit contenir exactement 10 chiffres.\n");
        } else if (rechercher_electeur(listeElecteurs, ninu) != NULL) {
            printf("ERREUR : ce NINU est deja inscrit comme electeur. Utilisez un autre NINU.\n");
        } else if ((ninu1 != NULL && strcmp(ninu, ninu1) == 0) ||
                   (ninu2 != NULL && strcmp(ninu, ninu2) == 0)) {
            printf("ERREUR : ce responsable est deja utilise pour ce BV.\n");
        } else {
            break;
        }
    } while (1);

    strcpy(e->ninu, ninu);
    saisir_chaine_obligatoire("Nom : ", e->nom, sizeof(e->nom));
    saisir_chaine_obligatoire("Prenom : ", e->prenom, sizeof(e->prenom));
    date_saisir_age_minimum("Date de naissance (JJ/MM/AAAA) : ", &e->dateNaissance, 18);
    saisir_adresse(&e->adresse);
    strcpy(e->idBV, idBV);
    return 1;
}

/* Cree un BV et inscrit immediatement ses trois responsables comme electeurs. */
void ajout_BV(MaillonBV **debut, MaillonElecteur **listeElecteurs) {
    BureauVote bv;
    Electeur president, vice, secretaire;
    MaillonBV *tmpBV;
    MaillonElecteur *tmpElecteur;
    char idBV[TAILLE_ID];

    printf("\n========== CREATION D'UN BUREAU DE VOTE ==========\n");
    printf("RAPPEL : le president, le vice-president et le secretaire\n");
    printf("         sont inscrits immediatement comme electeurs de ce BV.\n");
    printf("         Aucun traitement d'affectation supplementaire ne sera necessaire.\n\n");

    saisir_id_bv_unique(*debut, idBV);
    saisir_adresse(&bv.adresse);

    /* Les trois responsables sont des electeurs a part entiere. */
    memset(&president, 0, sizeof(Electeur));
    memset(&vice, 0, sizeof(Electeur));
    memset(&secretaire, 0, sizeof(Electeur));

    saisir_responsable_electeur(&president, *listeElecteurs, idBV,
                                "PRESIDENT", NULL, NULL);
    saisir_responsable_electeur(&vice, *listeElecteurs, idBV,
                                "VICE-PRESIDENT", president.ninu, NULL);
    saisir_responsable_electeur(&secretaire, *listeElecteurs, idBV,
                                "SECRETAIRE", president.ninu, vice.ninu);

    strcpy(bv.idBV, idBV);
    strcpy(bv.idPresident, president.ninu);
    strcpy(bv.idVicePresident, vice.ninu);
    strcpy(bv.idSecretaire, secretaire.ninu);
    bv.membresAffectes = 1;

    /* Ajout en memoire : les deux listes sont maintenant coherentes. */
    tmpElecteur = inserer_electeur(*listeElecteurs, president);
    if (tmpElecteur == NULL) {
        printf("ERREUR : memoire insuffisante. Creation annulee.\n");
        return;
    }
    *listeElecteurs = tmpElecteur;

    tmpElecteur = inserer_electeur(*listeElecteurs, vice);
    if (tmpElecteur == NULL) {
        int ok;
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, president.ninu, &ok);
        printf("ERREUR : memoire insuffisante. Creation annulee.\n");
        return;
    }
    *listeElecteurs = tmpElecteur;

    tmpElecteur = inserer_electeur(*listeElecteurs, secretaire);
    if (tmpElecteur == NULL) {
        int ok;
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, president.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, vice.ninu, &ok);
        printf("ERREUR : memoire insuffisante. Creation annulee.\n");
        return;
    }
    *listeElecteurs = tmpElecteur;

    tmpBV = inserer_bv(*debut, bv);
    if (tmpBV == NULL) {
        int ok;
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, president.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, vice.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, secretaire.ninu, &ok);
        printf("ERREUR : memoire insuffisante. Creation annulee.\n");
        return;
    }
    *debut = tmpBV;

    /* Sauvegarde du BV avant les electeurs. En cas d'echec, on rollbacke tout. */
    if (!sauvegarder_bv(*debut)) {
        int ok;
        *debut = supprimer_bv_liste(*debut, idBV, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, president.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, vice.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, secretaire.ninu, &ok);
        printf("ERREUR : sauvegarde du BV impossible. Creation annulee.\n");
        return;
    }

    if (!sauvegarder_electeurs(*listeElecteurs)) {
        int ok;
        *debut = supprimer_bv_liste(*debut, idBV, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, president.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, vice.ninu, &ok);
        *listeElecteurs = supprimer_electeur_liste(*listeElecteurs, secretaire.ninu, &ok);
        /* Restaure les fichiers avec les listes revenues a leur etat precedent. */
        sauvegarder_bv(*debut);
        sauvegarder_electeurs(*listeElecteurs);
        printf("ERREUR : sauvegarde des electeurs impossible. Creation annulee.\n");
        return;
    }

    printf("\nBureau de vote %s cree avec succes.\n", idBV);
    printf("Les trois responsables ont ete inscrits automatiquement comme electeurs.\n");
}

/* Affecte les trois membres apres qu'ils ont ete inscrits comme electeurs. */
void affecter_membres_BV(MaillonBV *debut, MaillonElecteur *listeElecteurs) {
    char id[TAILLE_ID];
    char president[TAILLE_NINU], vice[TAILLE_NINU], secretaire[TAILLE_NINU];
    MaillonBV *bv;

    printf("\n========== AFFECTATION DES MEMBRES D'UN BV ==========\n");
    printf("RAPPEL : chaque membre (president, vice-president, secretaire)\n");
    printf("         doit deja etre inscrit comme electeur.\n");
    if (listeElecteurs == NULL) {
        printf("ERREUR : aucun electeur n'est encore inscrit.\n");
        printf("Inscrivez d'abord les electeurs, puis revenez ici.\n");
        return;
    }
    if (debut == NULL) {
        printf("ERREUR : aucun bureau de vote n'est encore cree.\n");
        printf("Creez d'abord un BV, puis inscrivez les electeurs.\n");
        return;
    }

    saisir_chaine_obligatoire("ID_BV a traiter : ", id, sizeof(id));
    for (size_t i = 0; i < strlen(id); ++i)
        id[i] = (char)toupper((unsigned char)id[i]);
    if (!valider_id_format(id)) {
        printf("ERREUR : format ID_BV invalide.\n");
        return;
    }
    bv = rechercher_bv(debut, id);
    if (bv == NULL) {
        printf("ERREUR : bureau de vote introuvable.\n");
        return;
    }

    do {
        saisir_chaine_obligatoire("NINU du president : ", president, sizeof(president));
        if (!valider_membre_bv(listeElecteurs, president))
            printf("ERREUR : ce NINU ne correspond pas a un electeur inscrit.\n");
    } while (!valider_membre_bv(listeElecteurs, president));

    do {
        saisir_chaine_obligatoire("NINU du vice-president : ", vice, sizeof(vice));
        if (!valider_membre_bv(listeElecteurs, vice))
            printf("ERREUR : ce NINU ne correspond pas a un electeur inscrit.\n");
        else if (strcmp(vice, president) == 0)
            printf("ERREUR : le vice-president doit etre different du president.\n");
    } while (!valider_membre_bv(listeElecteurs, vice) || strcmp(vice, president) == 0);

    do {
        saisir_chaine_obligatoire("NINU du secretaire : ", secretaire, sizeof(secretaire));
        if (!valider_membre_bv(listeElecteurs, secretaire))
            printf("ERREUR : ce NINU ne correspond pas a un electeur inscrit.\n");
        else if (membre_deja_utilise(secretaire, president, vice))
            printf("ERREUR : chaque membre du BV doit etre distinct.\n");
    } while (!valider_membre_bv(listeElecteurs, secretaire) || membre_deja_utilise(secretaire, president, vice));

    strcpy(bv->information.idPresident, president);
    strcpy(bv->information.idVicePresident, vice);
    strcpy(bv->information.idSecretaire, secretaire);
    bv->information.membresAffectes = 1;

    if (sauvegarder_bv(debut))
        printf("Les trois membres ont ete affectes avec succes au BV %s.\n", id);
    else
        printf("AVERTISSEMENT : affectation faite en memoire, mais sauvegarde impossible.\n");
}

void modif_BV(MaillonBV *debut, MaillonElecteur *listeElecteurs) {
    char id[TAILLE_ID];
    MaillonBV *n;
    int choix;

    saisir_chaine_obligatoire("ID_BV a modifier : ", id, sizeof(id));
    for (size_t i = 0; i < strlen(id); ++i)
        id[i] = (char)toupper((unsigned char)id[i]);
    n = rechercher_bv(debut, id);
    if (n == NULL) {
        printf("Bureau introuvable.\n");
        return;
    }

    do {
        printf("\n1. Adresse\n2. Reaffecter les membres\n0. Retour\nVotre choix : ");
        if (scanf("%d", &choix) != 1) { vider_buffer(); choix = -1; }
        else vider_buffer();
        switch (choix) {
            case 1:
                saisir_adresse(&n->information.adresse);
                if (sauvegarder_bv(debut)) printf("Modification effectuee.\n");
                break;
            case 2:
                affecter_membres_BV(debut, listeElecteurs);
                break;
            case 0:
                break;
            default:
                printf("Choix invalide.\n");
        }
    } while (choix != 0);
}

void suppr_BV(MaillonBV **debut, MaillonElecteur *listeElecteurs) {
    char id[TAILLE_ID];
    int ok;
    MaillonElecteur *e;
    MaillonBV *bv;

    saisir_chaine_obligatoire("ID_BV a supprimer : ", id, sizeof(id));
    for (size_t i = 0; i < strlen(id); ++i)
        id[i] = (char)toupper((unsigned char)id[i]);
    bv = rechercher_bv(*debut, id);
    if (bv == NULL) {
        printf("Bureau introuvable.\n");
        return;
    }

    e = listeElecteurs;
    while (e != NULL) {
        if (strcmp(e->information.idBV, id) == 0) {
            printf("ERREUR : impossible de supprimer ce BV : des electeurs y sont encore assignes.\n");
            return;
        }
        e = e->suivant;
    }

    if (bv->information.membresAffectes) {
        printf("ERREUR : impossible de supprimer ce BV : ses membres sont encore affectes.\n");
        printf("Affectez d'abord les membres a un autre processus ou recreez le BV si necessaire.\n");
        return;
    }

    if (!confirmer("Confirmer la suppression du BV")) return;
    *debut = supprimer_bv_liste(*debut, id, &ok);
    if (ok) {
        sauvegarder_bv(*debut);
        printf("Bureau supprime avec succes.\n");
    }
}
