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

@class RakPrefsRepoView;

@interface RakPrefsRepoList : RakList

@property RakPrefsRepoView * __weak responder;

@property BOOL rootMode;

- (instancetype) initWithFrame : (NSRect) frame;

@end

@interface RakPrefsRepoView : NSView
{
	ROOT_REPO_DATA ** root;
	REPO_DATA ** repo;
	
	uint nbRoot, nbRepo;
	
	RakPrefsRepoList * list;
}

- (void **) listForMode : (BOOL) rootMode;
- (uint) sizeForMode : (BOOL) rootMode;

- (NSString *) nameOfParent : (uint) parentID;

@end
