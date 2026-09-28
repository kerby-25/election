/* ================================================================
   MODULE VOTES
   Fichier : ge_votes.c
   ================================================================ */
#include "election.h"

/* Insere un vote a la fin de la liste. */
MaillonVote *inserer_vote(MaillonVote*debut,Vote v){MaillonVote*nouveau=(MaillonVote*)malloc(sizeof(MaillonVote));MaillonVote*p;if(nouveau==NULL)return NULL;nouveau->information=v;nouveau->suivant=NULL;if(debut==NULL)return nouveau;p=debut;while(p->suivant!=NULL)p=p->suivant;p->suivant=nouveau;return debut;}
/* Recherche un vote par identifiant. */
MaillonVote *rechercher_vote_id(MaillonVote*debut,const char*id){while(debut!=NULL){if(strcmp(debut->information.idVote,id)==0)return debut;debut=debut->suivant;}return NULL;}
/* Recherche le vote correspondant a un electeur. */
MaillonVote *rechercher_vote_electeur(MaillonVote*debut,const char*ninu){while(debut!=NULL){if(strcmp(debut->information.ninu,ninu)==0)return debut;debut=debut->suivant;}return NULL;}
/* Libere toute la liste des votes. */
void liberer_votes(MaillonVote**debut){MaillonVote*p=*debut,*tmp;while(p!=NULL){tmp=p;p=p->suivant;free(tmp);}*debut=NULL;}
/* Insere un vote a la fin de la liste. */
int charger_votes(MaillonVote**debut){FILE*f=fopen(FICHIER_VOTE,"rb");Vote v;MaillonVote*liste=NULL,*tmp;int n=0;if(f==NULL)return 0;while(fread(&v,sizeof(Vote),1,f)==1){tmp=inserer_vote(liste,v);if(tmp==NULL){liberer_votes(&liste);fclose(f);return -1;}liste=tmp;n++;}fclose(f);liberer_votes(debut);*debut=liste;return n;}
/* Sauvegarde les votes dans votes.dat. */
int sauvegarder_votes(MaillonVote*debut){FILE*f=fopen(FICHIER_VOTE,"wb");if(f==NULL)return 0;while(debut!=NULL){if(fwrite(&debut->information,sizeof(Vote),1,f)!=1){fclose(f);return 0;}debut=debut->suivant;}fclose(f);return 1;}
/* Affiche les informations d'un seul vote. */
void afficher_vote(const Vote *v) {
    printf("ID vote   : %s\n", v->idVote);
    printf("Date vote : "); afficher_date(v->dateVote); printf("\n");
    printf("ID BV     : %s\n", v->idBV);
    printf("NINU      : %s\n", v->ninu);
    printf("Candidat  : %s\n", v->idCandidat);
}

/* Affiche tous les votes sous forme de tableau. */
void afficher_votes(MaillonVote *debut) {
    if (debut == NULL) {
        printf("Aucun vote enregistre.\n");
        return;
    }

    printf("\n================ LISTE DES VOTES ================\n\n");
    printf("%-10s %-12s %-10s %-12s %-12s\n",
           "ID_VOTE", "DATE", "ID_BV", "NINU", "CANDIDAT");

    while (debut != NULL) {
        char dateVote[16];
        snprintf(dateVote, sizeof(dateVote), "%02d/%02d/%04d",
                 debut->information.dateVote.jour,
                 debut->information.dateVote.mois,
                 debut->information.dateVote.annee);

        printf("%-10s %-12s %-10s %-12s %-12s\n",
               debut->information.idVote,
               dateVote,
               debut->information.idBV,
               debut->information.ninu,
               debut->information.idCandidat);
        debut = debut->suivant;
    }
}
static void generer_id_vote_unique(MaillonVote*liste,char*id){
    static unsigned long sequence=1;unsigned long n;
/* Recherche un vote par identifiant. */
    do{n=(unsigned long)time(NULL)%1000000UL; n=(n+sequence++)%1000000UL; snprintf(id,TAILLE_ID,"VT%06lu",n);}while(rechercher_vote_id(liste,id)!=NULL);
}

/* Enregistre un vote unique, normal ou blanc, avec date systeme. */
void enregistrer_vote(MaillonVote**listeVotes,MaillonElecteur*listeElecteurs,MaillonBV*listeBV,MaillonCandidat*listeCandidats){
    Vote v;char ninu[TAILLE_NINU],saisie[120];MaillonElecteur*e;MaillonCandidat*c=NULL;MaillonVote*tmp;int choix;
    printf("\n================ ENREGISTREMENT D'UN VOTE ================\n");
    if(listeElecteurs==NULL||listeCandidats==NULL){printf("ERREUR : il faut au moins un electeur et un candidat.\n");return;}
    do{saisir_chaine_obligatoire("NINU de l'electeur : ",ninu,sizeof(ninu));if(!valider_ninu_format(ninu))printf("ERREUR : NINU invalide.\n");else break;}while(1);
    e=rechercher_electeur(listeElecteurs,ninu);if(e==NULL){printf("ERREUR : electeur introuvable.\n");return;}
/* Recherche le vote correspondant a un electeur. */
    if(rechercher_vote_electeur(*listeVotes,ninu)!=NULL){printf("ERREUR : cet electeur a deja vote. Un electeur ne peut voter qu'une seule fois.\n");return;}
    if(rechercher_bv(listeBV,e->information.idBV)==NULL){printf("ERREUR : le bureau de vote de l'electeur est invalide.\n");return;}
    strcpy(v.ninu,ninu);strcpy(v.idBV,e->information.idBV);v.dateVote=date_systeme();generer_id_vote_unique(*listeVotes,v.idVote);

    printf("Electeur : %s %s | BV : %s\n",e->information.prenom,e->information.nom,e->information.idBV);
    do{
        printf("\nChoix du bulletin :\n1. Voter un candidat par NINU\n2. Voter un candidat par nom\n3. Vote blanc\nVotre choix : ");
        if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();
        if(choix==1){
            do{saisir_chaine_obligatoire("NINU du candidat : ",saisie,sizeof(saisie));if(!valider_ninu_format(saisie))printf("ERREUR : NINU invalide.\n");else{c=rechercher_candidat_ninu(listeCandidats,saisie);if(c==NULL)printf("ERREUR : candidat introuvable.\n");}}while(c==NULL||!valider_ninu_format(saisie));
            strcpy(v.idCandidat,c->information.idCandidat);break;
        }
        if(choix==2){
            saisir_chaine_obligatoire("Nom du candidat : ",saisie,sizeof(saisie));
            printf("\nCandidats correspondants :\n");afficher_candidats_par_nom(listeCandidats,saisie);
            printf("Saisir l'ID du candidat choisi : ");saisir_chaine_obligatoire("ID candidat : ",saisie,sizeof(saisie));
            for(size_t i=0;i<strlen(saisie);++i)saisie[i]=(char)toupper((unsigned char)saisie[i]);
            c=rechercher_candidat_id(listeCandidats,saisie);if(c==NULL)printf("ERREUR : candidat introuvable.\n");else{strcpy(v.idCandidat,c->information.idCandidat);break;}
        }
        if(choix==3){strcpy(v.idCandidat,VOTE_BLANC);break;}
        if(choix<1||choix>3)printf("Choix invalide.\n");
    }while(1);

    if(c!=NULL) printf("Candidat choisi : %s %s\n",c->information.prenom,c->information.nom);
    else printf("Vote blanc enregistre.\n");
    if(!confirmer("Confirmer le vote")){printf("Vote annule.\n");return;}
/* Insere un vote a la fin de la liste. */
    tmp=inserer_vote(*listeVotes,v);if(tmp==NULL){printf("ERREUR : memoire insuffisante.\n");return;}*listeVotes=tmp;
/* Sauvegarde les votes dans votes.dat. */
    if(sauvegarder_votes(*listeVotes))printf("Vote enregistre avec succes le ");else{printf("AVERTISSEMENT : vote en memoire mais sauvegarde impossible. Date : ");}afficher_date(v.dateVote);printf(".\n");
}
