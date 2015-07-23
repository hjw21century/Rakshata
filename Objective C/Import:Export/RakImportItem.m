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

@implementation RakImportItem

- (BOOL) isReadable
{
	if(!_isTome)
		return checkReadable(_projectData.data.project, false, _contentID);

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.tomesFull;
	uint lengthTomeFull = _projectData.data.project.nombreTomes;

	_projectData.data.project.tomesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nombreTomes = _projectData.data.nombreTomeLocal;

	bool output = checkReadable(_projectData.data.project, true, _contentID);

	_projectData.data.project.tomesFull = tomeFull;
	_projectData.data.project.nombreTomes = lengthTomeFull;

	return output;
}

- (BOOL) needMoreData
{
	if(_projectData.data.project.isInitialized)
		return false;

	PROJECT_DATA project = _projectData.data.project;

	if(project.authorName[0] == 0)
		return true;

	if(project.status == STATUS_INVALID)
		return true;

#ifdef DEV_VERSION
	return true;
#else
	return false;
#endif
}

- (BOOL) install : (unzFile *) archive withUI : (RakImportStatusController *) UI
{
	char * projectPath = getPathForProject(_projectData.data.project);
	if(projectPath == NULL)
		return false;

	char basePath[sizeof(projectPath) + 256];
	int selection = _contentID;

	//Local project
	if(_isTome)
	{
		snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/", projectPath);
	}
	else
	{
		if(selection % 10)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%d.%d", projectPath, selection / 10, selection % 10);
		else
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"CHAPTER_PREFIX"%d", projectPath, selection / 10);
	}

	//Main decompression cycle

	unzGoToFirstFile(archive);

	//We grab the number of file
	unz_global_info64 globalMeta;
	if(unzGetGlobalInfo64(archive, &globalMeta) != UNZ_OK)
	{
		free(projectPath);
		return false;
	}

	const char * startExpectedPath = [_path UTF8String];
	uint lengthExpected = strlen(startExpectedPath);
	bool foundOneThing = false;

	if(UI != nil)
		UI.nbElementInEntry = globalMeta.number_entry;

	for (uint pos = 0; pos < globalMeta.number_entry && (UI == nil || ![UI haveCanceled]); pos++)
	{
		if(UI != nil)
			UI.posInEntry = pos;
		
		//Get current item filename
		char filename[1024] = {0};
		if((unzGetCurrentFileInfo64(archive, NULL, filename, sizeof(filename), NULL, 0, NULL, 0)) == UNZ_OK)
		{
			//If file is within the expected path (but not the dir itself)
			if(!strncmp(filename, startExpectedPath, lengthExpected) && filename[lengthExpected] != '\0')
			{
				extractCurrentfile(archive, NULL, basePath, STRIP_PATH_FIRST, NULL);
				foundOneThing = true;
			}
		}

		unzGoToNextFile(archive);
	}

	//Decompression is over, now, we need to ensure everything is fine
	if((UI != nil && [UI haveCanceled]) || !foundOneThing || ![self isReadable])
	{
		//Oh, the entry was not valid 😱
		if(UI != nil && ![UI haveCanceled])
			logR("Uh? Invalid import :|");

		if(_isTome)
			snprintf(basePath, sizeof(basePath), PROJECT_ROOT"%s/"VOLUME_PREFIX"%d", projectPath, _contentID);

		removeFolder(basePath);
		free(projectPath);
		return false;
	}

	free(projectPath);
	return true;
}

- (BOOL) overrideDuplicate : (unzFile *) archive
{
	if(_issue != IMPORT_PROBLEM_DUPLICATE)
		return NO;

	[self deleteData];
	if(![self install:archive withUI:nil])
	{
		_issue = IMPORT_PROBLEM_INSTALL_ERROR;
		return NO;
	}

	[self processThumbs:archive];
	[self registerProject];
	_issue = IMPORT_PROBLEM_NONE;

	return YES;
}

- (void) deleteData
{
	if(!_isTome)
		return	internalDeleteCT(_projectData.data.project, false, _contentID);

	//The volume must be in the list
	META_TOME * tomeFull = _projectData.data.project.tomesFull;
	uint lengthTomeFull = _projectData.data.project.nombreTomes;

	_projectData.data.project.tomesInstalled = _projectData.data.project.tomesFull = _projectData.data.tomeLocal;
	_projectData.data.project.nombreTomesInstalled = _projectData.data.project.nombreTomes = _projectData.data.nombreTomeLocal;

	internalDeleteTome(_projectData.data.project, _contentID, true);

	_projectData.data.project.tomesFull = tomeFull;
	_projectData.data.project.nombreTomes = lengthTomeFull;
}

- (void) processThumbs : (unzFile *) archive
{
	const char * imagesSuffix[4] = {PROJ_IMG_SUFFIX_SRGRID, PROJ_IMG_SUFFIX_HEAD, PROJ_IMG_SUFFIX_CT, PROJ_IMG_SUFFIX_DD};

	char * encodedHash = getPathForRepo(_projectData.data.project.repo);
	if(encodedHash == NULL)
		return;

	char imagePath[1024];
	size_t length = MIN((uint) snprintf(imagePath, sizeof(imagePath), IMAGE_CACHE_DIR"/%s/%s%d_", encodedHash, _projectData.data.project.locale ? LOCAL_PATH_NAME"_" : "", _projectData.data.project.projectID), sizeof(imagePath));
	free(encodedHash);

	for(byte pos = 0; pos < NB_IMAGES; pos++)
	{
		if(!_projectData.haveImages[pos])
			continue;

		snprintf(&imagePath[length], sizeof(imagePath) - length, "%s%s.png", imagesSuffix[pos / 2], pos % 2 ? "@2x" : "");

		if(checkFileExist(imagePath))
			continue;

		if(unzLocateFile(archive, _projectData.URLImages[pos], true) != UNZ_OK)
			continue;

		extractCurrentfile(archive, NULL, imagePath, STRIP_TRUST_PATH_AS_FILENAME, NULL);
	}
}

- (NSData *) queryThumbIn : (unzFile *) archive withIndex : (uint) index
{
	if(index >= NB_IMAGES || !_projectData.haveImages[index])
		return nil;

	if(unzLocateFile(archive, _projectData.URLImages[index], true) != UNZ_OK)
		return nil;

	byte * data = NULL;
	uint64_t thumbSize;

	if(!extractToMem(archive, &data, &thumbSize))
		return nil;

	return [NSData dataWithBytesNoCopy:data length:thumbSize freeWhenDone:YES];
}

- (void) registerProject
{
	_projectData.data.project.isInitialized = true;
	registerImportEntry(_projectData.data, _isTome);
}

@end