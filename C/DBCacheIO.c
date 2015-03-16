/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **		Source code and assets are property of Taiki, distribution is stricly forbidden		**
 **                                                                                         **
 *********************************************************************************************/

#include "dbCache.h"

sqlite3_stmt * getAddToCacheRequest()
{
	sqlite3_stmt * request = NULL;
	
	sqlite3_prepare_v2(cache, "INSERT INTO rakSQLite("DBNAMETOID(RDB_repo)", "DBNAMETOID(RDB_projectID)", "DBNAMETOID(RDB_isInstalled)", "DBNAMETOID(RDB_projectName)", "DBNAMETOID(RDB_description)", "DBNAMETOID(RDB_authors)", "DBNAMETOID(RDB_status)", "DBNAMETOID(RDB_type)", "DBNAMETOID(RDB_asianOrder)", "DBNAMETOID(RDB_isPaid)", "DBNAMETOID(RDB_tag)", "DBNAMETOID(RDB_nombreChapitre)", "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_nombreTomes)", "DBNAMETOID(RDB_DRM)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_favoris)") values(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9, ?10, ?11, ?12, ?13, ?14, ?15, ?16, ?17, ?18);", -1, &request, NULL);
	
	return request;
}

bool addToCache(sqlite3_stmt* request, PROJECT_DATA data, uint64_t repoID, bool isInstalled)
{
	if(!data.isInitialized)
		return false;
	
	sqlite3_stmt * internalRequest = NULL;
	
	if(request != NULL)
		internalRequest = request;
	else
		internalRequest = getAddToCacheRequest();
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.projectName), lengthD = wstrlen(data.description), lengthA = wstrlen(data.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	bool output;
	
	sqlite3_bind_int64(internalRequest, 1, repoID);
	sqlite3_bind_int(internalRequest, 2, data.projectID);
	sqlite3_bind_int(internalRequest, 3, isInstalled);
	sqlite3_bind_text(internalRequest, 4, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 5, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(internalRequest, 6, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(internalRequest, 7, data.status);
	sqlite3_bind_int(internalRequest, 8, data.type);
	sqlite3_bind_int(internalRequest, 9, data.japaneseOrder);
	sqlite3_bind_int(internalRequest, 10, data.isPaid);
	sqlite3_bind_int(internalRequest, 11, data.tag);
	sqlite3_bind_int(internalRequest, 12, data.nombreChapitre);
	sqlite3_bind_int64(internalRequest, 13, (int64_t) data.chapitresFull);
	sqlite3_bind_int64(internalRequest, 14, (int64_t) data.chapitresPrix);
	sqlite3_bind_int(internalRequest, 15, data.nombreTomes);
	sqlite3_bind_int(internalRequest, 16, data.haveDRM);
	sqlite3_bind_int64(internalRequest, 17, (int64_t) data.tomesFull);
	sqlite3_bind_int(internalRequest, 18, data.favoris);
	
	output = sqlite3_step(internalRequest) == SQLITE_DONE;
	
	sqlite3_reset(internalRequest);
	nbElemInCache++;
	
	return output;
}

bool updateCache(PROJECT_DATA data, char whatCanIUse, uint projectID)
{
	uint DBID;
	void * buffer;
	sqlite3_stmt *request = NULL;
	
	if(cache == NULL && !setupBDDCache() && !data.isInitialized)	//Échec du chargement
		return false;
	
	//On libère la mémoire des éléments remplacés
	if(whatCanIUse == RDB_UPDATE_ID)
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
		sqlite3_bind_int(request, 1, data.cacheDBID);
		DBID = data.cacheDBID;
	}
	else
	{
		sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_repo)" = ?1 AND "DBNAMETOID(RDB_projectID)" = ?2", -1, &request, NULL);
		sqlite3_bind_int64(request, 1, getRepoID(data.repo));
		sqlite3_bind_int(request, 2, projectID);
	}
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);
		
		if(whatCanIUse != RDB_UPDATE_ID)
			DBID = sqlite3_column_int(request, 2);
	}
	else
	{
		sqlite3_finalize(request);
		return false;
	}
	
	sqlite3_finalize(request);
	
	//We convert wchar_t to utf8
	size_t lengthP = wstrlen(data.projectName), lengthD = wstrlen(data.description), lengthA = wstrlen(data.authorName);
	char utf8Project[4 * lengthP + 1], utf8Descriptions[4 * lengthD + 1], utf8Author[4 * lengthA + 1];
	
	lengthP = wchar_to_utf8(data.projectName, lengthP, utf8Project, sizeof(utf8Project), 0);			utf8Project[lengthP] = 0;
	lengthD = wchar_to_utf8(data.description, lengthD, utf8Descriptions, sizeof(utf8Descriptions), 0);	utf8Descriptions[lengthD] = 0;
	lengthA = wchar_to_utf8(data.authorName, lengthA, utf8Author, sizeof(utf8Author), 0);				utf8Author[lengthA] = 0;
	
	//On pratique le remplacement effectif
	sqlite3_prepare_v2(cache, "UPDATE rakSQLite SET "DBNAMETOID(RDB_projectName)" = ?1, "DBNAMETOID(RDB_description)" = ?2, "DBNAMETOID(RDB_authors)" = ?3, "DBNAMETOID(RDB_status)" = ?4, "DBNAMETOID(RDB_type)" = ?5, "DBNAMETOID(RDB_asianOrder)" = ?6, "DBNAMETOID(RDB_isPaid)" = ?7, "DBNAMETOID(RDB_tag)" = ?8, "DBNAMETOID(RDB_nombreChapitre)" = ?9, "DBNAMETOID(RDB_chapitres)" = ?10, "DBNAMETOID(RDB_chapitresPrice)" = ?11, "DBNAMETOID(RDB_nombreTomes)" = ?12, "DBNAMETOID(RDB_DRM)" = ?13, "DBNAMETOID(RDB_tomes)" = ?14, "DBNAMETOID(RDB_favoris)" = ?15 WHERE "DBNAMETOID(RDB_ID)" = ?16", -1, &request, NULL);
	
	sqlite3_bind_text(request, 1, utf8Project, lengthP, SQLITE_STATIC);
	sqlite3_bind_text(request, 2, utf8Descriptions, lengthD, SQLITE_STATIC);
	sqlite3_bind_text(request, 3, utf8Author, lengthA, SQLITE_STATIC);
	sqlite3_bind_int(request, 4, data.status);
	sqlite3_bind_int(request, 5, data.type);
	sqlite3_bind_int(request, 6, data.japaneseOrder);
	sqlite3_bind_int(request, 7, data.isPaid);
	sqlite3_bind_int(request, 8, data.tag);
	sqlite3_bind_int(request, 9, data.nombreChapitre);
	
	if(data.chapitresFull != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresFull, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 10, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 10, 0x0);
	
	if(data.chapitresPrix != NULL)
	{
		buffer = malloc((data.nombreChapitre + 1) * sizeof(int));
		if(buffer != NULL)
			memcpy(buffer, data.chapitresPrix, (data.nombreChapitre + 1) * sizeof(int));
		
		sqlite3_bind_int64(request, 11, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 11, 0x0);
	
	
	sqlite3_bind_int(request, 12, data.nombreTomes);
	sqlite3_bind_int(request, 13, data.haveDRM);
	
	if(data.tomesFull != NULL)
	{
		buffer = malloc((data.nombreTomes + 1) * sizeof(META_TOME));
		if(buffer != NULL)
			copyTomeList(data.tomesFull, data.nombreTomes, buffer);
		
		sqlite3_bind_int64(request, 14, (int64_t) buffer);
	}
	else
		sqlite3_bind_int64(request, 15, 0x0);
	
	sqlite3_bind_int(request, 15, data.favoris);
	
	sqlite3_bind_int(request, 16, DBID);	//WHERE
	
	if(sqlite3_step(request) != SQLITE_DONE || sqlite3_changes(cache) == 0)
		return false;
	
	sqlite3_finalize(request);
	
	return true;
}

void removeFromCache(PROJECT_DATA data)
{
	if(cache == NULL)
		return;
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)" FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	sqlite3_bind_int(request, 1, data.cacheDBID);
	
	if(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);
	}
	sqlite3_finalize(request);
	
	sqlite3_prepare_v2(cache, "DELETE FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &request, NULL);
	sqlite3_bind_int(request, 1, data.cacheDBID);
	sqlite3_step(request);
	sqlite3_finalize(request);
	
	nbElemInCache--;
}

void consolidateCache()
{
	sqlite3_stmt* request = NULL;
	sqlite3_prepare_v2(cache, "VACUUM", -1, &request, NULL);
	sqlite3_step(request);
	sqlite3_finalize(request);
}

#pragma mark - Repo

void addRootRepoToDB(ROOT_REPO_DATA * newRepo)
{
	insertRootRepoCache(&newRepo, 1);
	
	//We update the project cache with the new data
	PROJECT_DATA empty = getEmptyProject();
	ICONS_UPDATE * iconData = NULL, * endIcon, * newIcon;
	for(uint i = 0; i < newRepo->nombreSubrepo; i++)
	{
		if(newRepo->subRepo[i].active)
		{
			empty.repo = &newRepo->subRepo[i];
			
			newIcon = updateProjectsFromRepo(&empty, 0, 0, false);
			
			if(newIcon != NULL)
			{
				if(iconData == NULL)
					iconData = endIcon = newIcon;
				else
					endIcon->next = newIcon;
				
				while(endIcon->next != NULL)
					endIcon = endIcon->next;
			}
		}
	}
	
	createNewThread(updateProjectImages, iconData);
	syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
	notifyFullUpdate();
}

void removeRepoFromCache(REPO_DATA repo)
{
	if(cache == NULL)
		return;
	
	MUTEX_LOCK(cacheMutex);
	
	uint64_t repoID = getRepoID(&repo);
	
	//On libère la mémoire des éléments remplacés
	sqlite3_stmt* request = NULL, *deleteRequest = NULL;
	sqlite3_prepare_v2(cache, "SELECT "DBNAMETOID(RDB_chapitres)", "DBNAMETOID(RDB_chapitresPrice)", "DBNAMETOID(RDB_tomes)", "DBNAMETOID(RDB_ID)" FROM rakSQLite WHERE "DBNAMETOID(RDB_repo)" = ?1", -1, &request, NULL);
	sqlite3_prepare_v2(cache, "DELETE FROM rakSQLite WHERE "DBNAMETOID(RDB_ID)" = ?1", -1, &deleteRequest, NULL);
	sqlite3_bind_int64(request, 1, repoID);
	
	while(sqlite3_step(request) == SQLITE_ROW)
	{
		free((void*) sqlite3_column_int64(request, 0));
		free((void*) sqlite3_column_int64(request, 1));
		freeTomeList((void*) sqlite3_column_int64(request, 2), true);
		
		sqlite3_bind_int64(deleteRequest, 1, sqlite3_column_int64(request, 3));
		sqlite3_step(deleteRequest);
		sqlite3_reset(deleteRequest);
		
		nbElemInCache--;
	}
	sqlite3_finalize(deleteRequest);
	sqlite3_finalize(request);
	
	MUTEX_UNLOCK(cacheMutex);
	
	syncCacheToDisk(SYNC_REPO);
	notifyFullUpdate();
}

void activateRepo(REPO_DATA repo)
{
	MUTEX_LOCK(cacheMutex);
	
	bool alreadyActive = false;
	
	for(uint i = 0; i < lengthRootRepo; i++)
	{
		if(rootRepoList[i]->repoID == repo.parentRepoID)
		{
			for(uint j = 0, length = rootRepoList[i]->nombreSubrepo; j < length; j++)
			{
				if(rootRepoList[i]->subRepo[j].repoID == repo.repoID)
				{
					if(!rootRepoList[i]->subRepo[j].active)
						rootRepoList[i]->subRepo[j].active = true;
					else
						alreadyActive = true;
					
					break;
				}
			}
			
			break;
		}
	}
	
	if(!alreadyActive)
	{
		REPO_DATA ** newRepoList = realloc(repoList, (lengthRepo + 2) * sizeof(REPO_DATA *));
		if(newRepoList != NULL)
		{
			newRepoList[lengthRepo] = malloc(sizeof(REPO_DATA));
			if(newRepoList[lengthRepo] != NULL)
			{
				*newRepoList[lengthRepo] = repo;
				newRepoList[++lengthRepo] = NULL;
			}
			
			repoList = newRepoList;
		}
	}
	
	MUTEX_UNLOCK(cacheMutex);
	
	if(!alreadyActive)
	{
		//We now, we need to update the new repo to reflect changes
		ICONS_UPDATE * iconData = refreshRepo(&repo, false);

		createNewThread(updateProjectImages, iconData);
		syncCacheToDisk(SYNC_REPO | SYNC_PROJECTS);
		notifyFullUpdate();
	}
}

void deleteSubRepo(uint64_t repoID)
{
	if(lengthRepo == 0)
		return;
	
	MUTEX_LOCK(cacheMutex);
	
	//We first delete from the linearized cache
	bool foundSomething = false;
	uint length = lengthRepo;
	REPO_DATA ** newRepo = calloc(length, sizeof(REPO_DATA *));
	
	if(newRepo != NULL)
	{
		for(uint pos = 0, index = 0; pos < length; pos++)
		{
			if(repoList[pos] != NULL && getRepoID(repoList[pos]) != repoID)
				newRepo[index++] = repoList[pos];
			else if(repoList[pos] != NULL)
			{
				foundSomething = true;
				free(repoList[pos]);
			}
		}
	}
	
	if(foundSomething)
	{
		void * old = repoList;
		
		repoList = newRepo;
		lengthRepo--;
		
		free(old);
	}
	else
		free(newRepo);
	
	//Then we look for the root
	for(uint pos = 0, rootID = getRootFromRepoID(repoID); pos < lengthRootRepo; pos++)
	{
		if(rootRepoList[pos] != NULL && rootRepoList[pos]->repoID == rootID)
		{
			ROOT_REPO_DATA * rootRepo = rootRepoList[pos];
			length = rootRepo->nombreSubrepo;
			
			for(uint subPos = 0, subID = getSubrepoFromRepoID(repoID); subPos < length; subPos++)
			{
				if(rootRepo->subRepo[subPos].repoID == subID)
				{
					rootRepo->subRepo[subPos].active = false;
					break;
				}
			}
			break;
		}
	}
	
	MUTEX_UNLOCK(cacheMutex);
	syncCacheToDisk(SYNC_REPO);
}
