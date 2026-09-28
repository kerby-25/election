/* ================================================================
   MODULE RAPPORTS ET RESULTATS
   Fichier : rapp.c
   ================================================================ */
#include "election.h"

typedef struct NoeudResultat {
    char idCandidat[TAILLE_ID];
    char nom[TAILLE_NOM];
    char prenom[TAILLE_PRENOM];
    int totalVotes;
    double pourcentage;
    struct NoeudResultat *suivant;
} NoeudResultat;

static int vote_blanc_est(const Vote *v) {
    return strcmp(v->idCandidat, VOTE_BLANC) == 0;
}

/* Affiche les electeurs regroupes par departement et bureau de vote. */
void List_Electeurs(MaillonElecteur *listeElecteurs, MaillonBV *listeBV) {
    MaillonBV *bv;

    printf("\n================ LISTE DES ELECTEURS PAR BUREAU ================\n\n");
    if (listeElecteurs == NULL) {
        printf("Aucun electeur.\n");
        return;
    }

    bv = listeBV;
    while (bv != NULL) {
        MaillonElecteur *e = listeElecteurs;
        int trouve = 0;

        printf("\nBUREAU : %s    DEPARTEMENT : %s\n",
               bv->information.idBV, bv->information.adresse.departement);
        printf("%-12s %-22s %-22s %-30s\n",
               "NINU", "NOM", "PRENOM", "ADRESSE");

        while (e != NULL) {
            if (strcmp(e->information.idBV, bv->information.idBV) == 0) {
                char adresse[180];
                snprintf(adresse, sizeof(adresse), "%d, %s, %s",
                         e->information.adresse.noRue,
                         e->information.adresse.rue,
                         e->information.adresse.departement);
                printf("%-12s %-22.22s %-22.22s %-30.30s\n",
                       e->information.ninu,
                       e->information.nom,
                       e->information.prenom,
                       adresse);
                trouve = 1;
            }
            e = e->suivant;
        }
        if (!trouve)
            printf("Aucun electeur affecte a ce BV.\n");

        bv = bv->suivant;
    }
}
/* Affiche la liste des candidats a la presidentielle. */
void List_Candid(MaillonCandidat *listeCandidats) {
    printf("\n================ LISTE DES CANDIDATS ================\n\n");
    if (listeCandidats == NULL) {
        printf("Aucun candidat.\n");
        return;
    }

    printf("%-10s %-20s %-20s %-12s %-18s\n",
           "ID", "NOM", "PRENOM", "NINU", "TYPE");
    while (listeCandidats != NULL) {
        printf("%-10s %-20.20s %-20.20s %-12s %-18.18s\n",
               listeCandidats->information.idCandidat,
               listeCandidats->information.nom,
               listeCandidats->information.prenom,
               listeCandidats->information.ninu,
               strcmp(listeCandidats->information.type, CODE_INDEPENDANT) == 0 ? "Independant" : listeCandidats->information.type);
        listeCandidats = listeCandidats->suivant;
    }
}
static void noms_membres_bv(MaillonElecteur *electeurs, const char *ninu1, const char *ninu2, const char *ninu3,
                            char *nom1, size_t s1, char *nom2, size_t s2, char *nom3, size_t s3) {
    MaillonElecteur *e1 = rechercher_electeur(electeurs, ninu1);
    MaillonElecteur *e2 = rechercher_electeur(electeurs, ninu2);
    MaillonElecteur *e3 = rechercher_electeur(electeurs, ninu3);

    snprintf(nom1, s1, "%s %s", e1 ? e1->information.nom : "INCONNU", e1 ? e1->information.prenom : "");
    snprintf(nom2, s2, "%s %s", e2 ? e2->information.nom : "INCONNU", e2 ? e2->information.prenom : "");
    snprintf(nom3, s3, "%s %s", e3 ? e3->information.nom : "INCONNU", e3 ? e3->information.prenom : "");
}

/* Affiche les bureaux et les noms des membres. */
void List_BV(MaillonBV *listeBV, MaillonElecteur *listeElecteurs) {
    printf("\n================ LISTE DES BUREAUX DE VOTE ================\n\n");
    if (listeBV == NULL) {
        printf("Aucun bureau.\n");
        return;
    }

    printf("%-10s %-30s %-24s %-24s %-24s\n",
           "ID_BV", "ADRESSE", "PRESIDENT", "VICE-PRESIDENT", "SECRETAIRE");

    while (listeBV != NULL) {
        char p1[120], p2[120], p3[120], adresse[180];
        snprintf(adresse, sizeof(adresse), "%d, %s, %s",
                 listeBV->information.adresse.noRue,
                 listeBV->information.adresse.rue,
                 listeBV->information.adresse.departement);

        if (listeBV->information.membresAffectes) {
            noms_membres_bv(listeElecteurs,
                            listeBV->information.idPresident,
                            listeBV->information.idVicePresident,
                            listeBV->information.idSecretaire,
                            p1, sizeof(p1), p2, sizeof(p2), p3, sizeof(p3));
        } else {
            strcpy(p1, "NON AFFECTE");
            strcpy(p2, "NON AFFECTE");
            strcpy(p3, "NON AFFECTE");
        }

        printf("%-10s %-30.30s %-24.24s %-24.24s %-24.24s\n",
               listeBV->information.idBV, adresse, p1, p2, p3);
        listeBV = listeBV->suivant;
    }
}

/* Construit la liste unique des departements rencontres dans les donnees. */
static int collecter_departements_bv(MaillonBV *listeBV, char departements[][TAILLE_DEPT], int maxDepartements) {
    int count = 0;
    while (listeBV != NULL && count < maxDepartements) {
        int existe = 0;
        for (int i = 0; i < count; ++i) {
            if (strcmp(departements[i], listeBV->information.adresse.departement) == 0) {
                existe = 1;
                break;
            }
        }
        if (!existe) {
            strncpy(departements[count], listeBV->information.adresse.departement, TAILLE_DEPT - 1);
            departements[count][TAILLE_DEPT - 1] = '\0';
            ++count;
        }
        listeBV = listeBV->suivant;
    }
    return count;
}

/* Affiche les bureaux de vote regroupes par departement. */
void List_BV_Par_Departement(MaillonBV *listeBV, MaillonElecteur *listeElecteurs) {
    char departements[100][TAILLE_DEPT];
    int nbDepartements;

    printf("\n================ BUREAUX DE VOTE PAR DEPARTEMENT ================\n\n");
    if (listeBV == NULL) {
        printf("Aucun bureau de vote.\n");
        return;
    }

    nbDepartements = collecter_departements_bv(listeBV, departements, 100);
    for (int d = 0; d < nbDepartements; ++d) {
        int trouve = 0;
        printf("DEPARTEMENT : %s\n\n", departements[d]);
        printf("%-10s %-34s %-24s %-24s %-24s\n",
               "ID_BV", "ADRESSE", "PRESIDENT", "VICE-PRESIDENT", "SECRETAIRE");

        for (MaillonBV *bv = listeBV; bv != NULL; bv = bv->suivant) {
            if (strcmp(bv->information.adresse.departement, departements[d]) != 0)
                continue;

            char p1[120], p2[120], p3[120], adresse[150];
            snprintf(adresse, sizeof(adresse), "%d, %s", bv->information.adresse.noRue,
                     bv->information.adresse.rue);

            if (bv->information.membresAffectes) {
                noms_membres_bv(listeElecteurs,
                                bv->information.idPresident,
                                bv->information.idVicePresident,
                                bv->information.idSecretaire,
                                p1, sizeof(p1), p2, sizeof(p2), p3, sizeof(p3));
            } else {
                strcpy(p1, "NON AFFECTE");
                strcpy(p2, "NON AFFECTE");
                strcpy(p3, "NON AFFECTE");
            }

            printf("%-10s %-34.34s %-24.24s %-24.24s %-24.24s\n",
                   bv->information.idBV, adresse, p1, p2, p3);
            trouve = 1;
        }
        if (!trouve)
            printf("Aucun bureau dans ce departement.\n");
        printf("\n");
    }
}

/* Affiche les electeurs regroupes par departement puis par bureau de vote. */
void List_Electeurs_Par_Departement_BV(MaillonElecteur *listeElecteurs, MaillonBV *listeBV) {
    char departements[100][TAILLE_DEPT];
    int nbDepartements = 0;
    MaillonElecteur *e;

    printf("\n================ ELECTEURS PAR DEPARTEMENT ET PAR BV ================\n\n");
    if (listeElecteurs == NULL) {
        printf("Aucun electeur.\n");
        return;
    }

    /* Les departements sont pris directement depuis les adresses des electeurs. */
    e = listeElecteurs;
    while (e != NULL && nbDepartements < 100) {
        int existe = 0;
        for (int i = 0; i < nbDepartements; ++i) {
            if (strcmp(departements[i], e->information.adresse.departement) == 0) {
                existe = 1;
                break;
            }
        }
        if (!existe) {
            strncpy(departements[nbDepartements], e->information.adresse.departement, TAILLE_DEPT - 1);
            departements[nbDepartements][TAILLE_DEPT - 1] = '\0';
            ++nbDepartements;
        }
        e = e->suivant;
    }

    for (int d = 0; d < nbDepartements; ++d) {
        printf("DEPARTEMENT : %s\n\n", departements[d]);

        /* Parcours des BV pour donner un regroupement stable et lisible. */
        for (MaillonBV *bv = listeBV; bv != NULL; bv = bv->suivant) {
            int trouve = 0;
            if (strcmp(bv->information.adresse.departement, departements[d]) != 0)
                continue;

            printf("BUREAU : %s\n", bv->information.idBV);
            printf("%-12s %-22s %-22s %-30s\n", "NINU", "NOM", "PRENOM", "ADRESSE");

            for (e = listeElecteurs; e != NULL; e = e->suivant) {
                if (strcmp(e->information.idBV, bv->information.idBV) != 0)
                    continue;

                char adresse[180];
                snprintf(adresse, sizeof(adresse), "%d, %s",
                         e->information.adresse.noRue,
                         e->information.adresse.rue);
                printf("%-12s %-22.22s %-22.22s %-30.30s\n",
                       e->information.ninu,
                       e->information.nom,
                       e->information.prenom,
                       adresse);
                trouve = 1;
            }

            if (!trouve)
                printf("Aucun electeur affecte a ce BV.\n");
            printf("\n");
        }
    }
}

static void inserer_resultat_trie(NoeudResultat **debut, NoeudResultat *nouveau) {
    NoeudResultat *p, *prec = NULL;
    if (*debut == NULL || nouveau->pourcentage > (*debut)->pourcentage) {
        nouveau->suivant = *debut;
        *debut = nouveau;
        return;
    }
    p = *debut;
    while (p != NULL && p->pourcentage >= nouveau->pourcentage) {
        prec = p;
        p = p->suivant;
    }
    prec->suivant = nouveau;
    nouveau->suivant = p;
}

static void liberer_resultats(NoeudResultat **debut) {
    NoeudResultat *p = *debut, *tmp;
    while (p != NULL) {
        tmp = p;
        p = p->suivant;
        free(tmp);
    }
    *debut = NULL;
}

/* Calcule les resultats par departement et le resume national, puis trie par pourcentage decroissant. */
void Resultats(MaillonBV *listeBV, MaillonCandidat *listeCandidats, MaillonVote *listeVotes) {
    char deptTmp[100][TAILLE_DEPT];
    int dCount = 0;
    MaillonBV *b;
    MaillonVote *v;
    MaillonCandidat *c;

    printf("\n================ RESULTATS DU SCRUTIN PRESIDENTIEL ================\n");
    if (listeVotes == NULL || listeCandidats == NULL) {
        printf("Aucun resultat disponible.\n");
        return;
    }

    b = listeBV;
    while (b != NULL && dCount < 100) {
        int existe = 0;
        int i;
        for (i = 0; i < dCount; i++) {
            if (strcmp(deptTmp[i], b->information.adresse.departement) == 0) {
                existe = 1;
                break;
            }
        }
        if (!existe) {
            strcpy(deptTmp[dCount], b->information.adresse.departement);
            dCount++;
        }
        b = b->suivant;
    }

    for (int d = 0; d < dCount; d++) {
        NoeudResultat *resultats = NULL;
        int blancs = 0, nonBlancs = 0, total;

        for (v = listeVotes; v != NULL; v = v->suivant) {
            MaillonBV *bv = rechercher_bv(listeBV, v->information.idBV);
            if (bv == NULL || strcmp(bv->information.adresse.departement, deptTmp[d]) != 0)
                continue;
            if (vote_blanc_est(&v->information)) blancs++; else nonBlancs++;
        }
        total = blancs + nonBlancs;
        if (total == 0) continue;

        for (c = listeCandidats; c != NULL; c = c->suivant) {
            int nb = 0;
            for (v = listeVotes; v != NULL; v = v->suivant) {
                MaillonBV *bv = rechercher_bv(listeBV, v->information.idBV);
                if (bv != NULL && strcmp(bv->information.adresse.departement, deptTmp[d]) == 0 &&
                    strcmp(v->information.idCandidat, c->information.idCandidat) == 0)
                    nb++;
            }
            if (nb > 0) {
                NoeudResultat *n = (NoeudResultat *)malloc(sizeof(NoeudResultat));
                if (n == NULL) {
                    liberer_resultats(&resultats);
                    printf("ERREUR : memoire insuffisante.\n");
                    return;
                }
                strcpy(n->idCandidat, c->information.idCandidat);
                strcpy(n->nom, c->information.nom);
                strcpy(n->prenom, c->information.prenom);
                n->totalVotes = nb;
                n->pourcentage = (100.0 * nb) / (double)total;
                n->suivant = NULL;
                inserer_resultat_trie(&resultats, n);
            }
        }

        printf("\nDEPARTEMENT : %s\n", deptTmp[d]);
        printf("%-20s %-12s\n", "INDICATEUR", "VALEUR");
        printf("%-20s %-12d\n", "Votes non blancs", nonBlancs);
        printf("%-20s %-12d\n", "Votes blancs", blancs);
        printf("%-20s %-12d\n", "Total des votes", total);
        printf("\nCLASSEMENT DES CANDIDATS\n");
        if (resultats == NULL) {
            printf("Aucun candidat n'a obtenu de vote dans ce departement.\n");
        } else {
            NoeudResultat *r = resultats;
            printf("%-10s %-20s %-20s %-10s %-12s\n",
                   "ID", "NOM", "PRENOM", "VOTES", "POURCENTAGE");
            while (r != NULL) {
                printf("%-10s %-20.20s %-20.20s %-10d %8.2f %%\n",
                       r->idCandidat, r->nom, r->prenom, r->totalVotes, r->pourcentage);
                r = r->suivant;
            }
        }
        liberer_resultats(&resultats);
    }

    /* Resume national. */
    {
        NoeudResultat *nat = NULL;
        int blancs = 0, nonBlancs = 0, total;
        for (v = listeVotes; v != NULL; v = v->suivant) {
            if (vote_blanc_est(&v->information)) blancs++; else nonBlancs++;
        }
        total = blancs + nonBlancs;
        for (c = listeCandidats; c != NULL; c = c->suivant) {
            int nb = 0;
            for (v = listeVotes; v != NULL; v = v->suivant)
                if (strcmp(v->information.idCandidat, c->information.idCandidat) == 0) nb++;
            if (nb > 0) {
                NoeudResultat *n = (NoeudResultat *)malloc(sizeof(NoeudResultat));
                if (n == NULL) {
                    liberer_resultats(&nat);
                    printf("ERREUR : memoire insuffisante.\n");
                    return;
                }
                strcpy(n->idCandidat, c->information.idCandidat);
                strcpy(n->nom, c->information.nom);
                strcpy(n->prenom, c->information.prenom);
                n->totalVotes = nb;
                n->pourcentage = total ? 100.0 * nb / (double)total : 0.0;
                n->suivant = NULL;
                inserer_resultat_trie(&nat, n);
            }
        }

        printf("\n================ RESUME NATIONAL ================\n");
        printf("\n%-20s %-12s\n", "INDICATEUR", "VALEUR");
        printf("%-20s %-12d\n", "Votes non blancs", nonBlancs);
        printf("%-20s %-12d\n", "Votes blancs", blancs);
        printf("%-20s %-12d\n", "Total des votes", total);
        printf("\nCLASSEMENT NATIONAL\n");
        if (nat == NULL) {
            printf("Aucun candidat n'a obtenu de vote.\n");
        } else {
            printf("%-10s %-20s %-20s %-10s %-12s\n",
                   "ID", "NOM", "PRENOM", "VOTES", "POURCENTAGE");
            for (NoeudResultat *r = nat; r != NULL; r = r->suivant)
                printf("%-10s %-20.20s %-20.20s %-10d %8.2f %%\n",
                       r->idCandidat, r->nom, r->prenom, r->totalVotes, r->pourcentage);
        }
        liberer_resultats(&nat);
    }
}
