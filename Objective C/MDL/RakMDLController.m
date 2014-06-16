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

@implementation RakMDLController

- (id) init : (MDL *) tabMDL : (NSString *) state
{
	self = [super init];
	
	if(self != nil)
	{
		_tabMDL = tabMDL;
		IDToPosition = NULL;
		quit = false;
		
		cache = getCopyCache(RDB_LOADALL | SORT_NAME | RDB_CTXMDL, &sizeCache);

		char * stateChar = NULL;
		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
			stateChar = (char *) [state UTF8String];
		
		if(startMDL(stateChar, cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self))
		{
			IDToPosition = malloc(nbElem * sizeof(uint));
			if(IDToPosition != NULL)
			{
				for(discardedCount = 0; discardedCount < nbElem; discardedCount++)
					IDToPosition[discardedCount] = discardedCount;
			}
			else
			{
				[self release];			self = nil;
			}
		}
		else
		{
			[self release];			self = nil;
		}
	}
	
	return self;
}

- (void) needToQuit
{
	quit = true;
	if(isThreadStillRunning(coreWorker))
		MDLQuit();
}

#warning "Don't save paused elements. Also, should use IDToPosition"
- (NSString *) serializeData
{
	for(int pos = 0; pos < nbElem; pos++)
    {
		if (*status[pos] <= MDL_CODE_DEFAULT)	//Si le moindre élément n'est pas dans la pipeline, on serialise
		{
			char * data = MDLParseFile(*todoList, status, nbElem);
			if(data == NULL)
				return nil;
			NSString * output = [NSString stringWithUTF8String: data];
			free(data);
			return output;
		}
    }
	
	return nil;
}
		   
- (void) dealloc
{
	MDLCleanup(nbElem, status, statusCache, todoList, cache);
	free(IDToPosition);
	[super dealloc];
}

- (uint) getNbElem : (BOOL) considerDiscarded
{
	return considerDiscarded ? discardedCount : nbElem;
}

- (uint) convertRowToPos : (uint) row
{
	return (row >= discardedCount) ? -1 : IDToPosition[row];
}

- (DATA_LOADED **) getData : (uint) row : (BOOL) considerDiscarded
{
	if(row >= (considerDiscarded ? discardedCount : nbElem))
		return NULL;
	
	return &(*todoList)[considerDiscarded ? IDToPosition[row] : row];
}

- (int8_t) statusOfID : (uint) row : (BOOL) considerDiscarded
{
	if(row >= (considerDiscarded ? discardedCount : nbElem) || status == NULL)
		return MDL_CODE_INTERNAL_ERROR;
	
	if(considerDiscarded)
		row = IDToPosition[row];
	
	return *(status[row]);
}

- (void) addElement : (MANGAS_DATA) data : (BOOL) isTome : (int) element : (BOOL) partOfBatch
{
	if (element == VALEUR_FIN_STRUCTURE_CHAPITRE)
		return;
	
	uint pos;
	for (pos = 0; pos < sizeCache && cache[pos].cacheDBID != data.cacheDBID ; pos++);
	
	if(pos == sizeCache || cache[pos].cacheDBID != data.cacheDBID)
	{
		//We need to refresh
	}
	
	if(!nbElem || !MDLisThereCollision(data, isTome, element, *todoList, status, nbElem))
	{
		int newChunkSize;
		DATA_LOADED ** newElement = MDLCreateElement(&cache[pos], isTome, element, &newChunkSize);
		
		if(newElement == NULL || newChunkSize == 0)
		{
			free(newElement);
			return;
		}
		
		uint newSize = nbElem + newChunkSize;
		int8_t **newStatus = realloc(status, newSize * sizeof(int8_t*)), **newStatusCache = realloc(statusCache, newSize * sizeof(int8_t*));
		uint *newIDToPosition = realloc(IDToPosition, (discardedCount + newChunkSize) * sizeof(uint));
		
		//Even if one of them failed, we need to update the pointer of the other
		if(newStatus == NULL || newStatusCache == NULL || newIDToPosition == NULL)
		{
			if(newStatus != NULL)
				status = newStatus;
			
			if(newStatusCache != NULL)
				statusCache = newStatusCache;
			
			if(newIDToPosition != NULL)
				IDToPosition = newIDToPosition;
			
			return;
		}
		
		status = newStatus;
		statusCache = newStatusCache;
		IDToPosition = newIDToPosition;
		
		//Increase the size of the status buffer
		for (uint maxSize = nbElem + newChunkSize; nbElem < maxSize; nbElem++)
		{
			status[nbElem] = malloc(sizeof(int8_t));
			statusCache[nbElem] = malloc(sizeof(int8_t));
			
			if(status[nbElem] == NULL || statusCache[nbElem] == NULL)
			{
				uint i = nbElem - (maxSize - newChunkSize) + 1;
				nbElem = maxSize = newChunkSize;
				while (i > 0)
				{
					free(status[nbElem + --i]);
					free(statusCache[nbElem + i]);
				}
				
				return;
			}
			
			*status[nbElem] = *statusCache[nbElem] = MDL_CODE_DEFAULT;
			IDToPosition[discardedCount++] = nbElem;
		}
		
		int curPos = nbElem - 1;
		DATA_LOADED ** newTodoList = realloc(*todoList, nbElem * sizeof(DATA_LOADED *));
		
		if(newTodoList == NULL)
		{
			for (uint limit = nbElem - newChunkSize; limit < nbElem;)
			{
				free(status[--nbElem]);
				free(statusCache[nbElem]);
			}
		}
		
		*todoList = MDLInjectElementIntoMainList(newTodoList, &nbElem, &curPos, newElement, newChunkSize);
	}
	
	if(!partOfBatch && discardedCount)
	{
		//Great, the injection is now over... We need to reanimate what needs to be
		if(!isThreadStillRunning(coreWorker))
		{
			startMDL(NULL, cache, &coreWorker, &todoList, &status, &statusCache, &nbElem, &quit, self);
		}
		else
		{
			uint i;
			for(i = 0; i < discardedCount; i++)
			{
				//Anything running?
				if((*status[IDToPosition[i]]) > MDL_CODE_DEFAULT && (*status[IDToPosition[i]]) < MDL_CODE_INSTALL_OVER)
					break;
			}
			
			if(i == discardedCount)
				MDLDownloadOver();
		}
		
		//Worker should be at work, now, let's wake the UI up
		[_tabMDL wakeUp];
	}
}

- (uint) addBatch : (MANGAS_DATA) data : (BOOL) isTome : (BOOL) launchAtTheEnd
{
	//We assume our data are up-to-date
	int previousElem = VALEUR_FIN_STRUCTURE_CHAPITRE;
	uint posFull = 0, posInst = 0, nbFull, nbInst, countInjected = 0;
	
	if (isTome)
	{
		if(data.tomesFull == NULL || data.tomesInstalled == NULL || data.tomesFull[0].ID == VALEUR_FIN_STRUCTURE_CHAPITRE)
			return countInjected;
		
		nbFull = data.nombreTomes;
		nbInst = data.nombreTomesInstalled;
	}
	else
	{
		if(data.chapitresFull == NULL || data.chapitresInstalled == NULL || data.chapitresFull[0] == VALEUR_FIN_STRUCTURE_CHAPITRE)
			return countInjected;

		nbFull = data.nombreChapitre;
		nbInst = data.nombreChapitreInstalled;
	}
	
	//On choppe les trous
	for (; posFull < nbFull && MDLCTRL_getDataFull(data, posFull, isTome) != VALEUR_FIN_STRUCTURE_CHAPITRE && posInst < nbInst && MDLCTRL_getDataInstalled(data, posInst, isTome) != VALEUR_FIN_STRUCTURE_CHAPITRE; posFull++)
	{
		if (MDLCTRL_getDataFull(data, posFull, isTome) == MDLCTRL_getDataInstalled(data, posInst, isTome))
			posInst++;
		else
		{
			if(previousElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
			{
				[self addElement:data :isTome :previousElem :YES];
				countInjected++;
			}
			
			previousElem = MDLCTRL_getDataFull(data, posFull, isTome);
		}
	}
	
	//Le burst de fin
	while (posFull < nbFull && MDLCTRL_getDataFull(data, posFull, isTome) != VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		if(previousElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
		{
			[self addElement:data :isTome :previousElem :YES];
			countInjected++;
		}
		
		previousElem = MDLCTRL_getDataFull(data, posFull++, isTome);
	}
	
	if(previousElem != VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		[self addElement:data :isTome :previousElem :!launchAtTheEnd];
		countInjected++;
	}
	
	return countInjected;
}

- (void) reorderElements : (uint) posStart : (uint) posEnd : (uint) injectionPoint
{
	if(IDToPosition == NULL)
		return;
	
	if (posStart > posEnd || posEnd >= discardedCount || (injectionPoint >= posStart && injectionPoint <= posEnd))
	{
		NSLog(@"Invalid data");
		return;
	}
	
	if(injectionPoint == discardedCount)
		injectionPoint--;

	uint size = (posEnd - posStart + 1);
	bool isMovingPartBeforeInsertion = posEnd < injectionPoint;
	uint * movingPart = malloc(size);
	
	if (movingPart == NULL)
	{
		memoryError(size * sizeof(uint));
		return;
	}
	else
		memcpy(movingPart, &IDToPosition[posStart], size * sizeof(uint));
	
	if(isMovingPartBeforeInsertion)
		memcpy(&IDToPosition[posStart], &IDToPosition[posStart + size], (--injectionPoint - posStart) * sizeof(uint));
	else
		memcpy(&IDToPosition[injectionPoint + size], &IDToPosition[injectionPoint], (posStart - injectionPoint) * sizeof(uint));
	
	memcpy(&IDToPosition[injectionPoint], movingPart, size * sizeof(uint));
	free(movingPart);
}

- (BOOL) checkForCollision : (MANGAS_DATA) data : (BOOL) isTome : (int) element
{
	return nbElem && MDLisThereCollision(data, isTome, element, *todoList, status, nbElem);
}

- (void) discardElement : (uint) element
{
	discardedCount--;	//If decrement sent in the if, won't work for last element

	if(element <= discardedCount)
	{
		for (uint posDiscarded = element; posDiscarded < discardedCount; posDiscarded++)
		{
			IDToPosition[posDiscarded] = IDToPosition[posDiscarded+1];
		}
	}
}

- (void) setStatusOfID : (uint) row : (BOOL) considerDiscarded : (int8_t) value
{
	if(row >= (considerDiscarded ? discardedCount : nbElem) || status == NULL)
		return;
	
	if(considerDiscarded)
		row = IDToPosition[row];
	
	if(status[row] != NULL)
		*(status[row]) = value;
}

- (void) refreshCT : (uint) row
{
	DATA_LOADED ** data = [self getData:row :YES];
	
	if(data != NULL && *data != NULL)
	{
		for(NSView* view in _tabMDL.superview.subviews)
		{
			if([view class] == [CTSelec class])
			{
				[(CTSelec *) view refreshCT:YES :(*data)->datas->cacheDBID];
				break;
			}
		}
	}
}

@end
