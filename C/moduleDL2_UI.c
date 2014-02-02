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
#include "moduleDL.h"

SDL_Renderer *rendererDL = NULL;

#ifndef _WIN32

static int flag;
static void *arg1;
static void *arg2;
static void *arg3;
static SDL_Texture *arg4;

pthread_mutex_t mutexAskUIThreadWIP = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexStartUIThread = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condResumeExecution = PTHREAD_COND_INITIALIZER;

#define MDL_TUI_QUIT 1
#define MDL_TUI_COPYTEXTURE 2
#define MDL_TUI_PRINT_BACKGROUND 3
#define MDL_TUI_REFRESH 4
#define MDL_TUI_TTFWRITE 5
#define MDL_TUI_DESTTXT 6
#define MDL_TUI_CTFS 7
#define MDL_TUI_LOADIMG 8

void MDLUIThread()
{
    bool quit = false;

    SDL_FlushEvent(SDL_WINDOWEVENT);
    SDL_Window * window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, CREATE_WINDOW_FLAG);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    loadIcon(window);
    nameWindow(window, 2);

    SDL_FlushEvent(SDL_WINDOWEVENT);
    rendererDL = setupRendererSafe(window);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    WINDOW_SIZE_W_DL = getW(rendererDL);
    WINDOW_SIZE_H_DL = getH(rendererDL);
    isRetina = checkIfRetina(window);

    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);
    SDL_RenderClear(rendererDL);
    MUTEX_LOCK(mutexStartUIThread);

    while(!quit)
    {
        MUTEX_LOCK(mutexStartUIThread); //Ce seconde lock bloque l'execution jusqu'à que pthread_cond le débloque
		
        if(flag)
        {
            switch(flag)
            {
                case MDL_TUI_QUIT:
                {
                    quit = true;
                    break;
                }
                case MDL_TUI_COPYTEXTURE:
                {
                    SDL_RenderCopy(rendererDL, (SDL_Texture *) arg1, (SDL_Rect *) arg2, (SDL_Rect *) arg3);
                    SDL_RenderPresent(rendererDL);
                    arg1 = arg2 = arg3 = 0;
                    break;
                }
                case MDL_TUI_PRINT_BACKGROUND:
                {
                    SDL_RenderFillRect(rendererDL, (SDL_Rect *) arg1);
                    break;
                }

                case MDL_TUI_REFRESH:
                {
                    SDL_RenderPresent(rendererDL);
                    break;
                }
                case MDL_TUI_TTFWRITE:
                {
                    arg4 = TTF_Write(rendererDL, (TTF_Font*) arg1, (char*) arg2, *(SDL_Color *) arg3);
                    break;
                }
                case MDL_TUI_DESTTXT:
                {
                    SDL_DestroyTexture((SDL_Texture *) arg1);
                    break;
                }
                case MDL_TUI_CTFS:
                {
                    arg2 = SDL_CreateTextureFromSurface(rendererDL, (SDL_Surface *) arg1);
                    break;
                }
                case MDL_TUI_LOADIMG:
                {
                    arg2 = IMG_LoadTexture(rendererDL, (char*) arg1);
                    break;
                }
            }
			
            pthread_cond_broadcast(&condResumeExecution);	//On a reçu la requête, le thread sera libéré dès que le mutex sera debloqué
			MUTEX_UNLOCK(mutexStartUIThread);
			
			flag = 0;
			
			if(!quit)
			{
				SDL_Delay(5);
				while(!pthread_mutex_trylock(&mutexStartUIThread))   //On attend le lock
				{
					MUTEX_UNLOCK(mutexStartUIThread);
					if(flag != 0)	//Si nouvelle requête reçue
						break;
					else
						SDL_Delay(10);
				}
			}
        }
	}
    SDL_DestroyRenderer(rendererDL);
    SDL_DestroyWindow(window);
    rendererDL = NULL;
    quit_thread(0);
}
#endif

void startMDLUIThread()
{
#ifdef _WIN32

    for(; !isDirectXLoaded(); SDL_Delay(rand() % 50));

    SDL_FlushEvent(SDL_WINDOWEVENT);
    SDL_Window * window = SDL_CreateWindow(PROJECT_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGEUR, HAUTEUR_FENETRE_DL, CREATE_WINDOW_FLAG);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    loadIcon(window);
    nameWindow(window, 2);

    SDL_FlushEvent(SDL_WINDOWEVENT);
    rendererDL = setupRendererSafe(window);
    SDL_FlushEvent(SDL_WINDOWEVENT);

    WINDOW_SIZE_W_DL = getW(rendererDL);
    WINDOW_SIZE_H_DL = getH(rendererDL);
    isRetina = checkIfRetina(window);

    chargement(rendererDL, WINDOW_SIZE_H_DL, WINDOW_SIZE_W_DL);
    SDL_RenderClear(rendererDL);
#else
    createNewThread(MDLUIThread, NULL);
	while(!pthread_mutex_trylock(&mutexStartUIThread))
	{
		MUTEX_UNLOCK(mutexStartUIThread);
		SDL_Delay(25);
	}
#endif // _WIN32
}

void MDLTUIQuit()
{
    if(rendererDL == NULL)
        return;

    MUTEX_LOCK(mutexAskUIThreadWIP);
#ifdef _WIN32
    SDL_Window * window = rendererDL->window;
    SDL_DestroyRenderer(rendererDL);
    SDL_DestroyWindow(window);
    rendererDL = NULL;
#else
    flag = MDL_TUI_QUIT;
    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);

    pthread_cond_destroy(&condResumeExecution);
    pthread_mutex_destroy(&mutexStartUIThread);
#endif
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
}

void MDLTUICopy(SDL_Texture * texture, SDL_Rect * pos1, SDL_Rect * pos2)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
#ifdef _WIN32
    SDL_RenderCopy(rendererDL, texture, pos1, pos2);
#else
    flag = MDL_TUI_COPYTEXTURE;
    arg1 = texture;
    arg2 = pos1;
    arg3 = pos2;

    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
#endif // _WIN32
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
}

void MDLTUIBackground(int x, int y, int w, int h)
{
    SDL_Rect pos;
    pos.x = x;
    pos.y = y;
    pos.h = h;
    pos.w = w;
    MDLTUIBackgroundPreCrafted(&pos);
}

void MDLTUIBackgroundPreCrafted(SDL_Rect * pos)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
#ifdef _WIN32
    SDL_RenderFillRect(rendererDL, pos);
#else
    flag = MDL_TUI_PRINT_BACKGROUND;
    arg1 = pos;

    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
    arg1 = NULL;
#endif // _WIN32
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
}

void MDLTUIRefresh()
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
#ifdef _WIN32
    SDL_RenderPresent(rendererDL);
#else
    flag = MDL_TUI_REFRESH;
    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
#endif // _WIN32
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
}

SDL_Texture * MDLTUITTFWrite(TTF_Font * police, char * texte, SDL_Color couleur)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
	void * output;

#ifdef _WIN32
    output = TTF_Write(rendererDL, police, texte, couleur);
#else
    flag = MDL_TUI_TTFWRITE;
    arg1 = police;
    arg2 = texte;
    arg3 = &couleur;

    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
    arg3 = NULL;
	output = arg4;
#endif
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
    return output;
}

void MDLTUIDestroyTexture(SDL_Texture * texture)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
#ifdef _WIN32
    SDL_DestroyTexture(texture);
#else
    flag = MDL_TUI_DESTTXT;
    arg1 = texture;
    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
#endif
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
}

SDL_Texture * MDLTUICreateTextureFromSurface(SDL_Surface * surface)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
	void * output;
#ifdef _WIN32
    output = SDL_CreateTextureFromSurface(rendererDL, surface);
#else
    flag = MDL_TUI_CTFS;
    arg1 = surface;
    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
	output = arg2;
#endif
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
    return output;
}

SDL_Texture * MDLTUILoadIMG(SDL_Renderer * rendererVar, char* filename)
{
    MUTEX_LOCK(mutexAskUIThreadWIP);
	void * output;
#ifdef _WIN32
    output = IMG_LoadTexture(rendererVar, filename);
#else
    flag = MDL_TUI_LOADIMG;
    arg1 = filename;
    pthread_cond_wait(&condResumeExecution, &mutexStartUIThread);
	output = arg2;
#endif
    MUTEX_UNLOCK(mutexAskUIThreadWIP);
    return output;
}
