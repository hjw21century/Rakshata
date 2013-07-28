/*********************************************************************************************
**  __________         __           .__            __                  ____     ________    **
**  \______   \_____  |  | __  _____|  |__ _____ _/  |______    ___  _/_   |    \_____  \   **
**   |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \   \  \/ /|   |     /  ____/   **
**   |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   / |   |    /       \   **
**   |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /   \_/  |___| /\ \_______ \  **
**          \/      \/     \/     \/     \/     \/          \/               \/         \/  **
**                                                                                          **
**    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
**                                                                                          **
*********************************************************************************************/

#include <stdarg.h>
#include "main.h"

#define USE_FULL_PATH

/**Fonction de base modifiés**/
#ifdef USE_FULL_PATH

FILE* fopenR(void *_path, char *right)
{
    unsigned char *path = _path;
    unsigned char *temp = ralloc((ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2));
	FILE* output = NULL;
	if(temp == NULL)
        return NULL;
    snprintf((char *)temp, ustrlen(path) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, path);
    output = fopen((char *) temp, right);
    free(temp);
    return output;
}

void removeR(char *path)
{
	char *temp = ralloc(strlen(path) + strlen(REPERTOIREEXECUTION) + 2);
    snprintf(temp, strlen(path) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, path);
    remove(temp);
	free(temp);
}

void renameR(char *initialName, char *newName)
{
	char *temp = malloc(strlen(initialName) + strlen(REPERTOIREEXECUTION) + 2); //+1 pour le / et +1 pour \0
	char *temp2 = malloc(strlen(newName) + strlen(REPERTOIREEXECUTION) + 2);

    if(initialName[1] != ':')
        snprintf(temp, strlen(initialName) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, initialName);
    else
        strcpy(temp, initialName);
    if(newName[1] != ':')
        snprintf(temp2, strlen(newName) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, newName);
    else
        strcpy(temp, newName);
    rename(temp, temp2);
	free(temp);
	free(temp2);
}
#else
    #define fopenR(a, b) fopen(a, b)
    #define removeR(a) remove(a)
    #define renameR(a, b) rename(a, b)
#endif

void mkdirR(char *path)
{
#ifdef USE_FULL_PATH
    char *temp = malloc(strlen(path) + strlen(REPERTOIREEXECUTION) + 2);
    snprintf(temp, strlen(path) + strlen(REPERTOIREEXECUTION) + 2, "%s/%s", REPERTOIREEXECUTION, path);
#else
    char *temp = path;
#endif

#ifdef _WIN32
    mkdir(temp);
#else
    mkdir(temp, PERMISSIONS);
#endif

#ifdef USE_FULL_PATH
    free(temp);
#endif
}

void chdirR()
{
	chdir(REPERTOIREEXECUTION);
}

void resetOriginalCHDir(int *argc, char** argv)
{
    size_t length = strlen(argv[0])+1;
#ifdef __APPLE__
    if(argv[0] != NULL) {

        int i;
        char *newPath = malloc(length+20);
        if(newPath != NULL)
        {
            for(i = length -1; i > 0 && argv[0][i] != '/'; argv[0][i--] = 0);
            snprintf(newPath, length+19, "%s../..", argv[0]);
            chdir(newPath);
            free(newPath);
        }
        (*argc) -= 2;
        if(*argc > 1)
        {
            argv[1] = argv[3];
        }
        else
            argv[1] = NULL;
    }
    else
        argc = 0;
#else
    if(*argc >= 2 && argv[0] != NULL) //Si ouvert depuis un fichier, il faut restaurer le repertoire
    {
        char* path = malloc(length+1);
        if(path != NULL)
        {
            usstrcpy(path, length, argv[0]);
            length--;
            do
            {
                for(; length > 0 && path[length] != '/' && path[length] != '\\'; path[length--] = 0);
            }while(length > 0 && !checkFileExist(path));
            chdir(path);
            free(path);
        }
    }
#endif
    getcwd(REPERTOIREEXECUTION, sizeof(REPERTOIREEXECUTION));
}

void strend(char *recepter, size_t length, const char *sender)
{
    if(recepter && sender)
    {
        int i = 0;
        for(; *recepter; i++, recepter++);
        for(; *sender && length > i; i++, recepter++, sender++)
            *recepter = *sender;

        if(length < i)
            *recepter = 0;
        else
            *(recepter-1) = 0;
    }
}

char* mergeS(char* input1, char* input2)
{
    char *output = NULL;
    output = ralloc(strlen(input1) + strlen(input2)+10);
    if(output != NULL)
    {
        memcpy(output, input1, strlen(input1));
        memcpy(output+strlen(output), input2, strlen(input2));
        free(input1);
    }
    return output;
}

void *ralloc(size_t length)
{
    void* memory_allocated = calloc(1, length);
    if(memory_allocated == NULL)
        memoryError(length);
    return memory_allocated;
}

int charToInt(char *input)
{
    int output = 0;
    sscanf(input, "%d", &output);
    return output;
}

void fscanfs(FILE* stream, const char *format, ...)
{
    int i = 0, j = 0, format_read = 0;

    if(stream == NULL)
        return;

    va_list pointer_argument;
    va_start (pointer_argument, format);

    for(format_read = 0; format[format_read]; format_read++)
    {
        if(format[format_read] != '%' && format[format_read] != ' ') //Si on attend pas d'argument, on attend d'arriver au char demandé
        {
            if(format[format_read] != fgetc(stream))
            {
                fseek(stream, -1, SEEK_CUR);
                while((j = fgetc(stream)) != format[format_read] && j != EOF);
            }
            else
                fseek(stream, -1, SEEK_CUR);
        }

        else if(format[format_read] == '%') //Les choses interessantes commenencent
        {
            format_read++;
            switch(format[format_read])
            {
                case 's':
                {
                    int lenght = 0;
                    char *buffer = NULL;

                    buffer = va_arg (pointer_argument, char*);
                    lenght = va_arg (pointer_argument, int);

                    if(!buffer || !lenght)
                        break;

                    while(((j = fgetc(stream)) == ' ' ||  j == '\n') && j != EOF);
                    for(i = 0; i < lenght && j != ' ' && j != '\n' && j != '\r' && j != EOF; j = fgetc(stream))
                        buffer[i++] = j;

                    for(; j != ' ' && j != '\n' && j != EOF; j = fgetc(stream)); //On finis le mot si on a bloqué un buffer overflow

                    if(i == lenght)
                        i--;

                    buffer[i] = '\0';
                    buffer = NULL;
                    break;
                }

                case 'd':
                {
                    int *number = NULL, negatif = 0;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;
                    for(j = 0; (j < '0' ||  j > '9') && j != EOF;)
                    {
                        j = fgetc(stream);
                        if(j == '-')
                        {
                            if((j = fgetc(stream)) != EOF && j >= '0' && j <= '9')
                                negatif = 1;
                        }
                    }
                    buffer[0] = j;

                    for(i = 1; i < 9 /*limite de int*/ && (j = fgetc(stream)) >= '0' && j <= '9'; buffer[i++] = j);
                    for(; j != ' ' && j != '\n' && j != EOF; j = fgetc(stream)); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    if(negatif)
                        *number = *number *-1;
                    number = NULL;
                    break;
                }

                default:
                    format_read--;
                    break;
            }
        }
    }
    va_end (pointer_argument);
}

int sscanfs(char *char_input, const char *format, ...)
{
    int i = 0, j = 0, format_read = 0;
    char *pointer = char_input;

    va_list pointer_argument;
    va_start (pointer_argument, format);

    for(format_read = 0; format[format_read]; format_read++)
    {
        if(format[format_read] != '%' && format[format_read] != ' ' && format[format_read] != *char_input) //Si on attend pas d'argument, on attend d'arriver au char demandé
            while((j = *(++char_input)) != format[format_read] && j != 0);

        else if(format[format_read] == '%') //Les choses interessantes commenencent
        {
            format_read++;
            switch(format[format_read])
            {
                case 's':
                {
                    int lenght = 0;
                    char *buffer = NULL;

                    buffer = va_arg (pointer_argument, char*);
                    lenght = va_arg (pointer_argument, int);

                    if(!buffer || !lenght)
                        break;

                    for(; *char_input == ' ' || *char_input == '\r' || *char_input == '\n'; char_input++);
                    for(i = 0; i < lenght && *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++)
                        buffer[i++] = *char_input;
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    if(i >= lenght)
                        i--;

                    buffer[i] = '\0';
                    buffer = NULL;
                    break;
                }

                case 'd':
                {
                    int *number = NULL, negative = 0;
                    char buffer[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                    number = va_arg (pointer_argument, int*);

                    if(!number)
                        break;

                    while(*char_input && (*char_input < '0' || *char_input > '9'))
                    {
                        if(*char_input == '-')
                        {
                            char_input++;
                            if(*char_input >= '0' && *char_input <= '9')
                                negative = 1;
                        }
                        else
                            char_input++;
                    }
                    for(i = 0; i < 9 /*limite de int*/ && *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input && *char_input >= '0' && *char_input <= '9'; char_input++)
                        buffer[i++] = *char_input;
                    for(; *char_input != ' ' && *char_input != '\n' && *char_input != '\r' && *char_input; char_input++); //On finis le mot si on a bloqué un buffer overflow

                    buffer[i] = 0;
                    *number = charToInt(buffer);
                    if(negative)
                        *number *= -1;
                    number = NULL;
                    break;
                }

                default:
                    format_read--;
                    break;
            }
            if(format[format_read] && format[format_read+1] && format[format_read+1] != '\n')
                format_read++;
        }

        else
            char_input++;
    }
    va_end (pointer_argument);
    return char_input - pointer;
}

size_t ustrlen(void *input)
{
    unsigned char *copy = input;
    while(*(copy++));
    return copy - (unsigned char *)input;
}

void usstrcpy(void* output, size_t length, const void* input)
{
    char *output_char = output;
    strncpy(output, input, length);
    output_char[length-1] = 0;
}

void ustrcpy(void* output, const void* input)
{
    usstrcpy(output, ustrlen((void*)input), input);
}

void SDL_FreeSurfaceS(SDL_Surface *surface)
{
    if(surface != NULL && ((surface->w > 0 && surface->w <= 2*RESOLUTION[0]) || (surface->h > 0 && surface->h <= 2*RESOLUTION[1]))) //Si une des dimensions est normale
        SDL_FreeSurface(surface);

    else if(surface != NULL)
        logR("Invalid surface rejected\n");
}

void SDL_DestroyTextureS(SDL_Texture *texture)
{
    SDL_DestroyTexture(texture); //Gére déjà les cas imprévus
}

/**Différent en fonction de l'OS**/
int rmdir (const char *filename);
void removeFolder(char *path)
{
#ifdef DEV_VERSION
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Suppression detectee", path, window);
#endif
    DIR *directory;           /* pointeur de répertoire */
    struct dirent *entry;     /* représente une entrée dans un répertoire. */

    char *name = malloc(strlen(REPERTOIREEXECUTION) + strlen(path) + 100);
    char *buffer = NULL;

    snprintf(name, strlen(REPERTOIREEXECUTION) + strlen(path) + 100, "%s/%s", REPERTOIREEXECUTION, path);

    /* On ouvre le dossier. */
    directory = opendir(name);
    if ( directory == NULL )
    {
#ifdef DEV_VERSION
        char temp[300];
        snprintf(temp, 300, "Can't open directory %s\n", name);
        logR(temp);
#endif
        removeR(name);
        return;
    }

    /* On boucle sur les entrées du dossier. */
    while ( (entry = readdir(directory)) != NULL )
    {
        if(!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
            continue;

        buffer = malloc(strlen(path) + strlen(entry->d_name) + 0x10);
        if(buffer == NULL)
        {
            memoryError(strlen(path) + strlen(entry->d_name) + 0x10);
            continue;
        }
        snprintf(buffer, strlen(path) + strlen(entry->d_name) + 0x10, "%s/%s", path, entry->d_name);

        if(checkDirExist(buffer))
            removeFolder(buffer); // On est sur un dossier, on appelle cette fonction.
        else
            removeR(buffer); //On est sur un fichier, on le supprime.
        free(buffer);
    }
    closedir(directory);
    rmdir(name); //Maintenant le dossier doit être vide, on le supprime.
#ifdef DEV_VERSION
    char temp2[300];
    snprintf(temp2, 300, "Removed: %s\n", name);
    logR(temp2);
#endif
    free(name);
}

void ouvrirSite(char *URL)
{
    #ifdef _WIN32
        ShellExecute(NULL, "open", URL, NULL, NULL, SW_SHOWNOACTIVATE);
    #else
        char *bufferCMD;
        bufferCMD = malloc(strlen(URL) + 20);
        if(bufferCMD != NULL)
        {
            #ifdef __APPLE__
                snprintf(bufferCMD, strlen(URL) + 20, "open %s", URL);
            #else
                snprintf(bufferCMD, strlen(URL) + 20, "firefox %s", URL);
            #endif
            system(bufferCMD);
            free(bufferCMD);
        }
    #endif
}

int lancementExternalBinary(char cheminDAcces[100])
{
    char superTemp[400];
#ifdef _WIN32
    int i = 0, j = 0;
	char temp[250];

	crashTemp(temp, 250);
    for(i = 0; i < 250 && REPERTOIREEXECUTION[i] != 0; i++)
    {
        if(REPERTOIREEXECUTION[i] == '\\')
            temp[j++] = '\\';
        temp[j++] = REPERTOIREEXECUTION[i];
    }
    snprintf(superTemp, 400, "\"%s/%s\"", temp, cheminDAcces);
    ShellExecute(NULL, "open", superTemp, NULL, NULL, SW_SHOWDEFAULT);
 #else
    #ifdef __APPLE__
        snprintf(superTemp, 400, "open -n \"%s/%s\"", REPERTOIREEXECUTION, cheminDAcces);
    #else
        snprintf(superTemp, 400, "\"%s/%s\" &", REPERTOIREEXECUTION, cheminDAcces);
    #endif
    system(superTemp);
#endif
    return 0;
}

int checkPID(int PID)
{
#ifndef _WIN32
    
    char temp[TAILLE_BUFFER];
    FILE *test = NULL;

    if(!PID)
        return 1;
    #ifdef __APPLE__
    snprintf(temp, TAILLE_BUFFER, "ps %d", PID);
    test = popen(temp, "r");
    if(test != NULL)
    {
        while(fgetc(test) != '\n' && fgetc(test) != EOF);
        bool out = fgetc(test) == EOF;
        fclose(test);
        return out;
    }
    #else
	snprintf(temp, TAILLE_BUFFER, "proc/%d/cwd", PID);
    test = fopenR(temp, "r");
    if(test != NULL) //Si le PID existe
    {
		int i, j;
        crashTemp(temp, TAILLE_BUFFER);
        for(j = 0; (i = fgetc(test)) != EOF && j < TAILLE_BUFFER; j++)
            temp[j] = i;
        fclose(test);
        if(!strcmp(temp, REPERTOIREEXECUTION))
            return 0;
    }
    else //Sinon
        return 1;
    #endif
#endif
    return 0;
}

int checkDirExist(char *dirname)
{
    DIR *directory = NULL;
    if(dirname[1] == ':' || dirname[0] == '/')
        directory = opendir(dirname);
    else
    {
        char *directory_fullname = ralloc(strlen(dirname) + strlen(REPERTOIREEXECUTION) + 128);
        if(directory_fullname != NULL)
        {
            snprintf(directory_fullname, strlen(dirname) + strlen(REPERTOIREEXECUTION) + 128, "%s/%s", REPERTOIREEXECUTION, dirname);
            directory = opendir(directory_fullname);
            free(directory_fullname);
        }
    }
    if(directory != NULL)
    {
        closedir(directory);
        return 1;
    }
    return 0;
}

