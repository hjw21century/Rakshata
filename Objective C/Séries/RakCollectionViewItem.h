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

@interface RakCollectionViewItem : NSView
{
	BOOL _animationRequested, _noDrag;
	uint currentRequestID;
	PROJECT_DATA _project;
	
#ifdef TRACKING_AREA_WORK_PROPERLY
	BOOL haveTrackRect;
	NSTrackingArea * trackingArea;
#endif
	
	uint * _sharedActive;
	
	//Content
	RakText * name, * author;
	NSImageView * thumbnails;
	RakText * mainTag;
}

@property BOOL selected;
@property CGFloat requestedHeight;
@property NSRect workingArea;

- (instancetype) initWithProject : (PROJECT_DATA) project : (uint *) sharedActive;

@end
