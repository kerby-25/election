/* ================================================================
   MODULE CANDIDATS
   Fichier : ge_candi.c
   ================================================================ */
#include "election.h"

/* Insere un candidat a la fin de la liste. */
MaillonCandidat *inserer_candidat(MaillonCandidat *debut,Candidat c){
    MaillonCandidat *nouveau=(MaillonCandidat*)malloc(sizeof(MaillonCandidat));
    MaillonCandidat *p;
    if(nouveau==NULL) return NULL;
    nouveau->information=c;
    nouveau->suivant=NULL;
    if(debut==NULL) return nouveau;
    p=debut;
    while(p->suivant!=NULL) p=p->suivant;
    p->suivant=nouveau;
    return debut;
}

/* Recherche un candidat par ID. */
MaillonCandidat *rechercher_candidat_id(MaillonCandidat *debut,const char *id){
    while(debut!=NULL){if(strcmp(debut->information.idCandidat,id)==0)return debut;debut=debut->suivant;}return NULL;
}

/* Recherche un candidat par NINU. */
MaillonCandidat *rechercher_candidat_ninu(MaillonCandidat *debut,const char *ninu){
    while(debut!=NULL){if(strcmp(debut->information.ninu,ninu)==0)return debut;debut=debut->suivant;}return NULL;
}

/* Supprime un candidat d'une liste chainee. */
MaillonCandidat *supprimer_candidat_liste(MaillonCandidat *debut,const char *id,int *ok){
    MaillonCandidat *c=debut,*prec=NULL;
    *ok=0;
    while(c!=NULL&&strcmp(c->information.idCandidat,id)!=0){prec=c;c=c->suivant;}
    if(c==NULL) return debut;
    if(prec==NULL) debut=c->suivant;
    else prec->suivant=c->suivant;
    free(c);
    *ok=1;
    return debut;
}

/* Libere toute la liste des candidats. */
void liberer_candidats(MaillonCandidat **debut){MaillonCandidat*p=*debut,*tmp;while(p!=NULL){tmp=p;p=p->suivant;free(tmp);}*debut=NULL;}

/* Charge les candidats depuis candid.dat. */
int charger_candidats(MaillonCandidat **debut){
    FILE*f=fopen(FICHIER_CAND,"rb");Candidat c;MaillonCandidat*liste=NULL,*tmp;int n=0;if(f==NULL)return 0;
/* Insere un candidat a la fin de la liste. */
    while(fread(&c,sizeof(Candidat),1,f)==1){tmp=inserer_candidat(liste,c);if(tmp==NULL){liberer_candidats(&liste);fclose(f);return -1;}liste=tmp;n++;}
/* Libere toute la liste des candidats. */
    fclose(f);liberer_candidats(debut);*debut=liste;return n;
}

/* Sauvegarde les candidats dans candid.dat. */
int sauvegarder_candidats(MaillonCandidat*debut){FILE*f=fopen(FICHIER_CAND,"wb");if(f==NULL)return 0;while(debut!=NULL){if(fwrite(&debut->information,sizeof(Candidat),1,f)!=1){fclose(f);return 0;}debut=debut->suivant;}fclose(f);return 1;}

/* Affiche les informations d'un seul candidat. */
void afficher_candidat(const Candidat *c) {
    printf("ID candidat     : %s\n", c->idCandidat);
    printf("NINU            : %s\n", c->ninu);
    printf("Nom             : %s\n", c->nom);
    printf("Prenom          : %s\n", c->prenom);
    printf("Naissance       : "); afficher_date(c->dateNaissance);
    printf(" (age : %d)\n", age_aujourd_hui(c->dateNaissance));
    printf("Adresse         : "); afficher_adresse(&c->adresse); printf("\n");
    printf("Email           : %s\n", c->email);
    printf("Telephone       : %s\n", c->telephone);
    printf("Type            : %s\n", strcmp(c->type, CODE_INDEPENDANT) == 0 ? "Independant" : c->type);
    printf("Date inscription: "); afficher_date(c->dateInscription); printf("\n");
}

/* Affiche tous les candidats sous forme de tableau. */
void afficher_candidats(MaillonCandidat *debut) {
    if (debut == NULL) {
        printf("Aucun candidat.\n");
        return;
    }

    printf("\n================ LISTE DES CANDIDATS ================\n\n");
    printf("%-10s %-12s %-20s %-20s %-12s %-18s\n",
           "ID", "NINU", "NOM", "PRENOM", "TYPE", "INSCRIPTION");

    while (debut != NULL) {
        char dateInscription[16];
        snprintf(dateInscription, sizeof(dateInscription), "%02d/%02d/%04d",
                 debut->information.dateInscription.jour,
                 debut->information.dateInscription.mois,
                 debut->information.dateInscription.annee);

        printf("%-10s %-12s %-20.20s %-20.20s %-12.12s %-18s\n",
               debut->information.idCandidat,
               debut->information.ninu,
               debut->information.nom,
               debut->information.prenom,
               strcmp(debut->information.type, CODE_INDEPENDANT) == 0 ? "Independant" : debut->information.type,
               dateInscription);
        debut = debut->suivant;
    }
}
static int id_candidat_valide_unique(MaillonCandidat*liste,char*id){
    do{ saisir_chaine_obligatoire("Id_candidat (2 lettres + 6 chiffres) : ",id,TAILLE_ID);for(size_t i=0;i<strlen(id);++i)id[i]=(char)toupper((unsigned char)id[i]);
/* Recherche un candidat par ID. */
        if(!valider_id_format(id))printf("ERREUR : format invalide.\n");else if(rechercher_candidat_id(liste,id)!=NULL)printf("ERREUR : ID deja utilise.\n");else return 1;
    }while(1);
}

static void saisir_ninu_candidat_unique(MaillonCandidat*liste,char*ninu){
/* Recherche un candidat par NINU. */
    do{saisir_chaine_obligatoire("NINU du candidat (10 chiffres) : ",ninu,TAILLE_NINU);if(!valider_ninu_format(ninu))printf("ERREUR : le NINU doit contenir exactement 10 chiffres.\n");else if(rechercher_candidat_ninu(liste,ninu)!=NULL)printf("ERREUR : ce NINU est deja associe a un candidat.\n");else return;}while(1);
}

/* Inscrit un candidat avec controle de l'age et du type. */
void ins_cand(MaillonCandidat**debut,MaillonParti*listePartis){
    Candidat c;char choixType[50],saisie[100];MaillonCandidat*tmp;MaillonParti*p;
    printf("\n========== INSCRIPTION D'UN CANDIDAT ==========\n");
    id_candidat_valide_unique(*debut,c.idCandidat);saisir_ninu_candidat_unique(*debut,c.ninu);
    saisir_chaine_obligatoire("Nom : ",c.nom,sizeof(c.nom));saisir_chaine_obligatoire("Prenom : ",c.prenom,sizeof(c.prenom));
    date_saisir_age_minimum("Date de naissance (minimum 25 ans) : ",&c.dateNaissance,25);saisir_adresse(&c.adresse);
    do{saisir_chaine_obligatoire("Email Gmail : ",c.email,sizeof(c.email));if(!valider_email_gmail(c.email))printf("ERREUR : Gmail invalide.\n");}while(!valider_email_gmail(c.email));
    do{saisir_chaine_obligatoire("Telephone (8 chiffres) : ",saisie,sizeof(saisie));if(!valider_telephone(saisie))printf("ERREUR : telephone invalide.\n");}while(!valider_telephone(saisie));strcpy(c.telephone,saisie);
    do{
        printf("Type de candidat : 0 = Independant ; sinon saisir l'Id_PP du parti.\n");
        saisir_chaine_obligatoire("Type : ",choixType,sizeof(choixType));
        if(strcmp(choixType,CODE_INDEPENDANT)==0){strcpy(c.type,CODE_INDEPENDANT);break;}
        for(size_t i=0;i<strlen(choixType);++i)choixType[i]=(char)toupper((unsigned char)choixType[i]);
        if(!valider_id_format(choixType)){printf("ERREUR : Id_PP invalide.\n");continue;}
        p=rechercher_parti(listePartis,choixType);if(p==NULL)printf("ERREUR : ce parti politique n'existe pas.\n");else{strcpy(c.type,choixType);break;}
    }while(1);
    date_saisir("Date d'inscription (JJ/MM/AAAA) : ",&c.dateInscription,1);
/* Insere un candidat a la fin de la liste. */
    tmp=inserer_candidat(*debut,c);if(tmp==NULL){printf("ERREUR : memoire insuffisante.\n");return;}*debut=tmp;
/* Sauvegarde les candidats dans candid.dat. */
    if(sauvegarder_candidats(*debut))printf("Candidat inscrit avec succes.\n");else printf("AVERTISSEMENT : sauvegarde impossible.\n");
}

/* Modifie uniquement le champ choisi d'un candidat. */
void modif_cand(MaillonCandidat*debut,MaillonParti*listePartis){
    char id[TAILLE_ID],saisie[120];MaillonCandidat*n;int choix;MaillonParti*p;
    saisir_chaine_obligatoire("ID candidat a modifier : ",id,sizeof(id));for(size_t i=0;i<strlen(id);++i)id[i]=(char)toupper((unsigned char)id[i]);
/* Recherche un candidat par ID. */
    n=rechercher_candidat_id(debut,id);if(n==NULL){printf("Candidat introuvable.\n");return;}
    do{
        printf("\n1. NINU\n2. Nom\n3. Prenom\n4. Date naissance\n5. Adresse\n6. Email\n7. Telephone\n8. Type\n9. Date inscription\n0. Retour\nVotre choix : ");
        if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();
        switch(choix){
/* Recherche un candidat par NINU. */
            case 1: do{saisir_chaine_obligatoire("Nouveau NINU : ",saisie,sizeof(saisie));if(!valider_ninu_format(saisie))printf("ERREUR : NINU invalide.\n");else if(rechercher_candidat_ninu(debut,saisie)!=NULL && strcmp(rechercher_candidat_ninu(debut,saisie)->information.idCandidat,id)!=0)printf("ERREUR : NINU deja utilise.\n");else break;}while(1);strcpy(n->information.ninu,saisie);break;
            case 2:saisir_chaine_obligatoire("Nouveau nom : ",n->information.nom,sizeof(n->information.nom));break;
            case 3:saisir_chaine_obligatoire("Nouveau prenom : ",n->information.prenom,sizeof(n->information.prenom));break;
            case 4:date_saisir_age_minimum("Nouvelle date de naissance : ",&n->information.dateNaissance,25);break;
            case 5:saisir_adresse(&n->information.adresse);break;
            case 6:do{saisir_chaine_obligatoire("Nouvel email Gmail : ",n->information.email,sizeof(n->information.email));if(!valider_email_gmail(n->information.email))printf("ERREUR : Gmail invalide.\n");}while(!valider_email_gmail(n->information.email));break;
            case 7:do{saisir_chaine_obligatoire("Nouveau telephone : ",saisie,sizeof(saisie));if(!valider_telephone(saisie))printf("ERREUR : telephone invalide.\n");}while(!valider_telephone(saisie));strcpy(n->information.telephone,saisie);break;
            case 8:
                do{saisir_chaine_obligatoire("Nouveau type (0 ou Id_PP) : ",saisie,sizeof(saisie));if(strcmp(saisie,CODE_INDEPENDANT)==0){strcpy(n->information.type,CODE_INDEPENDANT);break;}for(size_t i=0;i<strlen(saisie);++i)saisie[i]=(char)toupper((unsigned char)saisie[i]);
                    p=rechercher_parti(listePartis,saisie);if(!valider_id_format(saisie)||p==NULL)printf("ERREUR : parti inexistant ou identifiant invalide.\n");else{strcpy(n->information.type,saisie);break;}
                }while(1);break;
            case 9:date_saisir("Nouvelle date d'inscription : ",&n->information.dateInscription,1);break;
            case 0:return;
            default:printf("Choix invalide.\n");
        }
        if(choix>=1&&choix<=9)printf("Modification effectuee.\n");
    }while(choix!=0);
}

/* Recherche un candidat par ID. */
void suppr_cand(MaillonCandidat**debut,MaillonVote*listeVotes){char id[TAILLE_ID];int ok;saisir_chaine_obligatoire("ID candidat a supprimer : ",id,sizeof(id));for(size_t i=0;i<strlen(id);++i)id[i]=(char)toupper((unsigned char)id[i]);if(rechercher_candidat_id(*debut,id)==NULL){printf("Candidat introuvable.\n");return;}MaillonVote*v=listeVotes;while(v!=NULL){if(strcmp(v->information.idCandidat,id)==0){printf("ERREUR : ce candidat possede deja des votes.\n");return;}v=v->suivant;}if(!confirmer("Confirmer la suppression du candidat"))return;*debut=supprimer_candidat_liste(*debut,id,&ok);if(ok){sauvegarder_candidats(*debut);printf("Candidat supprime.\n");}}

/* Affiche toutes les informations d'un candidat. */
void afficher_candidats_par_nom(MaillonCandidat*debut,const char*nom){int trouve=0;while(debut!=NULL){if(egale_sans_casse(debut->information.nom,nom)){printf("\n--------------------------------\n");afficher_candidat(&debut->information);trouve=1;}debut=debut->suivant;}if(!trouve)printf("Aucun candidat ne correspond a ce nom.\n");}
