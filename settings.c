/*********************************************************************************************
**      __________         __           .__            __                ____     ____      **
**      \______   \_____  |  | __  _____|  |__ _____ _/  |______    /\  /_   |   /_   |     **
**       |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \/   |   |    |   |     **
**       |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_ /\   |   |    |   |     **
**       |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  / \/   |___| /\ |___|     **
**              \/      \/     \/     \/     \/     \/          \/             \/           **
**                                                                                          **
**   Licence propriétaire, code source confidentiel, distribution formellement interdite    **
**                                                                                          **
*********************************************************************************************/

#include "main.h"

int affichageMenuGestion()
{
    int i = 0, j = 0;
    char menus[SIZE_TRAD_ID_3][LONGUEURTEXTE];
    SDL_Texture *texteAffiche = NULL;
    SDL_Rect position;
    TTF_Font *police;
    SDL_Color couleur = {POLICE_R, POLICE_G, POLICE_B};
    police = TTF_OpenFont(FONTUSED, POLICE_GROS);

    if(WINDOW_SIZE_H != HAUTEUR_SELECTION_REPO)
        updateWindowSize(LARGEUR, HAUTEUR_SELECTION_REPO);

    for(i = 0; i < SIZE_TRAD_ID_3; i++)
    {
        for(j = 0; j < LONGUEURTEXTE; j++)
            menus[i][j] = 0;
    }

    /*Remplissage des variables*/
    loadTrad(menus, 3);

    SDL_RenderClear(renderer);

    texteAffiche = TTF_Write(renderer, police, menus[0], couleur);
    position.x = WINDOW_SIZE_W / 2 - texteAffiche->w / 2;
    position.y = HAUTEUR_TEXTE;
    position.h = texteAffiche->h;
    position.w = texteAffiche->w;
    SDL_RenderCopy(renderer, texteAffiche, NULL, &position);
    SDL_DestroyTextureS(texteAffiche);
    TTF_CloseFont(police);

    return displayMenu(&(menus[1]) , NOMBRE_MENU_GESTION, HAUTEUR_CHOIX);
}

int menuGestion()
{
    int menu = 0;
    while(menu > -3)
    {
        menu = affichageMenuGestion();
        switch(menu)
        {
            case -2:
                menu = -3;
                break;

            case 1:
                /*Ajouter un dépot*/
                menu = ajoutRepo();

                raffraichissmenent();
                break;

            case 2:
                /*Supprimer un dépot*/
                menu = deleteRepo();

                /*Raffraichissement*/
                raffraichissmenent();
                break;

            case 3:
                /*Supprimer des mangas*/
                menu = deleteManga();

                if(menu == -5)
                    menu = -2;
                break;

            case 4:
                /*Raffra”chissement de la BDD*/
                raffraichissmenent();
                break;

            case 5:
                /*Changer langue*/
                menu = changementLangue();
                break;

            case 6:
                menu = PALIER_MENU; //Retour au menu principal
                break;

            default:
                break;
        }
    }
    return menu;
}

char *loadPrefFile()
{
    char *output = NULL;
    FILE* pref = NULL;
    pref = fopen(SETTINGS_FILE, "r");
    if(pref == NULL)
    {
        logR("Couldn't open settings\n");
        return NULL;
    }
    fseek(pref, 0, SEEK_END);
    output = calloc(1, ftell(pref) + 10); //Set at 0
    fclose(pref);

    if(output == NULL)
    {
        logR("Failed at allocate memory\n");
        return NULL;
    }

    AESDecrypt(SETTINGS_PASSWORD, SETTINGS_FILE, output, OUTPUT_IN_MEMORY);

    if(output[0] != '<')
    {
        free(output);
        logR("Incorrect settings decryption\n");
        return NULL;
    }
    return output;
}

void addToPref(char flag, char *stringToAdd)
{
    int i, j, length;
    char setFlag[10], *prefs = NULL, *newPrefs = NULL;
    snprintf(setFlag, 10, "<%c>", flag);

    prefs = loadPrefFile();
    if(prefs == NULL)
        return;

    if(positionnementApresChar(prefs, setFlag))
        removeFromPref(flag);

    i = strlen(prefs);
    length = i + strlen(stringToAdd);
    newPrefs = calloc(1, length);
    if(prefs == NULL)
        return;

    ustrcpy(newPrefs, prefs);
    for(j = 0; i < length && stringToAdd[j]; newPrefs[i++] = stringToAdd[j++]);
    newPrefs[i] = 0;

    AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
}

void removeFromPref(char flag)
{
    int i = 0, length = 0;
    char *newPrefs = NULL;
	char *prefs = NULL;

    prefs = loadPrefFile();
    if(prefs == NULL)
    {
        removeR(SETTINGS_FILE);
        return;
    }
    length = strlen(prefs);
    newPrefs = calloc(1, length);
    if(newPrefs == NULL)
    {
        free(prefs);
        return;
    }

    while(*prefs && i < length)
    {
        if(*prefs == '<' && *(prefs+1) == flag && *(prefs+2) == '>')
        {
            prefs += 3;
            while(*prefs && (*(prefs++) != '<' || *prefs != '/' || *(prefs+1) != flag || *(prefs+2) != '>')); //Balise suivante
            while(*prefs && *(prefs++) != '<'); //Balise suivante
            if(*prefs)
                prefs--;
        }
        else
            newPrefs[i++] = *(prefs++);
    }
    newPrefs[i] = 0;
    AESEncrypt(SETTINGS_PASSWORD, newPrefs, SETTINGS_FILE, INPUT_IN_MEMORY);
}

void updatePrefs(char flag, char *stringToAdd)
{
    removeFromPref(flag);
    addToPref(flag, stringToAdd);
}

int loadEmailProfile()
{
    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
		int i = 0;
        char flag[10];
        sprintf(flag, "<%c>", SETTINGS_EMAIL_FLAG);
        if((i = positionnementApresChar(prefs, flag)))
        {
            sscanfs(&(prefs[i]), "%s", COMPTE_PRINCIPAL_MAIL, 100);
            free(prefs);
            for(i = 0; COMPTE_PRINCIPAL_MAIL[i] && COMPTE_PRINCIPAL_MAIL[i] != '@'; i++);
            if(COMPTE_PRINCIPAL_MAIL[i] && i != 0)
                return 1;
        }
        else
            free(prefs);
    }
    crashTemp(COMPTE_PRINCIPAL_MAIL, 100);
    return 0;
}

int loadLangueProfile()
{
    int i = 0;
    char *prefs = loadPrefFile();
    if(prefs != NULL)
    {
        char flag[10];
        sprintf(flag, "<%c>", SETTINGS_LANGUE_FLAG);
        if((i = positionnementApresChar(prefs, flag)))
        {
            sscanfs(prefs+i, "%d", &langue);
            free(prefs);
            if(langue > 0 && langue <= NOMBRE_LANGUE)
                return 0;
            langue = 0;
        }
        else
            free(prefs);
    }
    mkdirR("data");
    char temp[100];
    langue = LANGUE_PAR_DEFAUT;
    sprintf(temp, "<%c>\n%d\n</%c>", SETTINGS_LANGUE_FLAG, langue, SETTINGS_LANGUE_FLAG);
    AESEncrypt(SETTINGS_PASSWORD, temp, SETTINGS_FILE, INPUT_IN_MEMORY);
    return 1;
}

char* loadLargePrefs(char flag)
{
	char *prefs = loadPrefFile();
	if(prefs != NULL)
	{
		int i;
		size_t bufferSize = 0;
		char flag_db[10];
		sprintf(flag_db, "<%c>", flag);
		if((i = positionnementApresChar(prefs, flag_db)))
		{
			prefs += i;
			while(prefs[++bufferSize] && (prefs[bufferSize] != '<' || prefs[bufferSize+1] != '/' || prefs[bufferSize+2] != flag || prefs[bufferSize+3] != '>'));
			char* databaseRAW = calloc(1, bufferSize + 5);
			if(databaseRAW != NULL)
			{
				memccpy(databaseRAW, prefs, 0, bufferSize);
				databaseRAW[bufferSize] = 0;
				return databaseRAW;

			}
		}
		else if (flag == SETTINGS_MANGADB_FLAG || flag == SETTINGS_REPODB_FLAG)
		{
			char temp[200], *buffer = NULL;
			if(flag == SETTINGS_MANGADB_FLAG)
				snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, MANGA_DATABASE);
			else
				snprintf(temp, 200, "http://www.%s/Recover/%d/%s", MAIN_SERVER_URL[0], CURRENTVERSION, REPO_DATABASE);
			buffer = calloc(1, 0x10000);
			setupBufferDL(buffer, 0x10, 0x10, 0x10, 0x10);
			download(temp, buffer, 0);
			addToPref(flag, buffer);
		}
	}
	return NULL;
}

