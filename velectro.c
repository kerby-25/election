/* ================================================================
   PROGRAMME PRINCIPAL ET OUTILS COMMUNS
   Fichier : velectro.c
   ================================================================ */
#include "election.h"

/* ---------------------- Outils de saisie ----------------------- */
void vider_buffer(void){int c;while((c=getchar())!='\n'&&c!=EOF){} }

/* Supprime les espaces inutiles et les fins de ligne d'une chaine. */
void nettoyer_chaine(char*s){size_t n;if(s==NULL)return;n=strlen(s);while(n>0&&(s[n-1]=='\n'||s[n-1]=='\r'||isspace((unsigned char)s[n-1]))){s[--n]='\0';}while(*s&&isspace((unsigned char)*s)){memmove(s,s+1,strlen(s));}}

/* Saisit une chaine et refuse tout champ vide. */
void saisir_chaine_obligatoire(const char*invite,char*destination,size_t taille){
    int c;
    size_t longueur;
    do {
        printf("%s", invite);
        if (fgets(destination, (int)taille, stdin) == NULL) {
            destination[0] = '\0';
            clearerr(stdin);
            continue;
        }
        longueur = strlen(destination);
        /* Cas important : si le champ remplit exactement le buffer,
           le '\n' peut etre reste dans stdin. On le consomme ici.
           Un caractere supplementaire signifie alors une saisie trop longue. */
        if (longueur > 0 && destination[longueur - 1] != '\n' && destination[longueur - 1] != '\r') {
            c = getchar();
            if (c != '\n' && c != EOF) {
                while ((c = getchar()) != '\n' && c != EOF) { }
                printf("ERREUR : saisie trop longue. Veuillez recommencer ce champ.\n");
                destination[0] = '\0';
                continue;
            }
        }
        nettoyer_chaine(destination);
        if (destination[0] == '\0')
            printf("ERREUR : ce champ est obligatoire. Veuillez le remplir avant de continuer.\n");
    } while (destination[0] == '\0');
}

/* Nettoie l'ecran de la console selon le systeme utilise. */
void vider_ecran(void){
#ifdef _WIN32
    system("cls");
#else
    system("clear >/dev/null 2>&1");
#endif
}

/* Met le programme en pause jusqu'a l'appui sur Entree. */
void pause_console(void){printf("\nAppuyez sur Entree pour continuer...");getchar();}

/* Teste si une chaine contient uniquement des chiffres. */
int chaine_est_numerique(const char*s){if(s==NULL||*s=='\0')return 0;for(size_t i=0;i<strlen(s);i++)if(!isdigit((unsigned char)s[i]))return 0;return 1;}
/* Teste si une chaine contient uniquement des lettres, espaces ou tirets. */
int chaine_est_lettres(const char*s){if(s==NULL||*s=='\0')return 0;for(size_t i=0;i<strlen(s);i++)if(!isalpha((unsigned char)s[i])&&s[i]!=' '&&s[i]!='-')return 0;return 1;}
/* Verifie le format exact de 8 caracteres : 2 lettres puis 6 chiffres. */
int valider_id_format(const char*id){if(id==NULL||strlen(id)!=8)return 0;if(!isalpha((unsigned char)id[0])||!isalpha((unsigned char)id[1]))return 0;for(int i=2;i<8;i++)if(!isdigit((unsigned char)id[i]))return 0;return 1;}
/* Teste si une chaine contient uniquement des chiffres. */
int valider_ninu_format(const char*ninu){return ninu!=NULL&&strlen(ninu)==10&&chaine_est_numerique(ninu);}

/* Compare deux chaines sans tenir compte de la casse. */
int egale_sans_casse(const char *a, const char *b){
    size_t i;
    if(a==NULL||b==NULL)return 0;
    if(strlen(a)!=strlen(b))return 0;
    for(i=0;i<strlen(a);++i){
        if(tolower((unsigned char)a[i])!=tolower((unsigned char)b[i]))return 0;
    }
    return 1;
}

/* Valide une adresse Gmail conforme au format nom@gmail.com. */
int valider_email_gmail(const char*email){
    const char *at;
    size_t localLen;
    if(email==NULL || strchr(email,' ')!=NULL)return 0;
    at=strchr(email,'@');
    if(at==NULL || strchr(at+1,'@')!=NULL)return 0;
/* Compare deux chaines sans tenir compte de la casse. */
    if(!egale_sans_casse(at+1,"gmail.com"))return 0;
    localLen=(size_t)(at-email);
    if(localLen<1 || localLen>64)return 0;
    if(email[0]=='.' || at[-1]=='.')return 0;
    for(size_t i=0;i<localLen;i++){
        unsigned char c=(unsigned char)email[i];
        if(!(isalnum(c) || c=='.'))return 0;
        if(i>0 && email[i]=='.' && email[i-1]=='.')return 0;
    }
    return 1;
}

/* Teste si une chaine contient uniquement des chiffres. */
int valider_telephone(const char*telephone){return telephone!=NULL&&strlen(telephone)==8&&chaine_est_numerique(telephone);}
/* Teste si une chaine contient uniquement des chiffres. */
int valider_numero_rue_1_3_chiffres(const char*numero){return numero!=NULL&&strlen(numero)>=1&&strlen(numero)<=3&&chaine_est_numerique(numero);}
/* Teste si une chaine contient uniquement des lettres, espaces ou tirets. */
int valider_nom_rue(const char*rue){return rue!=NULL&&strlen(rue)>0&&chaine_est_lettres(rue);}

/* Verifie qu'une date existe dans le calendrier. */
int date_valide(Date d){
    int joursParMois[]={0,31,28,31,30,31,30,31,31,30,31,30,31};
    if(d.annee<1900||d.mois<1||d.mois>12)return 0;
    if((d.annee%400==0)||(d.annee%4==0&&d.annee%100!=0))joursParMois[2]=29;
    return d.jour>=1&&d.jour<=joursParMois[d.mois];
}

/* Compare deux dates et retourne leur ordre chronologique. */
int comparer_dates(Date a,Date b){if(a.annee!=b.annee)return a.annee<b.annee?-1:1;if(a.mois!=b.mois)return a.mois<b.mois?-1:1;if(a.jour!=b.jour)return a.jour<b.jour?-1:1;return 0;}

/* Retourne la date courante du systeme. */
Date date_systeme(void){time_t maintenant=time(NULL);struct tm*tmInfo=localtime(&maintenant);Date d={tmInfo->tm_mday,tmInfo->tm_mon+1,tmInfo->tm_year+1900};return d;}
/* Compare deux dates et retourne leur ordre chronologique. */
int date_egale_systeme(Date d){return comparer_dates(d,date_systeme())==0;}

/* Retourne la date courante du systeme. */
int age_aujourd_hui(Date naissance){Date aj=date_systeme();int age=aj.annee-naissance.annee;if(aj.mois<naissance.mois||(aj.mois==naissance.mois&&aj.jour<naissance.jour))age--;return age;}

/* Verifie qu'une date existe dans le calendrier. */
static int lire_date_brute(const char*s,Date*d){int j,m,a;char extra;if(sscanf(s,"%d/%d/%d %c",&j,&m,&a,&extra)!=3)return 0;d->jour=j;d->mois=m;d->annee=a;return date_valide(*d);}

/* Saisit une date valide et peut interdire les dates futures. */
int date_saisir(const char*invite,Date*d,int dateNeDoitPasEtreFuture){
/* Retourne la date courante du systeme. */
    char s[80];Date auj=date_systeme();
/* Saisit une chaine et refuse tout champ vide. */
    do{saisir_chaine_obligatoire(invite,s,sizeof(s));if(!lire_date_brute(s,d)){printf("ERREUR : date invalide. Utilisez JJ/MM/AAAA et une date reelle.\n");continue;}if(dateNeDoitPasEtreFuture&&comparer_dates(*d,auj)>0){printf("ERREUR : la date saisie est dans le futur. Elle doit etre anterieure ou egale au %02d/%02d/%04d.\n",auj.jour,auj.mois,auj.annee);continue;}return 1;}while(1);
}

/* Saisit une date et impose un age minimal. */
int date_saisir_age_minimum(const char*invite,Date*d,int ageMinimum){
/* Calcule l'age courant a partir de la date de naissance. */
    do{date_saisir(invite,d,1);if(age_aujourd_hui(*d)<ageMinimum)printf("ERREUR : la personne doit avoir au moins %d ans. Veuillez corriger la date maintenant.\n",ageMinimum);else return 1;}while(1);
}

/* Saisit et valide les trois sous-champs d'une adresse. */
void saisir_adresse(Adresse*adresse){char s[120];
/* Saisit une chaine et refuse tout champ vide. */
    do{saisir_chaine_obligatoire("No Rue (1 a 3 chiffres) : ",s,sizeof(s));if(!valider_numero_rue_1_3_chiffres(s))printf("ERREUR : le numero de rue doit contenir de 1 a 3 chiffres maximum.\n");else break;}while(1);adresse->noRue=atoi(s);
/* Saisit une chaine et refuse tout champ vide. */
    do{saisir_chaine_obligatoire("Rue (lettres, espaces et tirets) : ",adresse->rue,sizeof(adresse->rue));if(!valider_nom_rue(adresse->rue))printf("ERREUR : le nom de rue doit etre compose de lettres, espaces ou tirets.\n");else break;}while(1);
/* Saisit une chaine et refuse tout champ vide. */
    saisir_chaine_obligatoire("Departement : ",adresse->departement,sizeof(adresse->departement));
}

/* Supprime les espaces inutiles et les fins de ligne d'une chaine. */
int confirmer(const char*question){char rep[10];do{printf("%s (O/N) : ",question);if(fgets(rep,sizeof(rep),stdin)==NULL)return 0;nettoyer_chaine(rep);if(rep[0]=='O'||rep[0]=='o')return 1;if(rep[0]=='N'||rep[0]=='n')return 0;printf("Veuillez repondre par O ou N.\n");}while(1);}

/* Affiche une date au format JJ/MM/AAAA. */
void afficher_date(Date d){printf("%02d/%02d/%04d",d.jour,d.mois,d.annee);}
/* Affiche une adresse complete. */
void afficher_adresse(const Adresse*a){printf("%d, %s, %s",a->noRue,a->rue,a->departement);}

/* -------------------------- Sous-menus ------------------------- */
static void menu_partis(MaillonParti**partis,MaillonCandidat*candidats){int choix;do{vider_ecran();printf("================ GESTION DES PARTIS ================\n1. Inscrire un parti\n2. Afficher les partis\n3. Modifier un parti\n4. Supprimer un parti\n0. Retour\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:ins_PP(partis);pause_console();break;case 2:afficher_partis(*partis);pause_console();break;case 3:modif_PP(*partis);sauvegarder_partis(*partis);pause_console();break;case 4:suppr_PP(partis,candidats);pause_console();break;case 0:break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Vide les caracteres restants dans le flux standard d'entree. */
static void menu_bv(MaillonBV**bvs,MaillonElecteur**electeurs){
    int choix;
    do {
        vider_ecran();
        printf("================ GESTION DES BUREAUX DE VOTE ================\n");
        printf("RAPPEL : a la creation d'un BV, ses trois responsables sont inscrits immediatement comme electeurs.\n\n");
        printf("1. Creer un BV et inscrire ses responsables\n2. Afficher les BV\n3. Reaffecter les membres\n4. Modifier un BV\n5. Supprimer un BV\n0. Retour\nVotre choix : ");
        if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();
        switch(choix){
            case 1:ajout_BV(bvs,electeurs);pause_console();break;
            case 2:afficher_bvs(*bvs);pause_console();break;
            case 3:affecter_membres_BV(*bvs,*electeurs);pause_console();break;
            case 4:modif_BV(*bvs,*electeurs);pause_console();break;
            case 5:suppr_BV(bvs,*electeurs);pause_console();break;
            case 0:break;
            default:printf("Choix invalide.\n");pause_console();
        }
    } while(choix!=0);
}

/* Vide les caracteres restants dans le flux standard d'entree. */
static void menu_electeurs(MaillonElecteur**electeurs,MaillonBV*bvs,MaillonVote*votes){int choix;do{vider_ecran();printf("================ GESTION DES ELECTEURS ================\n");printf("RAPPEL : creez d abord les BV. Les responsables d un BV sont inscrits automatiquement lors de sa creation.\n\n");printf("1. Enregistrer un electeur\n2. Afficher les electeurs\n3. Rechercher un electeur\n4. Modifier un electeur\n5. Supprimer un electeur\n0. Retour\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:ins_elect(electeurs,bvs);pause_console();break;case 2:afficher_electeurs(*electeurs);pause_console();break;case 3:{char ninu[TAILLE_NINU];saisir_chaine_obligatoire("NINU : ",ninu,sizeof(ninu));MaillonElecteur*n=rechercher_electeur(*electeurs,ninu);if(n)afficher_electeur(&n->information);else printf("Electeur introuvable.\n");pause_console();break;}case 4:modif_elect(*electeurs,bvs);sauvegarder_electeurs(*electeurs);pause_console();break;case 5:suppr_elect(electeurs,bvs,votes);pause_console();break;case 0:break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Vide les caracteres restants dans le flux standard d'entree. */
static void menu_candidats(MaillonCandidat**candidats,MaillonParti*partis,MaillonVote*votes){int choix;do{vider_ecran();printf("================ GESTION DES CANDIDATS ================\n1. Inscrire un candidat\n2. Afficher les candidats\n3. Rechercher un candidat\n4. Modifier un candidat\n5. Supprimer un candidat\n0. Retour\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:ins_cand(candidats,partis);pause_console();break;case 2:afficher_candidats(*candidats);pause_console();break;case 3:{int t;char s[120];printf("1. Par ID\n2. Par NINU\n3. Par nom\nChoix : ");if(scanf("%d",&t)!=1){vider_buffer();t=-1;}else vider_buffer();if(t==1){saisir_chaine_obligatoire("ID : ",s,sizeof(s));for(size_t i=0;i<strlen(s);++i)s[i]=(char)toupper((unsigned char)s[i]);MaillonCandidat*n=rechercher_candidat_id(*candidats,s);if(n)afficher_candidat(&n->information);else printf("Candidat introuvable.\n");}else if(t==2){saisir_chaine_obligatoire("NINU : ",s,sizeof(s));MaillonCandidat*n=rechercher_candidat_ninu(*candidats,s);if(n)afficher_candidat(&n->information);else printf("Candidat introuvable.\n");}else if(t==3){saisir_chaine_obligatoire("Nom : ",s,sizeof(s));afficher_candidats_par_nom(*candidats,s);}else printf("Choix invalide.\n");pause_console();break;}case 4:modif_cand(*candidats,partis);sauvegarder_candidats(*candidats);pause_console();break;case 5:suppr_cand(candidats,votes);pause_console();break;case 0:break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Vide les caracteres restants dans le flux standard d'entree. */
static void menu_votes(MaillonVote**votes,MaillonElecteur*electeurs,MaillonBV*bvs,MaillonCandidat*candidats){int choix;do{vider_ecran();printf("================ GESTION DES VOTES ================\n1. Enregistrer un vote\n2. Afficher les votes\n0. Retour\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:enregistrer_vote(votes,electeurs,bvs,candidats);pause_console();break;case 2:afficher_votes(*votes);pause_console();break;case 0:break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Vide les caracteres restants dans le flux standard d'entree. */
static void menu_rapports(MaillonElecteur*electeurs,MaillonBV*bvs,MaillonCandidat*candidats,MaillonVote*votes){int choix;do{vider_ecran();printf("================ RAPPORTS ET RESULTATS ================\n1. Liste des electeurs\n2. Liste des candidats\n3. Liste des BV\n4. Resultats\n5. BV par departement\n6. Electeurs par departement et par BV\n0. Retour\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:List_Electeurs(electeurs,bvs);pause_console();break;case 2:List_Candid(candidats);pause_console();break;case 3:List_BV(bvs,electeurs);pause_console();break;case 4:Resultats(bvs,candidats,votes);pause_console();break;case 5:List_BV_Par_Departement(bvs,electeurs);pause_console();break;case 6:List_Electeurs_Par_Departement_BV(electeurs,bvs);pause_console();break;case 0:break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Vide les caracteres restants dans le flux standard d'entree. */
void menu_principal(MaillonParti**partis,MaillonBV**bvs,MaillonElecteur**electeurs,MaillonCandidat**candidats,MaillonVote**votes){int choix;do{vider_ecran();printf("==============================================================\n");printf("        SYSTEME DE GESTION ELECTORALE - PRESIDENTIELLE\n");printf("==============================================================\n");printf("1. Gestion des partis politiques\n2. Gestion des bureaux de vote\n3. Gestion des electeurs\n4. Gestion des candidats\n5. Gestion des votes\n6. Rapports et resultats\n0. Quitter\n\nVotre choix : ");if(scanf("%d",&choix)!=1){vider_buffer();choix=-1;}else vider_buffer();switch(choix){case 1:menu_partis(partis,*candidats);break;case 2:menu_bv(bvs,electeurs);break;case 3:menu_electeurs(electeurs,*bvs,*votes);break;case 4:menu_candidats(candidats,*partis,*votes);break;case 5:menu_votes(votes,*electeurs,*bvs,*candidats);break;case 6:menu_rapports(*electeurs,*bvs,*candidats,*votes);break;case 0:printf("Fermeture du systeme...\n");break;default:printf("Choix invalide.\n");pause_console();}}while(choix!=0);}

/* Point d'entree du programme : chargement, menu et liberation des listes. */
int main(void){
    MaillonParti*partis=NULL;MaillonBV*bvs=NULL;MaillonElecteur*electeurs=NULL;MaillonCandidat*candidats=NULL;MaillonVote*votes=NULL;
    int np=charger_partis(&partis),nb=charger_bv(&bvs),ne=charger_electeurs(&electeurs),nc=charger_candidats(&candidats),nv=charger_votes(&votes);
/* Nettoie l'ecran de la console selon le systeme utilise. */
    vider_ecran();
    printf("Chargement des donnees : Partis=%d | BV=%d | Electeurs=%d | Candidats=%d | Votes=%d\n",np,nb,ne,nc,nv);
/* Met le programme en pause jusqu'a l'appui sur Entree. */
    pause_console();
    menu_principal(&partis,&bvs,&electeurs,&candidats,&votes);
    liberer_partis(&partis);liberer_bv(&bvs);liberer_electeurs(&electeurs);liberer_candidats(&candidats);liberer_votes(&votes);
    return 0;
}
