/* ================================================================
   MODULE PARTIS POLITIQUES
   Fichier : ge_parti.c
   ================================================================ */
#include "election.h"

static int parti_id_existe(MaillonParti *debut, const char *id) {
/* Recherche un parti par son identifiant. */
    return rechercher_parti(debut, id) != NULL;
}

/* Insere un parti a la fin de la liste chainee. */
MaillonParti *inserer_parti(MaillonParti *debut, Parti p) {
    MaillonParti *nouveau = (MaillonParti *)malloc(sizeof(MaillonParti));
    MaillonParti *pCourant;

    if (nouveau == NULL) {
        return NULL;
    }

    nouveau->information = p;
    nouveau->suivant = NULL;

    if (debut == NULL) {
        return nouveau;
    }

    /* Insertion a la fin, conforme au schema du cours sur les listes. */
    pCourant = debut;
    while (pCourant->suivant != NULL) {
        pCourant = pCourant->suivant;
    }
    pCourant->suivant = nouveau;
    return debut;
}

/* Recherche un parti par son identifiant. */
MaillonParti *rechercher_parti(MaillonParti *debut, const char *id) {
    while (debut != NULL) {
        if (strcmp(debut->information.idPP, id) == 0) {
            return debut;
        }
        debut = debut->suivant;
    }
    return NULL;
}

/* Supprime un parti d'une liste chainee et libere son maillon. */
MaillonParti *supprimer_parti_liste(MaillonParti *debut, const char *id, int *ok) {
    MaillonParti *courant = debut;
    MaillonParti *precedent = NULL;

    *ok = 0;
    while (courant != NULL && strcmp(courant->information.idPP, id) != 0) {
        precedent = courant;
        courant = courant->suivant;
    }

    if (courant == NULL) {
        return debut;
    }

    if (precedent == NULL) {
        debut = courant->suivant;
    } else {
        precedent->suivant = courant->suivant;
    }

    free(courant);
    *ok = 1;
    return debut;
}

/* Libere toute la memoire de la liste des partis. */
void liberer_partis(MaillonParti **debut) {
    MaillonParti *p = *debut;
    MaillonParti *temp;
    while (p != NULL) {
        temp = p;
        p = p->suivant;
        free(temp);
    }
    *debut = NULL;
}

/* Teste si la liste des partis est vide. */
int liste_partis_vide(MaillonParti *debut) {
    return (debut == NULL);
}

/* Charge les partis depuis p_pol.dat vers une liste dynamique. */
int charger_partis(MaillonParti **debut) {
    FILE *f = fopen(FICHIER_PP, "rb");
    Parti p;
    MaillonParti *nouvelleListe = NULL;
    MaillonParti *tmp;
    int total = 0;

    if (f == NULL) {
        return 0;
    }

    while (fread(&p, sizeof(Parti), 1, f) == 1) {
/* Insere un parti a la fin de la liste chainee. */
        tmp = inserer_parti(nouvelleListe, p);
        if (tmp == NULL) {
/* Libere toute la memoire de la liste des partis. */
            liberer_partis(&nouvelleListe);
            fclose(f);
            return -1;
        }
        nouvelleListe = tmp;
        total++;
    }

    fclose(f);
/* Libere toute la memoire de la liste des partis. */
    liberer_partis(debut);
    *debut = nouvelleListe;
    return total;
}

/* Sauvegarde la liste des partis dans p_pol.dat. */
int sauvegarder_partis(MaillonParti *debut) {
    FILE *f = fopen(FICHIER_PP, "wb");
    if (f == NULL) {
        return 0;
    }

    while (debut != NULL) {
        if (fwrite(&debut->information, sizeof(Parti), 1, f) != 1) {
            fclose(f);
            return 0;
        }
        debut = debut->suivant;
    }
    fclose(f);
    return 1;
}

/* Affiche les informations d'un seul parti. */
void afficher_parti(const Parti *p) {
    printf("ID PP           : %s\n", p->idPP);
    printf("Nom             : %s\n", p->nom);
    printf("Date creation   : "); afficher_date(p->dateCreation); printf("\n");
    printf("Date inscription: "); afficher_date(p->dateInscription); printf("\n");
    printf("Adresse         : "); afficher_adresse(&p->adresse); printf("\n");
    printf("Responsable     : %s\n", p->responsable);
    printf("Telephone       : %s\n", p->telephone);
    printf("Email           : %s\n", p->email);
}

/* Affiche tous les partis sous forme de tableau. */
void afficher_partis(MaillonParti *debut) {
    if (liste_partis_vide(debut)) {
        printf("Aucun parti politique enregistre.\n");
        return;
    }

    printf("\n================ LISTE DES PARTIS POLITIQUES ================\n\n");
    printf("%-10s %-22s %-12s %-12s %-30s %-22s %-12s %-30s\n",
           "ID_PP", "NOM", "CREATION", "INSCRIPTION", "ADRESSE", "RESPONSABLE", "TELEPHONE", "EMAIL");

    while (debut != NULL) {
        char dateCreation[16], dateInscription[16], adresse[180];
        snprintf(dateCreation, sizeof(dateCreation), "%02d/%02d/%04d",
                 debut->information.dateCreation.jour,
                 debut->information.dateCreation.mois,
                 debut->information.dateCreation.annee);
        snprintf(dateInscription, sizeof(dateInscription), "%02d/%02d/%04d",
                 debut->information.dateInscription.jour,
                 debut->information.dateInscription.mois,
                 debut->information.dateInscription.annee);
        snprintf(adresse, sizeof(adresse), "%d, %s, %s",
                 debut->information.adresse.noRue,
                 debut->information.adresse.rue,
                 debut->information.adresse.departement);

        printf("%-10s %-22.22s %-12s %-12s %-30.30s %-22.22s %-12s %-30.30s\n",
               debut->information.idPP,
               debut->information.nom,
               dateCreation,
               dateInscription,
               adresse,
               debut->information.responsable,
               debut->information.telephone,
               debut->information.email);
        debut = debut->suivant;
    }
}
/* Enregistre un nouveau parti avec toutes les validations. */
void ins_PP(MaillonParti **debut) {
    Parti p;
    char saisie[100];
    MaillonParti *tmp;

    printf("\n========== INSCRIPTION D'UN PARTI POLITIQUE ==========\n");

    do {
        saisir_chaine_obligatoire("ID_PP (2 lettres + 6 chiffres) : ", saisie, sizeof(saisie));
        for (size_t i = 0; i < strlen(saisie); ++i) saisie[i] = (char)toupper((unsigned char)saisie[i]);
        if (!valider_id_format(saisie)) {
            printf("ERREUR : l'ID doit contenir exactement 8 caracteres : 2 lettres et 6 chiffres.\n");
        } else if (parti_id_existe(*debut, saisie)) {
            printf("ERREUR : cet ID_PP existe deja. Veuillez en choisir un autre.\n");
        } else {
            break;
        }
    } while (1);
    strcpy(p.idPP, saisie);

    saisir_chaine_obligatoire("Nom du parti : ", p.nom, sizeof(p.nom));
    date_saisir("Date de creation (JJ/MM/AAAA) : ", &p.dateCreation, 1);

    do {
        date_saisir("Date d'inscription (JJ/MM/AAAA) : ", &p.dateInscription, 1);
        if (comparer_dates(p.dateCreation, p.dateInscription) > 0) {
            printf("ERREUR : la date d'inscription ne peut pas preceder la creation du parti.\n");
        } else {
            break;
        }
    } while (1);

    saisir_adresse(&p.adresse);
    saisir_chaine_obligatoire("Nom du responsable : ", p.responsable, sizeof(p.responsable));

    do {
        saisir_chaine_obligatoire("Telephone (8 chiffres) : ", saisie, sizeof(saisie));
        if (!valider_telephone(saisie)) {
            printf("ERREUR : le telephone doit contenir exactement 8 chiffres.\n");
        } else {
            strcpy(p.telephone, saisie);
            break;
        }
    } while (1);

    do {
        saisir_chaine_obligatoire("Email Gmail : ", p.email, sizeof(p.email));
        if (!valider_email_gmail(p.email)) {
            printf("ERREUR : utilisez une adresse de la forme nom@gmail.com, sans espace.\n");
        } else {
            break;
        }
    } while (1);

/* Insere un parti a la fin de la liste chainee. */
    tmp = inserer_parti(*debut, p);
    if (tmp == NULL) {
        printf("ERREUR : allocation memoire insuffisante.\n");
        return;
    }
    *debut = tmp;

/* Sauvegarde la liste des partis dans p_pol.dat. */
    if (!sauvegarder_partis(*debut)) {
        printf("AVERTISSEMENT : enregistrement en memoire reussi, mais sauvegarde fichier echouee.\n");
    } else {
        printf("Parti politique enregistre avec succes.\n");
    }
}

/* Modifie uniquement le champ choisi d'un parti. */
void modif_PP(MaillonParti *debut) {
    char id[TAILLE_ID], saisie[160];
    MaillonParti *noeud;
    int choix;

    printf("\n========== MODIFICATION D'UN PARTI ==========\n");
    saisir_chaine_obligatoire("ID_PP a modifier : ", id, sizeof(id));
    for (size_t i = 0; i < strlen(id); ++i) id[i] = (char)toupper((unsigned char)id[i]);
/* Recherche un parti par son identifiant. */
    noeud = rechercher_parti(debut, id);

    if (noeud == NULL) {
        printf("Aucun parti correspondant a cet ID.\n");
        return;
    }

    do {
        printf("\n1. Nom\n2. Date creation\n3. Date inscription\n4. Adresse\n5. Responsable\n6. Telephone\n7. Email\n0. Retour\n");
        printf("Votre choix : ");
        if (scanf("%d", &choix) != 1) { vider_buffer(); choix = -1; }
        vider_buffer();

        switch (choix) {
            case 1: saisir_chaine_obligatoire("Nouveau nom : ", noeud->information.nom, sizeof(noeud->information.nom)); break;
            case 2:
                date_saisir("Nouvelle date de creation : ", &noeud->information.dateCreation, 1);
                while (comparer_dates(noeud->information.dateCreation, noeud->information.dateInscription) > 0) {
                    printf("ERREUR : la creation ne peut pas etre apres l'inscription.\n");
                    date_saisir("Nouvelle date de creation : ", &noeud->information.dateCreation, 1);
                }
                break;
            case 3:
                date_saisir("Nouvelle date d'inscription : ", &noeud->information.dateInscription, 1);
                while (comparer_dates(noeud->information.dateCreation, noeud->information.dateInscription) > 0) {
                    printf("ERREUR : l'inscription ne peut pas preceder la creation.\n");
                    date_saisir("Nouvelle date d'inscription : ", &noeud->information.dateInscription, 1);
                }
                break;
            case 4: saisir_adresse(&noeud->information.adresse); break;
            case 5: saisir_chaine_obligatoire("Nouveau responsable : ", noeud->information.responsable, sizeof(noeud->information.responsable)); break;
            case 6:
                do {
                    saisir_chaine_obligatoire("Nouveau telephone (8 chiffres) : ", saisie, sizeof(saisie));
                    if (!valider_telephone(saisie)) printf("ERREUR : telephone invalide.\n");
                } while (!valider_telephone(saisie));
                strcpy(noeud->information.telephone, saisie);
                break;
            case 7:
                do {
                    saisir_chaine_obligatoire("Nouvel email Gmail : ", noeud->information.email, sizeof(noeud->information.email));
                    if (!valider_email_gmail(noeud->information.email)) printf("ERREUR : adresse Gmail invalide.\n");
                } while (!valider_email_gmail(noeud->information.email));
                break;
            case 0: return;
            default: printf("Choix invalide.\n");
        }

        if (choix >= 1 && choix <= 7) {
            MaillonParti *racine = noeud;
            (void)racine;
            /* Le noeud est deja dans la liste : on sauvegarde simplement. */
            /* Le pointeur debut n'est pas necessaire pour reecrire le fichier. */
            printf("Modification effectuee.\n");
            /* Le fichier est recalcule depuis le debut via une recherche d'ancrage impossible ici.
               Pour eviter une variable globale, le menu appellera sauvegarder apres modification. */
        }
    } while (choix != 0);
}

/* Supprime un parti apres verification des dependances. */
void suppr_PP(MaillonParti **debut, MaillonCandidat *listeCandidats) {
    char id[TAILLE_ID];
    int ok = 0;
    MaillonCandidat *c;

    printf("\n========== SUPPRESSION D'UN PARTI ==========\n");
    saisir_chaine_obligatoire("ID_PP a supprimer : ", id, sizeof(id));
    for (size_t i = 0; i < strlen(id); ++i) id[i] = (char)toupper((unsigned char)id[i]);

/* Recherche un parti par son identifiant. */
    if (rechercher_parti(*debut, id) == NULL) {
        printf("Parti introuvable.\n");
        return;
    }

    c = listeCandidats;
    while (c != NULL) {
        if (strcmp(c->information.type, id) == 0) {
            printf("ERREUR : impossible de supprimer ce parti : un candidat y est rattache.\n");
            return;
        }
        c = c->suivant;
    }

    if (!confirmer("Confirmer la suppression")) return;

    *debut = supprimer_parti_liste(*debut, id, &ok);
    if (ok) {
/* Sauvegarde la liste des partis dans p_pol.dat. */
        sauvegarder_partis(*debut);
        printf("Parti supprime avec succes.\n");
    }
}
