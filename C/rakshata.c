/*********************************************************************************************
**	__________         __           .__            __                 ________   _______   	**
**	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
**	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
**	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
**	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
**	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
**                                                                                          **
**		Source code and assets are property of Taiki, distribution is stricly forbidden		**
**                                                                                          **
*********************************************************************************************/

#include "db.h"

volatile int NETWORK_ACCESS = CONNEXION_OK;
char *COMPTE_PRINCIPAL_MAIL = NULL;
MUTEX_VAR networkAndDBRefreshMutex;

#ifdef _WIN32
    #ifdef main
        #undef main
    #endif
#else
	int NSApplicationMain(int argc, const char *argv[]);
#endif

int main(int argc, char *argv[])
{
	if(!earlyInit(argc, argv)) //On regroupe tout dans une fonction pour vider main
        return -1; //Si echec

	setupBDDCache();

    int ret_value = NSApplicationMain(argc, (const char **)argv);

	free(COMPTE_PRINCIPAL_MAIL);
	flushDB();
    releaseDNSCache();
    MUTEX_DESTROY(networkAndDBRefreshMutex);

    return ret_value;
}

