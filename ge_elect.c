/* ================================================================
   MODULE ELECTEURS
   Fichier : ge_elect.c
   ================================================================ */
#include "election.h"

/* Insere un electeur a la fin de la liste. */
MaillonElecteur *inserer_electeur(MaillonElecteur *debut, Electeur e) {
    MaillonElecteur *nouveau=(MaillonElecteur*)malloc(sizeof(MaillonElecteur));
    MaillonElecteur *p;
    if(nouveau==NULL) return NULL;
    nouveau->information=e; nouveau->suivant=NULL;
    if(debut==NULL) return nouveau;
    p=debut; while(p->suivant!=NULL) p=p->suivant;
    p->suivant=nouveau; return debut;
}

/* Recherche un electeur par son NINU. */
MaillonElecteur *rechercher_electeur(MaillonElecteur *debut, const char *ninu) {
    while(debut!=NULL){ if(strcmp(debut->information.ninu,ninu)==0) return debut; debut=debut->suivant; }
    return NULL;
}

/* Supprime un electeur d'une liste chainee. */
MaillonElecteur *supprimer_electeur_liste(MaillonElecteur *debut,const char *ninu,int *ok){
    MaillonElecteur *c=debut,*prec=NULL; *ok=0;
    while(c!=NULL && strcmp(c->information.ninu,ninu)!=0){prec=c;c=c->suivant;}
    if(c==NULL) return debut;
    if(prec==NULL) debut=c->suivant; else prec->suivant=c->suivant;
    free(c); *ok=1; return debut;
}

/* Libere toute la liste des electeurs. */
void liberer_electeurs(MaillonElecteur **debut){
    MaillonElecteur *p=*debut,*tmp; while(p!=NULL){tmp=p;p=p->suivant;free(tmp);} *debut=NULL;
}

/* Charge les electeurs depuis elect.dat. */
int charger_electeurs(MaillonElecteur **debut){
    FILE *f=fopen(FICHIER_ELECT,"rb"); Electeur e; MaillonElecteur *liste=NULL,*tmp; int n=0;
    if(f==NULL) return 0;
/* Insere un electeur a la fin de la liste. */
    while(fread(&e,sizeof(Electeur),1,f)==1){tmp=inserer_electeur(liste,e); if(tmp==NULL){liberer_electeurs(&liste);fclose(f);return -1;} liste=tmp;n++;}
/* Libere toute la liste des electeurs. */
    fclose(f);liberer_electeurs(debut);*debut=liste;return n;
}

/* Sauvegarde les electeurs dans elect.dat. */
int sauvegarder_electeurs(MaillonElecteur *debut){
    FILE *f=fopen(FICHIER_ELECT,"wb"); if(f==NULL)return 0;
    while(debut!=NULL){if(fwrite(&debut->information,sizeof(Electeur),1,f)!=1){fclose(f);return 0;}debut=debut->suivant;}
    fclose(f);return 1;
}

/* Affiche les informations d'un seul electeur. */
void afficher_electeur(const Electeur *e) {
    printf("NINU            : %s\n", e->ninu);
    printf("Nom             : %s\n", e->nom);
    printf("Prenom          : %s\n", e->prenom);
    printf("Naissance       : "); afficher_date(e->dateNaissance);
    printf(" (age : %d)\n", age_aujourd_hui(e->dateNaissance));
    printf("Adresse         : "); afficher_adresse(&e->adresse); printf("\n");
    printf("Bureau de vote  : %s\n", e->idBV);
}

/* Affiche tous les electeurs sous forme de tableau. */
void afficher_electeurs(MaillonElecteur *debut) {
    if (debut == NULL) {
        printf("Aucun electeur enregistre.\n");
        return;
    }

    printf("\n================ LISTE DES ELECTEURS ================\n\n");
    printf("%-12s %-20s %-20s %-12s %-30s %-10s\n",
           "NINU", "NOM", "PRENOM", "NAISSANCE", "ADRESSE", "ID_BV");

    while (debut != NULL) {
        char naissance[16], adresse[180];
        snprintf(naissance, sizeof(naissance), "%02d/%02d/%04d",
                 debut->information.dateNaissance.jour,
                 debut->information.dateNaissance.mois,
                 debut->information.dateNaissance.annee);
        snprintf(adresse, sizeof(adresse), "%d, %s, %s",
                 debut->information.adresse.noRue,
                 debut->information.adresse.rue,
                 debut->information.adresse.departement);

        printf("%-12s %-20.20s %-20.20s %-12s %-30.30s %-10s\n",
               debut->information.ninu,
               debut->information.nom,
               debut->information.prenom,
               naissance,
               adresse,
               debut->information.idBV);
        debut = debut->suivant;
    }
}
static void saisir_ninu_unique(MaillonElecteur *liste,char *ninu){
    do{
        saisir_chaine_obligatoire("NINU (exactement 10 chiffres) : ",ninu,TAILLE_NINU);
        if(!valider_ninu_format(ninu)) printf("ERREUR : le NINU doit contenir exactement 10 chiffres.\n");
/* Recherche un electeur par son NINU. */
        else if(rechercher_electeur(liste,ninu)!=NULL) printf("ERREUR : ce NINU existe deja.\n");
        else return;
    }while(1);
}

static void saisir_bv_option(Electeur *e, MaillonBV *listeBV){
    do{
        saisir_chaine_obligatoire("ID du Bureau de Vote : ",e->idBV,sizeof(e->idBV));
        for(size_t i=0;i<strlen(e->idBV);++i)e->idBV[i]=(char)toupper((unsigned char)e->idBV[i]);
        if(!valider_id_format(e->idBV)) printf("ERREUR : ID_BV invalide.\n");
        else if(rechercher_bv(listeBV,e->idBV)==NULL) printf("ERREUR : ce bureau n'existe pas.\n");
        else return;
    }while(1);
}

/* Inscrit un electeur avec controle du NINU, de l'age et du BV. */
void ins_elect(MaillonElecteur **debut, MaillonBV *listeBV){
    Electeur e;MaillonElecteur *tmp;
    printf("\n========== INSCRIPTION D'UN ELECTEUR ==========\n");
    saisir_ninu_unique(*debut,e.ninu);
    saisir_chaine_obligatoire("Nom : ",e.nom,sizeof(e.nom));
    saisir_chaine_obligatoire("Prenom : ",e.prenom,sizeof(e.prenom));
    date_saisir_age_minimum("Date de naissance (JJ/MM/AAAA) : ",&e.dateNaissance,18);
    saisir_adresse(&e.adresse);
    if(listeBV==NULL){printf("ERREUR : aucun Bureau de Vote n'est disponible. Vous devez creer un BV avant d'assigner un electeur.\n");return;}
    saisir_bv_option(&e,listeBV);
/* Insere un electeur a la fin de la liste. */
    tmp=inserer_electeur(*debut,e);
    if(tmp==NULL){printf("ERREUR : memoire insuffisante.\n");return;}
    *debut=tmp;
    if(sauvegarder_electeurs(*debut)) printf("Electeur enregistre avec succes.\n"); else printf("AVERTISSEMENT : sauvegarde impossible.\n");
}

/* Modifie uniquement le champ choisi d'un electeur. */
void modif_elect(MaillonElecteur *debut, MaillonBV *listeBV){
    char ninu[TAILLE_NINU];MaillonElecteur *n;int choix;
    saisir_chaine_obligatoire("NINU de l'electeur a modifier : ",ninu,sizeof(ninu));
    if(!valider_ninu_format(ninu)){printf("NINU invalide.\n");return;}
/* Recherche un electeur par son NINU. */
    n=rechercher_electeur(debut,ninu);
    if(n==NULL){printf("Electeur introuvable.\n");return;}
    do{
        printf("\n1. Nom\n2. Prenom\n3. Date de naissance\n4. Adresse\n5. Bureau de vote\n0. Retour\nVotre choix : ");
        if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();
        switch(choix){
            case 1:saisir_chaine_obligatoire("Nouveau nom : ",n->information.nom,sizeof(n->information.nom));break;
            case 2:saisir_chaine_obligatoire("Nouveau prenom : ",n->information.prenom,sizeof(n->information.prenom));break;
            case 3:date_saisir_age_minimum("Nouvelle date de naissance : ",&n->information.dateNaissance,18);break;
            case 4:saisir_adresse(&n->information.adresse);break;
            case 5:saisir_bv_option(&n->information,listeBV);break;
            case 0:return;
            default:printf("Choix invalide.\n");
        }
        if(choix>=1&&choix<=5)printf("Modification effectuee.\n");
    }while(choix!=0);
}

/* Supprime un electeur apres controles d'integrite. */
void suppr_elect(MaillonElecteur **debut, MaillonBV *listeBV, MaillonVote *listeVotes){
    char ninu[TAILLE_NINU];int ok;MaillonBV *bv;
    saisir_chaine_obligatoire("NINU de l'electeur a supprimer : ",ninu,sizeof(ninu));
/* Recherche un electeur par son NINU. */
    if(rechercher_electeur(*debut,ninu)==NULL){printf("Electeur introuvable.\n");return;}
    /* Verifier qu'il n'est pas membre d'un bureau. */
    bv=listeBV; while(bv!=NULL){
        if(strcmp(bv->information.idPresident,ninu)==0||strcmp(bv->information.idVicePresident,ninu)==0||strcmp(bv->information.idSecretaire,ninu)==0){
            printf("ERREUR : cet electeur est membre d'un Bureau de Vote.\n");return;
        } bv=bv->suivant;
    }
    if(rechercher_vote_electeur(listeVotes,ninu)!=NULL){printf("ERREUR : cet electeur a deja vote et ne peut pas etre supprime sans casser l'historique des votes.\n");return;}
    if(!confirmer("Confirmer la suppression de l'electeur"))return;
    *debut=supprimer_electeur_liste(*debut,ninu,&ok);
    if(ok){sauvegarder_electeurs(*debut);printf("Electeur supprime avec succes.\n");}
}
