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

#define THREESOLD_IMAGES_FOR_VOL 80

id <RakImportIO> createIOForFilename(NSString * filename)
{
	NSString * extension = [filename pathExtension];

	//Import a .rak
	if([extension caseInsensitiveCompare:ARCHIVE_FILE_EXT] == NSOrderedSame)
		return [[RakImportDotRakController alloc] initWithFilename:filename];

	//Import a directory
	if([extension isEqualToString:@""] && checkDirExist([filename UTF8String]))
		return [[RakImportDirController alloc] initWithFilename:filename];

	//Look for Rar
	BOOL foundIt = NO;
	for(NSString * string in EXTERNAL_FILE_EXT_RAR)
	{
		if([extension caseInsensitiveCompare:string] == NSOrderedSame)
		{
			foundIt = YES;
			break;
		}
	}

	if(foundIt)
		return [[RakImportRarController alloc] initWithFilename:filename];

	//Look for zip
	for(NSString * string in EXTERNAL_FILE_EXT_ZIP)
	{
		if([extension caseInsensitiveCompare:string] == NSOrderedSame)
		{
			foundIt = YES;
			break;
		}
	}

	if(foundIt)
		return [[RakImportZipController alloc] initWithFilename:filename];

	return nil;
}

NSArray <RakImportItem *> * getManifestForIOs(NSArray <id <RakImportIO>> * _IOControllers)
{
	NSMutableArray <RakImportItem *> * output = [NSMutableArray array];

	//We iterate input files, we perform independant analysis on each of them
	//We don't try to merge different input files (eg, all flat CT) because it would be an issues
	//	if the reader was importing the daily releases of various projects in one take
	for(id <RakImportIO> IOController in _IOControllers)
	{
		//Cool, simplified analysis available
		if([IOController respondsToSelector:@selector(getManifest)])
		{
			[output addObjectsFromArray:[(id) IOController getManifest]];
			continue;
		}

		//We get the node
		RakImportNode * node = [IOController getNode];
		if(!node.isValid)
		{
#ifdef DEV_VERSION
			logR("Invalid node");
#endif
			continue;
		}

		//Easy case, no analysis needed
		if(node.isFlatCT)
		{
			RakImportItem * item = [RakImportItem new];
			if(item == nil)
				continue;

			item.issue = IMPORT_PROBLEM_METADATA;
			item.path = node.nodeName;
			item.projectData = getEmptyExtraProject();
			item.IOController = IOController;

			item.contentID = INVALID_VALUE;
			item.isTome = node.nbImages > THREESOLD_IMAGES_FOR_VOL;
			NSString * inferedName = [item inferMetadataFromPathWithHint:YES];

			PROJECT_DATA_EXTRA extraProject = item.projectData;

			if(inferedName == nil)
				inferedName = [item.path lastPathComponent];

			//We try to find if we can easily match a project
			extraProject.data.project.cacheDBID = getProjectByName([inferedName UTF8String]);

			//No luck ¯\_(ツ)_/¯
			if(extraProject.data.project.cacheDBID == INVALID_VALUE)
			{
				wstrncpy(extraProject.data.project.projectName, LENGTH_PROJECT_NAME, getStringFromUTF8((const byte *) [inferedName UTF8String]));
				extraProject.data.project.locale = true;
				extraProject.data.project.projectID = getEmptyLocalSlot(extraProject.data.project);
			}
			else
			{
				extraProject.data.project = getProjectByID(extraProject.data.project.cacheDBID);
			}

			if(item.isTome)
			{
				extraProject.data.tomeLocal = calloc(1, sizeof(META_TOME));
				if(extraProject.data.tomeLocal != NULL)
				{
					CONTENT_TOME * content = malloc(sizeof(CONTENT_TOME));
					if(content != NULL)
					{
						content->ID = CHAPTER_FOR_IMPORTED_VOLUMES;
						content->isPrivate = true;
						
						extraProject.data.tomeLocal[0].details = content;
						extraProject.data.tomeLocal[0].lengthDetails = 1;
						extraProject.data.tomeLocal[0].readingID = item.contentID == INVALID_VALUE ? INVALID_SIGNED_VALUE : (int) item.contentID;
						item.contentID = extraProject.data.tomeLocal[0].ID = getVolumeIDForImport(extraProject.data.project);
						extraProject.data.nombreTomeLocal++;
					}
					else
					{
						memoryError(sizeof(CONTENT_TOME));
						free(extraProject.data.tomeLocal);
						extraProject.data.tomeLocal = NULL;
					}
				}
			}
			else
			{
				extraProject.data.chapitresLocal = malloc(sizeof(uint));
				if(extraProject.data.chapitresLocal != NULL)
				{
					extraProject.data.chapitresLocal[0] = item.contentID;
					extraProject.data.nombreChapitreLocal++;
				}
			}

			item.projectData = extraProject;

			[output addObject:item];
		}
		else
		{
			[[NSAlert alertWithMessageText:@"Oh, relax, not quite ready for such a large import, soon!" defaultButton:@"Ok :c" alternateButton:@"Ok :c" otherButton:@"Ok :c" informativeTextWithFormat:@"WIP"] runModal];
			break;
		}
	}

	return [NSArray arrayWithArray:output];
}

void createIOConfigDatForData(NSString * path, char ** filenames, uint nbFiles)
{
    if(path == nil || filenames == NULL || nbFiles == 0)
        return;
    
    NSString * currentString, * extension;
    NSMutableArray * array = [NSMutableArray new];
    NSArray * formats = @[@"png", @"jpg", @"jpeg", @"pdf", @"tiff", @"gif"];
    
    //Look for files with valid formats
    for(uint i = 0; i < nbFiles; ++i)
    {
        if(filenames[i] == NULL)
            continue;
        
        currentString = [NSString stringWithUTF8String:filenames[i]];
        
        if(currentString == nil)
            continue;
        
        extension = [currentString pathExtension];
        
        for(NSString * currentExtension in formats)
        {
            if([extension caseInsensitiveCompare:currentExtension] == NSOrderedSame)
            {
                [array addObject:[currentString lastPathComponent]];
                break;
            }
        }
    }
    
    //No file
    if([array count] == 0 || [array count] > UINT_MAX)
        return;
	
	path = [path stringByAppendingString:@"/"];
    
    //We craft the config.dat
    FILE * config = fopen([[path stringByAppendingString:@CONFIGFILE] UTF8String], "w+");
    
    if(config == NULL)
        return;
    
    fprintf(config, "%u\nN", (uint) [array count]);
	
	uint counter = 0;
    
    for(NSString * file in array)
	{
		//We need to rename the file with a cleaner name
		NSString * newFile = [NSString stringWithFormat:@"%d.%@", counter++, [file pathExtension]];
		rename([[path stringByAppendingString:file] UTF8String], [[path stringByAppendingString:newFile] UTF8String]);
		
		fprintf(config, "\n0 %s", [newFile UTF8String]);
	}
	
    fclose(config);
}

@implementation RakImportNode
@end
