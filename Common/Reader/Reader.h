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
 ********************************************************************************************/

#include "lecteur.h"
#import "RakPageScrollView.h"
#import "RakPageController.h"

#define READER_MAGNIFICATION_MIN 0.25
#define READER_MAGNIFICATION_MAX 3.0

@class RakReaderBottomBar;

@interface Reader : RakTabView
{
	BOOL initialized;
	
	uint gonnaReduceTabs;
	RakView * container;
	RakReaderBottomBar * bottomBar;
	RakReaderControllerUIQuery * newStuffsQuery;
	
	BOOL queryHidden;
	uint * _queryArrayData;
	size_t _queryArraySize;
	
	//Page management
	BOOL saveMagnification, overrideDirection;
	CGFloat lastKnownMagnification;
	RakPageScrollView * _scrollView;
	
	RakPageController * mainScroller;
	RakImage * loadingPlaceholder;
	RakImage * loadingFailedPlaceholder;
	
	MUTEX_VAR cacheMutex;
	uint cacheSession, workingCacheSession;
	byte previousMove;
	
	BOOL bottomBarHidden, oldDFState;
	NSTimer * delaySinceLastMove;
	NSPoint cursorPosBeforeLastMove;
	
	//Context data
	BOOL _readerMode;
	BOOL _alreadyRefreshed;
	BOOL _dontGiveACrapAboutCTPosUpdate;
	BOOL _flushingCache;
	
	BOOL _cacheBeingBuilt;
	BOOL _haveScrollerPosToCommit;
	NSPoint _scrollerPosToCommit;
	BOOL _endingTransition;
	
	PROJECT_DATA _project;
	BOOL dataLoaded;
	DATA_LECTURE _data;
	
	uint _currentElem;
	uint _posElemInStructure;
	
	//Cache data
	BOOL previousDataLoaded;
	DATA_LECTURE _previousData;
	BOOL nextDataLoaded;
	DATA_LECTURE _nextData;
}

@property BOOL distractionFree;
@property BOOL isTome;
@property BOOL preventRecursion;

- (instancetype) init : (RakView*)contentView : (NSString *) state;
- (void) initReaderMainView : (NSString *) state;

- (void) restoreProject : (PROJECT_DATA) project withInsertionPoint : (NSDictionary *) insertionPoint;

- (void) startReading : (PROJECT_DATA) project : (uint) elemToRead : (BOOL) isTome : (uint) startPage;
- (void) resetReader;
- (void) willLeaveReader;
- (void) willOpenReader;

- (void) collapseAllTabs : (BOOL) forced;
- (void) hideBothTab;
- (void) unhideBothTab;

- (void) switchDistractionFree;
- (void) shouldLeaveDistractionFreeMode;
- (void) startFadeTimer : (NSPoint) cursorPosition;
- (void) abortFadeTimer;
- (void) cursorShouldFadeAway;
- (void) fadeBottomBar : (CGFloat) alpha;

- (PROJECT_DATA) activeProject;
- (uint) currentElem;

- (void) switchFavs;
- (void) triggerFullscreen;
- (void) updatePage : (uint) newCurrentPage : (uint) newPageMax;
- (void) updateTitleBar : (PROJECT_DATA) project : (BOOL) isTome : (uint) position;

@end

#import "RakReaderSuggestions.h"
#import "RakReaderControllerUIQuery.h"
#import "RakPageCounter.h"
#import "RakFavsInfo.h"
#import "RakDeleteConfirm.h"
#import "RakReaderBottomBar.h"
#import "RakPage.h"

BOOL preventWindowCaptureForWindow(NSWindow *window);

enum
{
	RD_CONTROLBAR_HEIGHT = 26,
	RD_CONTROLBAR_WIDHT_PERC = 75,
	RD_CONTROLBAR_WIDHT_MIN = 500,
	RD_CONTROLBAR_WIDHT_MAX	= 1000,
	RD_CONTROLBAR_POSY = 20,
	
	READER_BORDURE_VERT_PAGE = 10,
	READER_PAGE_TOP_BORDER = 50,
	READER_PAGE_BOTTOM_BORDER = (RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + 10),
	READER_PAGE_BORDERS_HIGH = (READER_PAGE_TOP_BORDER + READER_PAGE_BOTTOM_BORDER),
};