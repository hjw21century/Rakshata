/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

char *COMPTE_PRINCIPAL_MAIL = NULL;
MUTEX_VAR DBRefreshMutex;

#ifdef _WIN32
    #ifdef main
        #undef main
    #endif
#else
	int launchApp(int argc, char *argv[]);
#endif

void cleanup()
{
	free(COMPTE_PRINCIPAL_MAIL);
	flushRecentMutex();
	flushDB();
	releaseDNSCache();
	MUTEX_DESTROY(DBRefreshMutex);
}

int main(int argc, char *argv[])
{
	configureSandbox();
	createCrashFile();
	registerExtensions();
	
	//Initialisation
	MUTEX_CREATE(DBRefreshMutex);
	
	loadEmailProfile();
	initializeDNSCache();
	
#ifdef VERBOSE_DB_MANAGEMENT
	removeFolder("log");
	mkdirR("log");
#endif
	
	atexit(cleanup);
	createNewThread(networkAndVersionTest, NULL); //On met le test dans un nouveau thread pour pas ralentir le démarrage
	setupBDDCache();
	return launchApp(argc, argv);
}
