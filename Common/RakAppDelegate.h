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

@class RakAboutWindow;

@interface RakAppDelegate : NSObject
{
	Series * tabSerie;
	CTSelec * tabCT;
	Reader * tabReader;
	MDL * tabMDL;
	
	RakContentView * _contentView;
	
	BOOL loginPromptOpen;
	pthread_cond_t loginLock;
	MUTEX_VAR loginMutex;
}

@property BOOL initialized;
@property BOOL haveDistractionFree;

- (RakView *) earlyInit;
- (void) mainInit : (RakView *) contentView;

- (pthread_cond_t*) sharedLoginLock;
- (MUTEX_VAR *) sharedLoginMutex : (BOOL) locked;

- (void) openLoginPrompt;
- (void) loginPromptClosed;

- (RakContentView*) getContentView;

- (Series *)	serie;
- (CTSelec *)	CT;
- (MDL *)		MDL;
- (Reader *)	reader;

//Shouldn't be accessed directly
- (Series *)	_serie;
- (CTSelec *)	_CT;
- (MDL *)		_MDL;
- (Reader *)	_reader;

- (void) flushState;

@end