/*********************************************
**	        	 Rakshata v1.1 		        **
**     Licence propriétaire, code source    **
**        confidentiel, distribution        **
**          formellement interdite          **
**********************************************/

#include "main.h"

int status;
static int error;

int telechargement()
{
    int i = 0, j = 0, k = 0, l = 0, chapitre = 0, mangaActuel = 0, mangaTotal = 0, pourcentage = 0, glados = 1;
    char buffer[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX], teamCourt[LONGUEUR_COURT], temp[200], mangaLong[LONGUEUR_NOM_MANGA_MAX], historiqueTeam[1000][LONGUEUR_COURT];
    char URL[LONGUEUR_URL], type[5], superTemp[400], trad[SIZE_TRAD_ID_22][100], site[LONGUEUR_SITE], MOT_DE_PASSE_COMPTE[100];
    FILE* fichier = NULL;
    FILE* ressources = NULL;
    FILE* test = NULL;
    SDL_Texture *texte = NULL;
    TTF_Font *police_big = NULL;
    TTF_Font *police = NULL;
    SDL_Rect position;
	SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};

    while(NETWORK_ACCESS == CONNEXION_TEST_IN_PROGRESS);

    if(NETWORK_ACCESS == CONNEXION_DOWN)
        quit_thread(0);

    police_big = TTF_OpenFont(FONTUSED, POLICE_GROS);
    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);

    fichier = fopenR("tmp/import.dat", "r");

    crashTemp(MOT_DE_PASSE_COMPTE, 100);

    /*On compte le nombre de mangas*/
    mangaTotal = 1;
    fgetc(fichier);//Si la première ligne est vide, on la compte pas
    while((i = fgetc(fichier)) != EOF)
    {
        if(i == '\n')
            mangaTotal++;
    }
    rewind(fichier);

    for(i = 0; i < SIZE_TRAD_ID_22; i++)
    {
        for(j = 0; j < LONGUEUR_COURT; j++)
            trad[i][j] = 0;
    }
    for(i = 0; i < 1000; i++)
    {
        for(j = 100; j < 100; j++)
            historiqueTeam[i][j] = 0;
    }
    loadTrad(trad, 22);

    for(mangaActuel = 1; fgetc(fichier) != EOF && glados; mangaActuel++) //On démarre Ã  1 car sinon, le premier pourcentage serait de 0
    {
        if(mangaTotal + (mangaActuel - 1) != 0)
            pourcentage = mangaActuel * 100 / (mangaTotal + mangaActuel -1);
        else
            pourcentage = 100;

        /*Extraction du chapitre*/
        fseek(fichier, -1, SEEK_CUR);
        crashTemp(buffer, LONGUEUR_NOM_MANGA_MAX);
        crashTemp(teamCourt, LONGUEUR_COURT);
        fscanfs(fichier, "%s %s %d", teamCourt, LONGUEUR_COURT, buffer, LONGUEUR_NOM_MANGA_MAX, &chapitre);
        fclose(fichier);

        /*Récuperer les données relative a la team*/
        ressources = fopenR("data/repo", "r");
        crashTemp(temp, 200);
        do
        {
            if(positionnementApres(ressources, teamCourt))
            {
                crashTemp(URL, LONGUEUR_URL);
                crashTemp(type, 5);
                crashTemp(site, LONGUEUR_SITE);
                fscanfs(ressources, "%s %s %s", type, 5, URL, LONGUEUR_URL, site, LONGUEUR_SITE); //Récupération des infos nécéssaire au DL
                fclose(ressources);

                /*Etablir relation entre le nom du manga et son nomComplet*/
                ressources = fopenR(MANGA_DATABASE, "r");
                crashTemp(temp, 200);
                /*On se position dans l'espace team*/
                if(positionnementApres(ressources, teamCourt))
                {
                    while((i = fgetc(ressources)) != '#' && i != EOF && strcmp(temp, buffer))
                    {
                        fseek(ressources, -1, SEEK_CUR);
                        crashTemp(temp, 200);
                        crashTemp(mangaLong, LONGUEUR_NOM_MANGA_MAX);
                        fscanfs(ressources, "%s %s %d %d %d %d", mangaLong, LONGUEUR_NOM_MANGA_MAX, temp, 200, &l, &i, &j, &k);
                    }
                    fclose(ressources);

                    if(!strcmp(temp, buffer))
                    {
                        /*Ouverture du site de la team*/
                        for(i = 0; i < 1000 && strcmp(teamCourt, historiqueTeam[i]) != 0 && historiqueTeam[i][0] != 0; i++);
                        if(i < 1000 && historiqueTeam[i][0] == 0) //Si pas déjÃ  installé
                        {
                            ustrcpy(historiqueTeam[i], teamCourt);
                            ouvrirSite(teamCourt);
                        }

                        /*Si on a bien tout extrait*/
                        teamOfProject(mangaLong, teamLong);
                        crashTemp(superTemp, 400);
                        /*Génération de l'URL*/
                        if (!strcmp(type, TYPE_DEPOT_1))
                            sprintf(superTemp, "http://dl-web.dropbox.com/u/%s/%s/%s_Chapitre_%d.zip", URL, mangaLong, buffer, chapitre);

                        else if (!strcmp(type, TYPE_DEPOT_2))
                            sprintf(superTemp, "http://%s/%s/%s_Chapitre_%d.zip", URL, mangaLong, buffer, chapitre);

                        else if (!strcmp(type, TYPE_DEPOT_3)) //DL Payant
                        {
                            if(MOT_DE_PASSE_COMPTE[0] == -1)
                                break;

                            else if(MOT_DE_PASSE_COMPTE[0] || (i = getPassword(MOT_DE_PASSE_COMPTE)) == 1)
                                sprintf(superTemp, "http://rsp.%s/main_controler.php?target=%s&project=%s&chapter=%d&mail=%s&pass=%s", MAIN_SERVER_URL[0], URL, mangaLong, chapitre, COMPTE_PRINCIPAL_MAIL, MOT_DE_PASSE_COMPTE);

                            else if(i == PALIER_QUIT)
                            {
                                glados = 0;
                                break;
                            }

                            else
                                MOT_DE_PASSE_COMPTE[0] = -1;

                            chargement();
                        }

                        else
                        {
                            sprintf(superTemp, "URL non gérée: %s\n", type);
                            logR(superTemp);
                            break;
                        }

                        /*Affichage du DL*/
                        crashTemp(temp, 200);
                        changeTo(mangaLong, '_', ' ');

                        sprintf(temp, "%s %s %s %d %s %s (%d%% %s)", trad[0], mangaLong, trad[1], chapitre, trad[2], teamCourt, pourcentage, trad[3]);

                        changeTo(mangaLong, ' ', '_');

                        //On remplis la fenêtre

                        applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);;
                        SDL_RenderPresent(renderer);
                        texte = TTF_Write(renderer, police_big, trad[4], couleurTexte);
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.y = HAUTEUR_MESSAGE_INITIALISATION;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_RenderPresent(renderer);
                        SDL_DestroyTextureS(texte);

                        texte = TTF_Write(renderer, police, temp, couleurTexte);
                        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
                        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
                        position.h = texte->h;
                        position.w = texte->w;
                        SDL_RenderCopy(renderer, texte, NULL, &position);
                        SDL_DestroyTextureS(texte);

                        SDL_RenderPresent(renderer);

                        /**Téléchargement**/

                        /*On teste si le chapitre est déjÃ  installé*/
                        crashTemp(temp, 200);
                        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaLong, chapitre, CONFIGFILE);
                        test = fopenR(temp, "r");

                        crashTemp(temp, 200);
                        sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, buffer, chapitre);
                        ressources = fopenR(temp, "r");

                        if(ressources == NULL && test == NULL)
                        {
                            if(UNZIP_NEW_PATH) //Il faut modifier le paths
                            {
                                crashTemp(temp, 200);
                                sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, buffer, chapitre);
                            }

                            glados = download(superTemp, temp, 1);

                            ressources = fopenR(temp, "r");

                            if(glados < -1 && glados != 42)
                                glados = -1;

                            if(ressources != NULL && checkFileValide(ressources))
                                fclose(ressources);
                            else
                            {
                                if(ressources != NULL)
                                {
                                    fclose(ressources);
                                    crashTemp(superTemp, 400);
                                    sprintf(superTemp, "Erreur dans le telechargement d'un chapitre: Team: %s\n", teamCourt);
                                    logR(superTemp);
                                    remove(temp);
                                }
                                glados = -1;//On annule l'installation
                            }
                        }

                        else
                        {
                            if(ressources != NULL)
                                fclose(ressources);
                            if(test != NULL)
                                fclose(test);
                        }

                        if(glados == -1) //Si fermeture
                        {
                            glados = 0;
                            break;
                        }

                        else if(glados > 0) // Archive pas corrompue
                        {
                            crashTemp(temp, 200);
                            sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, buffer, chapitre); //chaine déjÃ  traité
                            ressources = fopenR(temp, "r");
                            if(ressources != NULL)
                            {
                                DATA_INSTALL* data_instal = malloc(sizeof(DATA_INSTALL));
                                status += 1; //On signale le lancement d'une installation
								fclose(ressources);
                                /**Installation**/
                                ustrcpy(data_instal->teamCourt, teamCourt);
                                ustrcpy(data_instal->mangaCourt, buffer);
                                data_instal->chapitre = chapitre;
                                #ifdef _WIN32
                                CreateThread(NULL, 0, installation, data_instal, 0, NULL); //Initialisation du thread
                                #else
                                if (pthread_create(&thread, NULL, installation, data_instal))
                                {
                                    logR("Failled at create thread MDL\n");
                                    exit(EXIT_FAILURE);
                                }
                                #endif
                            }
                        }

                        sprintf(temp, "manga/%s/%s/infos.png", teamLong, mangaLong);
                        test = fopenR(temp, "r");

                        if(test == NULL && k) //k peut avoir a être > 1
                        {
                            crashTemp(temp, TAILLE_BUFFER);
                            sprintf(temp, "manga/%s/%s/%s", teamLong, mangaLong, CONFIGFILE);
                            test = fopenR(temp, "r");
                            if(test == NULL)
                            {
                                crashTemp(temp, TAILLE_BUFFER);
                                sprintf(temp, "manga/%s", teamLong);
                                mkdirR(temp);
                                sprintf(temp, "manga/%s/%s", teamLong, mangaLong);
                                mkdirR(temp);
                            }
                            else
                                fclose(test);

                            crashTemp(superTemp, 400);
                            /*Génération de l'URL*/
                            if(!strcmp(type, TYPE_DEPOT_1))
                            {
                                sprintf(superTemp, "http://dl-web.dropbox.com/u/%s/%s/infos.png", URL, mangaLong);
                            }
                            else if (!strcmp(type, TYPE_DEPOT_2))
                            {
                                sprintf(superTemp, "http://%s/%s/infos.png", type, mangaLong);
                            }
                            else if(!strcmp(type, TYPE_DEPOT_3))
                            {
                                sprintf(superTemp, "http://%s/getinfopng.php?owner=%s&manga=%s", MAIN_SERVER_URL[0], teamLong, mangaLong);
                            }
                            else
                            {
                                sprintf(superTemp, "URL non gérée: %s\n", type);
                                logR(superTemp);
                                continue;
                            }

                            crashTemp(temp, TAILLE_BUFFER);
                            sprintf(temp, "manga/%s/%s/infos.png", teamLong, mangaLong);
                            download(superTemp, temp, 0);
                        }

                        else if(test != NULL && k)
                            fclose(test);

                        else //Si k = 0 et infos.png existe
                            removeR(temp);
                    }
                }
            }
        } while(0); //Permet d'interrompre le code avec break;

        if(glados > 0)
        {
            test = fopenR("tmp/import.dat.tmp", "a+");
            fichier = fopenR("tmp/import.dat", "r");

            fgetc(fichier); //On saute le premier caractère qui est parfoisun saut de ligne
            while((i = fgetc(fichier)) != '\n' && i != EOF); //On saute la ligne actuelle

            mangaTotal = 1; //Pour compter la dernière ligne, sans \n
            while((i = fgetc(fichier)) != EOF) //On copie la fin du fichier dans un buffer
            {
                fputc(i, test);
                if(i == '\n')
                    mangaTotal++;
            }
            fclose(test);
            fclose(fichier);

            removeR("tmp/import.dat");
            renameR("tmp/import.dat.tmp", "tmp/import.dat");

            fichier = fopenR("tmp/import.dat", "r");
        }
    }
    if(fichier != NULL)
        fclose(fichier);

    if(glados)
        removeR("tmp/import.dat");

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
    texte = TTF_Write(renderer, police, trad[5], couleurTexte);
    position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texte->h / 2;
    position.h = texte->h;
    position.w = texte->w;
    SDL_RenderCopy(renderer, texte, NULL, &position);
    SDL_DestroyTextureS(texte);

    SDL_RenderPresent(renderer);
    TTF_CloseFont(police_big);
    TTF_CloseFont(police);
    while(status > 1)
        SDL_Delay(50);

    chargement();
    status = 0;
    quit_thread(0);
}

#ifdef _WIN32
DWORD WINAPI installation(LPVOID datas)
#else
void* installation(void* datas)
#endif
{
    int nouveauDossier = 0, i = 0, j = 0, k = 0, l = 0, extremes[2], erreurs = 0, dernierLu = 0, chapitre = 0;
    char temp[TAILLE_BUFFER], buffer1[200], mangaLong[LONGUEUR_NOM_MANGA_MAX], teamLong[LONGUEUR_NOM_MANGA_MAX];
    char teamCourt[LONGUEUR_COURT], mangaCourt[LONGUEUR_COURT];
    FILE* ressources = NULL;
    FILE* test = NULL;

	DATA_INSTALL *valeurs = (DATA_INSTALL*)datas;

    nameWindow(status);

    /*Récupération des valeurs envoyés*/
    ustrcpy(teamCourt, valeurs->teamCourt);
    ustrcpy(mangaCourt, valeurs->mangaCourt);
    chapitre = valeurs->chapitre;

    teamOfProject(mangaCourt, teamLong);

    /*Lecture du fichier*/
    nouveauDossier = 0;
    crashTemp(temp, TAILLE_BUFFER);
    sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, mangaCourt, chapitre);
    test = fopenR(temp, "r");

    if(test == NULL) //return;
    {
        free(valeurs);
        status--; //On signale la fin de l'installation
        nameWindow(status);
        quit_thread(0);
    }

    fclose(test);
    /*On récupère les données de manga relative a ce manga*/
    /*Etablir relation entre le nom du manga et son nomComplet*/
    ressources = fopenR(MANGA_DATABASE, "r");
    crashTemp(temp, TAILLE_BUFFER);

    positionnementApres(ressources, teamCourt); //On se position dans l'espace team

    /*On lit les mangas*/
    while(fgetc(ressources) != '#' && strcmp(temp, mangaCourt))
    {
        fseek(ressources, -1, SEEK_CUR);
        crashTemp(temp, TAILLE_BUFFER);
        crashTemp(mangaLong, LONGUEUR_NOM_MANGA_MAX);
        i = j = k = 0;
        fscanfs(ressources, "%s %s %d %d %d %d", mangaLong, LONGUEUR_NOM_MANGA_MAX, temp, TAILLE_BUFFER, &l, &i, &j, &k);
    }
    fclose(ressources);

    if(strcmp(temp, mangaCourt) == 0)
    {
        /*On continue si tout vas bien*/
        crashTemp(temp, TAILLE_BUFFER);
        /*Vérification d'existance*/
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaLong, chapitre, CONFIGFILE);
        test = fopenR(temp, "r");
        if(test == NULL)
        {
            /*Si le chapitre n'est pas déjâ€¡ installé*/
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/%s", teamLong, mangaLong, CONFIGFILE);
            test = fopenR(temp, "r");
            if(test == NULL)
            {
                /*Si le dossier du manga n'existe pas*/
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s", teamLong, mangaLong);
                #ifdef _WIN32
                mkdir(temp);
                #else
                mkdir(temp, PERMISSIONS);
                #endif

                /*On signale le nouveau dossier*/
                nouveauDossier = 1;
            }
            else
                fclose(test);

            /**Décompression dans le repertoire de destination**/

            /*Création du répertoire de destination*/
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d", teamLong, mangaLong, chapitre);
            mkdirR(temp);

            crashTemp(temp, TAILLE_BUFFER);
            crashTemp(buffer1, 200);

            sprintf(buffer1, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, mangaCourt, chapitre);
            sprintf(temp, "manga/%s/%s/Chapitre_%d", teamLong, mangaLong, chapitre);

            applyWindowsPathCrap(temp);
            erreurs = miniunzip(buffer1, temp, "", chapitre);

            /*Si c'est pas un nouveau dossier, on modifie config.dat du manga*/
            if(!erreurs)
            {
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaLong, chapitre, CONFIGFILE);
                ressources = fopenR(temp, "r");
            }

            if(erreurs != -1 && nouveauDossier == 0 && ressources != NULL)
            {
                fclose(ressources);
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/%s", teamLong, mangaLong, CONFIGFILE);
                ressources = fopenR(temp, "r+");
                fscanfs(ressources, "%d %d", &extremes[0], &extremes[1]);
                if(fgetc(ressources) != EOF)
                    fscanfs(ressources, "%d", &dernierLu);
                else
                    dernierLu = -1;
                fclose(ressources);
                ressources = fopenR(temp, "w+");
                if(extremes[0] > chapitre)
                    fprintf(ressources, "%d %d", chapitre, extremes[1]);

                else if(extremes[1] < chapitre)
                    fprintf(ressources, "%d %d", extremes[0], chapitre);

                else
                    fprintf(ressources, "%d %d", extremes[0], extremes[1]);
                if(dernierLu != -1)
                    fprintf(ressources, " %d", dernierLu);

                fclose(ressources);
            }

            else if(erreurs != -1 && ressources != NULL)
            {
                fclose(ressources);
                /*Création du config.dat du nouveau manga*/
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/%s", teamLong, mangaLong, CONFIGFILE);
                ressources = fopenR(temp, "w+");
                fprintf(ressources, "%d %d", chapitre, chapitre);
                fclose(ressources);
            }

            else //Archive corrompue
            {
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "Archive Corrompue: %s - %d\n", mangaLong, chapitre);
                logR(temp);
                crashTemp(temp, TAILLE_BUFFER);
                sprintf(temp, "manga/%s/%s/Chapitre_%d", teamLong, mangaLong, chapitre);
                removeFolder(temp);
                erreurs = 1;
            }

            /*On supprime l'archive*/
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, mangaCourt, chapitre);
            removeR(temp);
        }
        else
        {
            fclose(test);
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "tmp/[%s]%s_Chapitre_%d.zip", teamCourt, mangaCourt, chapitre);
            removeR(temp);
        }
    }
    if(erreurs)
        error++; //On note si le chapitre a posé un problème

    free(valeurs);
    status--; //On signale la fin de l'installation
    nameWindow(status);
    quit_thread(0);
}

int interditWhileDL()
{
    /*Initialisateurs graphique*/
	char texte[SIZE_TRAD_ID_9][100];
	SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    loadTrad(texte, 9);

    texteAffiche = TTF_Write(renderer, police, texte[0], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 - texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    texteAffiche = TTF_Write(renderer, police, texte[1], couleur);

    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = WINDOW_SIZE_H / 2 + texteAffiche->h;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);

    SDL_RenderPresent(renderer);
    TTF_CloseFont(police);

    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);

    return waitEnter();
}

int ecritureDansImport(char mangaDispoLong[LONGUEUR_NOM_MANGA_MAX], char mangaDispoCourt[LONGUEUR_COURT], int chapitreChoisis, char teamsCourt[LONGUEUR_COURT])
{
    FILE* fichier = NULL;
    FILE* test = NULL;
    char temp[TAILLE_BUFFER], teamLong[LONGUEUR_NOM_MANGA_MAX];
    int dernierChap = 0, i = 0, j = 0, nombreChapitre = 0;

    teamOfProject(mangaDispoLong, teamLong);

    /*On ouvre le fichier d'import*/
    fichier = fopenR("tmp/import.dat", "a+");

    if(chapitreChoisis != 0)
    {
		nombreChapitre = 1;
        /*On test l'existance du fichier (zipé ou dézipé)*/
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispoLong, chapitreChoisis, CONFIGFILE);
        test = fopenR(temp, "r");
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "manga/%s/%s/Chapitre_%d.zip", teamLong, mangaDispoLong, chapitreChoisis);
        if(test == NULL)
        {
            test = fopenR(temp, "r");
            if(test == NULL)
            {
                fprintf(fichier, "\n%s %s %d", teamsCourt, mangaDispoCourt, chapitreChoisis);
            }
            else
            {
                fclose(test);
            }
        }
        else
            fclose(test);
    }
    else
    {
        test = fopenR(MANGA_DATABASE, "r");
        if(!positionnementApres(test, mangaDispoCourt))
            return -1;
        fscanfs(test, "%d %d", &j, &dernierChap);
        fclose(test);
        for(i = j; i <= dernierChap; i++)
        {
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d/%s", teamLong, mangaDispoLong, i, CONFIGFILE);
            test = fopenR(temp, "r");
            crashTemp(temp, TAILLE_BUFFER);
            sprintf(temp, "manga/%s/%s/Chapitre_%d.zip", teamLong, mangaDispoLong, i);
            if(test == NULL)
            {
                test = fopenR(temp, "r");
                if(test == NULL)
                {
                    fprintf(fichier, "\n%s %s %d", teamsCourt, mangaDispoCourt, i);
					nombreChapitre++;
                }
                else
                {
                    fclose(test);
                }
            }
            else
                fclose(test);
        }
    }
    fclose(fichier);
	return nombreChapitre;
}

void DLmanager() //Equivalent du main, ne fais rien à part lancer et attendre
{
    char temp[TAILLE_BUFFER], texteTrad[SIZE_TRAD_ID_16][LONGUEURTEXTE];
	SDL_Texture *texte = NULL;
    TTF_Font *police = NULL;
    SDL_Color couleurTexte = {POLICE_R, POLICE_G, POLICE_B};
	SDL_Rect position;

/*On affiche la petite fenêtre*/

    window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, SDL_WINDOW_OPENGL);

    SDL_Surface *icon = NULL;
    icon = IMG_Load("data/icone.png");
    if(icon != NULL)
    {
        SDL_SetWindowIcon(window, icon); //Int icon for the main window
        SDL_FreeSurfaceS(icon);
    }
    else
        logR((char*)SDL_GetError());

    nameWindow(1);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawColor(renderer, FOND_R, FOND_G, FOND_B, 255);

    WINDOW_SIZE_W = LARGEUR;
    WINDOW_SIZE_H = HAUTEUR_FENETRE_DL;

    chargement();

    telechargement();

    //Chargement de la traduction
    loadTrad(texteTrad, 16);
    police = TTF_OpenFont(FONTUSED, POLICE_PETIT);
    applyBackground(0, 0, WINDOW_SIZE_W, WINDOW_SIZE_H);
    if(!error)
    {
        texte = TTF_Write(renderer, police, texteTrad[0], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = WINDOW_SIZE_H / 2 - texte->h / 2 * 3;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[1], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(renderer);
        waitEnter();
    }
    else if (error > 0)
    {
        crashTemp(temp, TAILLE_BUFFER);
        sprintf(temp, "%s %d %s", texteTrad[2], error, texteTrad[3]);

        texte = TTF_Write(renderer, police, temp, couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT - texte->h - MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[4], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        texte = TTF_Write(renderer, police, texteTrad[5], couleurTexte);
        position.x = WINDOW_SIZE_W / 2 - texte->w / 2;
        position.y = HAUTEUR_TEXTE_TELECHARGEMENT + texte->h + MINIINTERLIGNE;
        position.h = texte->h;
        position.w = texte->w;
        SDL_RenderCopy(renderer, texte, NULL, &position);
        SDL_DestroyTextureS(texte);

        SDL_RenderPresent(renderer);
        waitEnter();
    }
    TTF_CloseFont(police);
}

void lancementModuleDL()
{
    #ifdef _WIN32
    lancementExternalBinary("Rakshata.exe");
    #else
        #ifdef __APPLE__
        lancementExternalBinary("Rakshata.app");
        #else
        lancementExternalBinary("Rakshata");
        #endif
    #endif
}

