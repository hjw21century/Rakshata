/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#include "main.h"
#include "lecteur.h"

/**	Set up the evnt	**/

int reader_getNextReadableElement(MANGAS_DATA mangaDB, bool isTome, int *currentPosIntoStructure)
{
	if(isTome)
	{
		for((*currentPosIntoStructure)++; *currentPosIntoStructure < mangaDB.nombreTomes &&
						!checkReadable(mangaDB, isTome, &mangaDB.tomes[*currentPosIntoStructure]) ; (*currentPosIntoStructure)++);
		
		return *currentPosIntoStructure < mangaDB.nombreTomes;	//As-ton trouvé un tome?
	}
	
	for((*currentPosIntoStructure)++; *currentPosIntoStructure < mangaDB.nombreChapitre &&
				!checkReadable(mangaDB, isTome, &mangaDB.chapitres[*currentPosIntoStructure]) ; (*currentPosIntoStructure)++);

	return *currentPosIntoStructure < mangaDB.nombreChapitre;	//As-ton trouvé un tome?
}

int reader_getCurrentPageIfRestore(char localization[SIZE_TRAD_ID_21][TRAD_LENGTH])
{
	if(checkRestore())
    {
		int var;
		
		reader_loadStateForRestore(NULL, NULL, NULL, &var, true);
		reader_notifyUserRestore(localization);

		return var;
    }
	return 0;
}

/**	Load the reader data	**/

int configFileLoader(MANGAS_DATA *mangaDB, bool isTome, int chapitre_tome, DATA_LECTURE* dataReader)
{
    int i, prevPos = 0, nombrePages = 0, posID = 0, nombreTours = 1, lengthBasePath, lengthFullPath;
    char name[LONGUEUR_NOM_PAGE];
    FILE* config = NULL;
	
    void * intermediaryPtr;
    bool allocError;
	
    dataReader->nombrePageTotale = 1;
	
    dataReader->nomPages = dataReader->path = NULL;
    dataReader->pathNumber = dataReader->pageCouranteDuChapitre = dataReader->chapitreTomeCPT = NULL;
	
    if(isTome)
    {
        snprintf(name, LONGUEUR_NOM_PAGE, "manga/%s/%s/Tome_%d/%s", mangaDB->team->teamLong, mangaDB->mangaName, chapitre_tome, CONFIGFILETOME);
        config = fopen(name, "r");
        if(config == NULL)
		{
			char log[LONGUEUR_NOM_PAGE+50];
			snprintf(log, LONGUEUR_NOM_PAGE+50, "Error: Couldn't open volume data: %s", name);
			logR(log);
			return 1;
		}

        name[0] = 0;
        fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
    }
    else
    {
        if(chapitre_tome%10)
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d.%d", chapitre_tome/10, chapitre_tome%10);
        else
            snprintf(name, LONGUEUR_NOM_PAGE, "Chapitre_%d", chapitre_tome/10);
    }
	
    do
    {
        char input_path[LONGUEUR_NOM_PAGE], **nomPagesTmp = NULL;
		
        snprintf(input_path, LONGUEUR_NOM_PAGE, "manga/%s/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name, CONFIGFILE);
        allocError = false;
		
        nomPagesTmp = loadChapterConfigDat(input_path, &nombrePages);
        if(nomPagesTmp != NULL)
        {
            /*On réalloue la mémoire en utilisant un buffer intermédiaire*/
            dataReader->nombrePageTotale += nombrePages;
			
            ///pathNumber
            intermediaryPtr = realloc(dataReader->pathNumber, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pathNumber = intermediaryPtr;
            else
                allocError = true;
			
            ///pageCouranteDuChapitre
            intermediaryPtr = realloc(dataReader->pageCouranteDuChapitre, (dataReader->nombrePageTotale+1) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->pageCouranteDuChapitre = intermediaryPtr;
            else
                allocError = true;
			
            ///nomPages
            intermediaryPtr = realloc(dataReader->nomPages, (dataReader->nombrePageTotale+1) * sizeof(char*));
            if(intermediaryPtr != NULL)
                dataReader->nomPages = intermediaryPtr;
            else
                allocError = true;
			
            ///chapitreTomeCPT
            intermediaryPtr = realloc(dataReader->chapitreTomeCPT, (++nombreTours) * sizeof(int));
            if(intermediaryPtr != NULL)
                dataReader->chapitreTomeCPT = intermediaryPtr;
            else
                allocError = true;
			
            ///path
            intermediaryPtr = realloc(dataReader->path, nombreTours * sizeof(char*));
            if(intermediaryPtr != NULL)
            {
                dataReader->path = intermediaryPtr;
                dataReader->path[nombreTours-2] = malloc(LONGUEUR_NOM_PAGE);
                
                if(dataReader->path[nombreTours-2] == NULL)
                    allocError = true;
            }
            else
                allocError = true;
			
            if(allocError)  //Si on a eu un problème en allouant de la mémoire
            {
				if(dataReader->pathNumber == NULL || dataReader->pageCouranteDuChapitre == NULL || dataReader->nomPages == NULL || dataReader->chapitreTomeCPT == NULL || dataReader->path == NULL || dataReader->path[nombreTours-2] == NULL)
				{
					free(dataReader->pathNumber);				dataReader->pathNumber = NULL;
					free(dataReader->pageCouranteDuChapitre);	dataReader->pageCouranteDuChapitre = NULL;
					free(dataReader->nomPages);					dataReader->nomPages = NULL;
					free(dataReader->chapitreTomeCPT);			dataReader->chapitreTomeCPT = NULL;
					if(dataReader->path != NULL)
						free(dataReader->path[nombreTours-2]);
					
					free(dataReader->path);						dataReader->path = NULL;
				}
                dataReader->nombrePageTotale -= nombrePages;
                nombreTours--;
            }
            else
            {
                dataReader->path[nombreTours-1] = NULL;
				
                snprintf(dataReader->path[posID], LONGUEUR_NOM_PAGE, "manga/%s/%s/%s", mangaDB->team->teamLong, mangaDB->mangaName, name);
                if(isTome)
                    dataReader->chapitreTomeCPT[posID] = extractNumFromConfigTome(name, chapitre_tome);
                else
                    dataReader->chapitreTomeCPT[posID] = chapitre_tome;
				
                lengthBasePath = strlen(dataReader->path[posID]);
				
                for(i = 0; prevPos < dataReader->nombrePageTotale; prevPos++) //Réinintialisation
                {
                    lengthFullPath = lengthBasePath + strlen(nomPagesTmp[i]) + 0x10; // '/' + \0 + margin
                    dataReader->nomPages[prevPos] = malloc(lengthFullPath);
                    if(dataReader->nomPages[prevPos] != NULL)
                    {
                        snprintf(dataReader->nomPages[prevPos], lengthFullPath, "%s/%s", dataReader->path[posID], nomPagesTmp[i]);
                        dataReader->pathNumber[prevPos] = posID;
                        dataReader->pageCouranteDuChapitre[prevPos] = i++;
                    }
                    else    //Si problème d'allocation
                    {
                        prevPos--;
                    }
                }
                posID++;
                for(i = 0; nomPagesTmp[i] != NULL; free(nomPagesTmp[i++]));
            }
			
			free(nomPagesTmp);
        }
		
        if(isTome)
        {
            if(fgetc(config) == EOF)
            {
                fclose(config);
                break;
            }
            fseek(config, -1, SEEK_CUR);
            fscanfs(config, "%s", name, LONGUEUR_NOM_PAGE);
        }
    } while(isTome && posID < LONGUEUR_NOM_PAGE);
	
    if(dataReader->pathNumber != NULL && dataReader->nomPages != NULL)
    {
        dataReader->IDDisplayed = chapitre_tome;
        dataReader->pathNumber[prevPos] = VALEUR_FIN_STRUCTURE_CHAPITRE;
        dataReader->nomPages[dataReader->nombrePageTotale] = NULL; //On signale la fin de la structure
        dataReader->nombrePageTotale--; //Décallage pour l'utilisation dans le lecteur
    }
    if(dataReader->pageCourante > dataReader->nombrePageTotale)
        dataReader->pageCourante = dataReader->nombrePageTotale;
    return 0;
}

char ** loadChapterConfigDat(char* input, int *nombrePage)
{
    char ** output;
    int i, j, scriptUtilise = 0;
    FILE* file_input = fopenR(input, "r");
	if(file_input == NULL)
        return NULL;
	
    fscanfs(file_input, "%d", nombrePage);
	
    if(fgetc(file_input) != EOF)
    {
        fseek(file_input, -1, SEEK_CUR);
        if(fgetc(file_input) == 'N')
            scriptUtilise = 1;
        else
            fseek(file_input, -1, SEEK_CUR);
		
        output = calloc(5+*nombrePage, sizeof(char*));
		
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            if(!scriptUtilise)
                fscanfs(file_input, "%d %s\n", &j, output[i], LONGUEUR_NOM_PAGE);
			
            else
                fscanfs(file_input, "%d %s", &j, output[i], LONGUEUR_NOM_PAGE);
            changeTo(output[i], '&', ' ');
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
	
    else
    {
        (*nombrePage)++;
        output = calloc(5+*nombrePage, sizeof(char*));
        for(i = 0; i < *nombrePage; i++)
        {
            output[i] = malloc(LONGUEUR_NOM_PAGE+1);
            snprintf(output[i], LONGUEUR_NOM_PAGE, "%d.jpg", i);
        }
        output[i] = malloc(LONGUEUR_NOM_PAGE);
        output[i][0] = 0;
    }
    fclose(file_input);
    for(i = strlen(input); i > 0 && input[i] != '/'; input[i--] = 0);
	
    char temp[300];
    for(i = *nombrePage; i >= 0; i--)
    {
        if(output[i][0])
        {
            snprintf(temp, 300, "%s%s", input, output[i]);
            if(checkFileExist(temp))
            {
                *nombrePage = i;
                break;
            }
        }
    }
    return output;
}

/**	Load pages	**/

void reader_switchToNextPage(SDL_Surface ** prevPage, SDL_Surface ** page, SDL_Texture ** pageTexture, bool pageTooBigToLoad, SDL_Surface ** nextPage)
{
	MUTEX_UNIX_LOCK;
	
	SDL_FreeSurfaceS(*prevPage);	//Il gère le cas où la surface
	*prevPage = NULL;				//est nulle, pas de checks requis
	
	*prevPage = SDL_CreateRGBSurface(0, (*page)->w, (*page)->h, 32, 0, 0, 0, 0);
	SDL_FillRect(*prevPage, NULL, SDL_MapRGB((*prevPage)->format, palette.fond.r, palette.fond.g, palette.fond.b));
	SDL_BlitSurface(*page, NULL, *prevPage, NULL);
	
	SDL_FreeSurfaceS(*page);
	freeCurrentPage(*pageTexture, pageTooBigToLoad);
	*pageTexture = NULL;
	
	*page = SDL_CreateRGBSurface(0, (*nextPage)->w, (*nextPage)->h, 32, 0, 0, 0, 0);
	SDL_FillRect(*page, NULL, SDL_MapRGB((*page)->format, palette.fond.r, palette.fond.g, palette.fond.b));
	SDL_BlitSurface(*nextPage, NULL, *page, NULL);
	SDL_FreeSurfaceS(*nextPage);
	*nextPage = NULL;
	
	MUTEX_UNIX_UNLOCK;
}

void reader_switchToPrevPage(SDL_Surface ** prevPage, SDL_Surface ** page, SDL_Texture ** pageTexture, bool pageTooBigToLoad, SDL_Surface ** nextPage)
{
	MUTEX_UNIX_LOCK;
	
	SDL_FreeSurfaceS(*nextPage);	//Il gère le cas où la surface
	*nextPage = NULL;				//est nulle, pas de checks requis
	
	*nextPage = SDL_CreateRGBSurface(0, (*page)->w, (*page)->h, 32, 0, 0, 0, 0);
	SDL_FillRect(*nextPage, NULL, SDL_MapRGB((*nextPage)->format, palette.fond.r, palette.fond.g, palette.fond.b));
	SDL_BlitSurface(*page, NULL, *nextPage, NULL);
	
	SDL_FreeSurfaceS(*page);
	freeCurrentPage(*pageTexture, pageTooBigToLoad);
	*pageTexture = NULL;
	
	*page = SDL_CreateRGBSurface(0, (*prevPage)->w, (*prevPage)->h, 32, 0, 0, 0, 0);
	SDL_FillRect(*page, NULL, SDL_MapRGB((*page)->format, palette.fond.r, palette.fond.g, palette.fond.b));
	SDL_BlitSurface(*prevPage, NULL, *page, NULL);
	SDL_FreeSurfaceS(*prevPage);
	*prevPage = NULL;
	
	MUTEX_UNIX_UNLOCK;
}

void reader_loadInitialPage(DATA_LECTURE dataReader, SDL_Surface ** prevPage, SDL_Surface ** page)
{
	if(dataReader.pageCourante > 0) //Si il faut charger la page n - 1
	{
		*prevPage = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.nomPages[dataReader.pageCourante - 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante - 1]);
	}
	
	*page = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante]], dataReader.nomPages[dataReader.pageCourante], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante]);

	if((dataReader.pageCourante > 0 && *prevPage == NULL) || *page == NULL)
	{
		SDL_FreeSurfaceS(*prevPage);
		SDL_FreeSurfaceS(*page);
		*prevPage = *page = NULL;
	}
}

SDL_Texture * reader_getPageTexture(SDL_Surface *pageSurface, bool * pageTooBigToLoad)
{
	MUTEX_UNIX_LOCK;
	SDL_Texture *pageTexture = SDL_CreateTextureFromSurface(renderer, pageSurface);
	MUTEX_UNIX_UNLOCK;
	
	if(pageTexture == NULL)
	{
		MUTEX_UNIX_LOCK;
		int sizeMax = defineMaxTextureSize(pageSurface->h), nombreParties;
		int nombreMiniTexture = pageSurface->h/sizeMax + (pageSurface->h%sizeMax?1:0);
		SDL_Texture **texture = calloc(nombreMiniTexture+1, sizeof(SDL_Texture*));
		SDL_Surface *pageBuf = NULL;
		SDL_Rect pos;
		pos.w = pageSurface->w;
		pos.h = sizeMax;
		for(pos.x = nombreParties = 0; nombreParties < nombreMiniTexture-1; nombreParties++)
		{
			pos.y = nombreParties * sizeMax;
			pageBuf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
			SDL_SetColorKey(pageBuf, SDL_TRUE, SDL_MapRGB(pageBuf->format, palette.fond.r, palette.fond.g, palette.fond.b));
			SDL_BlitSurface(pageSurface, &pos, pageBuf, NULL);
			texture[nombreParties] = SDL_CreateTextureFromSurface(renderer, pageBuf);
			SDL_FreeSurface(pageBuf);
		}
		if(sizeMax && nombreParties && pageSurface->h % sizeMax)
		{
			pos.y = nombreParties * sizeMax;
			pos.h = pageSurface->h % pos.y;
			pageBuf = SDL_CreateRGBSurface(0, pos.w, pos.h, 32, 0, 0 , 0, 0);
			SDL_BlitSurface(pageSurface, &pos, pageBuf, NULL);
			texture[nombreParties] = SDL_CreateTextureFromSurface(renderer, pageBuf);
			SDL_FreeSurface(pageBuf);
		}
		*pageTooBigToLoad = true;
		pageTexture = (SDL_Texture*) texture;
		MUTEX_UNIX_UNLOCK;
	}
	else
		*pageTooBigToLoad = false;
	
	return pageTexture;
}

SDL_Surface * reader_bufferisePages(DATA_LECTURE dataReader, bool bufNextPage)
{
	SDL_Surface *srfOut;
	if(bufNextPage)
	{
		if(dataReader.pageCourante >= dataReader.nombrePageTotale)
			srfOut = NULL;
		else
		{
			srfOut = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.nomPages[dataReader.pageCourante + 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante + 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante + 1]);
		}
	}
	else
	{
		if(dataReader.pageCourante <= 0)
			srfOut = NULL;
		
		else
		{
			srfOut = IMG_LoadS(dataReader.path[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.nomPages[dataReader.pageCourante - 1], dataReader.chapitreTomeCPT[dataReader.pathNumber[dataReader.pageCourante - 1]], dataReader.pageCouranteDuChapitre[dataReader.pageCourante - 1]);
		}
	}
	return srfOut;
}

void reader_initPagePosition(SDL_Surface * page, bool fullscreen, bool pageTooBigForScreen, SDL_Rect *positionPage, SDL_Rect *positionSlide)
{
	if(pageTooBigForScreen)
	{
		positionPage->x = 0;
		positionPage->y = BORDURE_HOR_LECTURE;

		positionPage->w = positionSlide->w = getPtRetinaW(renderer) - BORDURE_LAT_LECTURE;
		positionPage->h = positionSlide->h = getPtRetinaH(renderer) - BORDURE_HOR_LECTURE;
		
		positionSlide->x = page->w - positionSlide->w;
		positionSlide->y = 0;
	}
	
	else
	{
		positionPage->x = getPtRetinaW(renderer) / 2 - page->w / 2;
		positionPage->y = BORDURE_HOR_LECTURE;

		positionPage->w = positionSlide->w = page->w;
		positionPage->h = positionSlide->h = page->h;
		
		positionSlide->x = positionSlide->y = 0;
		
	}
	
	//Si en fullscreen et que la page ne fait pas toute la hauteur, on la centre
	if(fullscreen && page->h < getPtRetinaH(renderer) - BORDURE_HOR_LECTURE - BORDURE_CONTROLE_LECTEUR)
		positionPage->y += (getPtRetinaH(renderer) - BORDURE_CONTROLE_LECTEUR - BORDURE_HOR_LECTURE - page->h) / 2;
}

/**	Load environnement data	**/
void reader_setContextData(int * largeurMax, int * hauteurMax, bool fullscreen, SDL_Surface page, bool * pageTooBigForScreen)
{
	//Set max dimensions
	*largeurMax = page.w + BORDURE_LAT_LECTURE * 2;
	*hauteurMax = page.h + BORDURE_HOR_LECTURE + BORDURE_CONTROLE_LECTEUR;
	
	if(*hauteurMax > RESOLUTION[1] - BARRE_DES_TACHES_WINDOWS)
		*hauteurMax = RESOLUTION[1] - BARRE_DES_TACHES_WINDOWS;
	
	/*Initialisation des différentes surfaces*/
	if(!fullscreen)
	{
		*pageTooBigForScreen = (*largeurMax > LARGEUR_MAX_LECTEUR);

		if(*pageTooBigForScreen)
			*largeurMax = LARGEUR_MAX_LECTEUR;
		
		else if(*largeurMax < LARGEUR)
			*largeurMax = LARGEUR;
	}
	else
	{
		*pageTooBigForScreen = *largeurMax > getPtRetinaW(renderer);
	}
}

void reader_setScreenToSize(int largeurMax, int hauteurMax, bool fullscreen, bool changementEtat, SDL_Texture ** controlBar, SDL_Rect *positionControlBar, bool isFavoris)
{
	if(!fullscreen && changementEtat)
	{
		MUTEX_UNIX_LOCK;
		SDL_FlushEvent(SDL_WINDOWEVENT);
		SDL_SetWindowFullscreen(window, SDL_FALSE);
		SDL_FlushEvent(SDL_WINDOWEVENT);
		
		//We restart the window
		MUTEX_LOCK(mutex);
		SDL_DestroyTexture(*controlBar);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		window = SDL_CreateWindow(PROJECT_NAME, RESOLUTION[0] / 2 - LARGEUR / 2, 25, largeurMax, hauteurMax, CREATE_WINDOW_FLAG|SDL_WINDOW_SHOWN);
		
		WINDOW_SIZE_W = getPtRetinaW(renderer);
		WINDOW_SIZE_H = getPtRetinaH(renderer);
		
		loadIcon(window);
		nameWindow(window, 0);
		renderer = setupRendererSafe(window);
		*controlBar = loadControlBar(isFavoris);
		SDL_FlushEvent(SDL_WINDOWEVENT);
		MUTEX_UNLOCK(mutex);
		MUTEX_UNIX_UNLOCK;
	}
	else if(!fullscreen)
	{
		updateWindowSize(largeurMax, hauteurMax);
		SDL_RenderClear(renderer);
	}
	else if(fullscreen && changementEtat)
	{
		SDL_FlushEvent(SDL_WINDOWEVENT);
		SDL_DestroyTexture(*controlBar);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 0, 0, CREATE_WINDOW_FLAG|SDL_WINDOW_FULLSCREEN_DESKTOP);
		
		loadIcon(window);
		nameWindow(window, 0);
		renderer = setupRendererSafe(window);
		*controlBar = loadControlBar(isFavoris);
		SDL_FlushEvent(SDL_WINDOWEVENT);
		
		WINDOW_SIZE_W = RESOLUTION[0] = getPtRetinaW(renderer);
		WINDOW_SIZE_H = RESOLUTION[1] = getPtRetinaH(renderer);
		
		SDL_RenderClear(renderer);
		SDL_RenderPresent(renderer);
	}
	
	reader_initializePosControlBar(*controlBar, positionControlBar);
}

int changementDePage(MANGAS_DATA *mangaDB, DATA_LECTURE* dataReader, bool goToNextPage, int *changementPage, bool isTome, int *chapitreChoisis, int currentPosIntoStructure)
{
    int ret_value;
	
    if(goToNextPage) //Page suivante
    {
        if (dataReader->pageCourante < dataReader->nombrePageTotale) //Changement de page
        {
            dataReader->pageCourante += 1;
            *changementPage = READER_ETAT_NEXTPAGE;
			ret_value = READER_CHANGEPAGE_SUCCESS;
        }
        else if(changeChapter(mangaDB, isTome, chapitreChoisis, currentPosIntoStructure, goToNextPage)) //On envois l'ordre de passer au chapitre suivant
        {
            ret_value = READER_CHANGEPAGE_NEXTCHAP;
        }
        else //On met le menu en rouge
        {
            ret_value = READER_CHANGEPAGE_UPDATE_TOPBAR;
        }
    }
    else
    {
        if (dataReader->pageCourante > 0)
        {
            dataReader->pageCourante -= 1;
            *changementPage = READER_ETAT_PREVPAGE;
			ret_value = READER_CHANGEPAGE_SUCCESS;
        }
        else if(changeChapter(mangaDB, isTome, chapitreChoisis, currentPosIntoStructure, goToNextPage))
        {
            ret_value = READER_CHANGEPAGE_NEXTCHAP;
        }
        else
            ret_value = READER_CHANGEPAGE_UPDATE_TOPBAR;
    }
	SDL_FlushEvent(SDL_KEYDOWN);
    return ret_value;
}

bool changeChapter(MANGAS_DATA* mangaDB, bool isTome, int *ptrToSelectedID, int posIntoStruc, bool goToNextChap)
{
	posIntoStruc += (goToNextChap ? 1 : -1);
	
	if(changeChapterAllowed(mangaDB, isTome, posIntoStruc))
	{
		if(isTome)
            *ptrToSelectedID = mangaDB->tomes[posIntoStruc].ID;
        else
            *ptrToSelectedID = mangaDB->chapitres[posIntoStruc];
		return true;
	}
	return false;
}

bool changeChapterAllowed(MANGAS_DATA* mangaDB, bool isTome, int posIntoStruc)
{
    getUpdatedCTList(mangaDB, isTome);
	
	return (isTome && posIntoStruc < mangaDB->nombreTomes) || (!isTome && posIntoStruc < mangaDB->nombreChapitre);
}
