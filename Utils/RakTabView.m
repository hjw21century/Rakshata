/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                          **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite   **
 **                                                                                          **
 *********************************************************************************************/

#import "superHeader.h"

@implementation RakTabView

- (NSView *) setUpView: (NSView *)superView
{
	NSRect frame = [self createFrame:superView];
	
	self = [super initWithFrame:frame];
	[superView addSubview:self];
	[self setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
	[self setAutoresizesSubviews:YES];
	[self setNeedsDisplay:YES];
	[self drawRect:frame];
	
	int mainThread;
	[Prefs getPref:PREFS_GET_MAIN_THREAD :&mainThread];
	readerMode = (mainThread & GUI_THREAD_READER) != 0;
	trackingArea = NULL;
		
	return self;
}

- (void) drawContentView: (NSRect) frame
{
	NSRectFill(frame);
}

- (void)drawRect:(NSRect)dirtyRect
{
	[self drawContentView:dirtyRect];
    [super drawRect:dirtyRect];
}

- (void) refreshLevelViews : (NSView*) superView
{
	NSArray *subView = [superView subviews];
	RakTabView *subViewView;
	NSUInteger i, count = [subView count];
	
	for(i = 0; i < count; i++)
	{
		subViewView = [subView objectAtIndex:i];
		if([subViewView respondsToSelector:@selector(refreshViewSize)])
			[subViewView refreshViewSize];
	}
}

- (void) refreshViewSize
{
	NSView * superView = [self superview];
	[self setFrameSize:NSMakeSize([self getRequestedViewWidth: superView.frame.size.width], superView.frame.size.height)];
	
	[self applyRefreshSizeReaderChecks];
}

- (void)setFrameSize:(NSSize)newSize
{
	NSRect frame = [self createFrame:[self superview]];
	[super setFrameSize:frame.size];
	[self setFrameOrigin:frame.origin];
}

/**		Reader		**/

- (void) readerIsOpening
{
	//Appelé quand les tabs ont été réduits
	if([self isCursorOnMe])
	{
		[Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER:flag];
		[self refreshLevelViews:[self superview]];
	}
	else
	{
		[self resizeReaderCatchArea];
	}
}

- (void) resizeReaderCatchArea
{
	[self releaseReaderCatchArea];
		
	trackingArea = [[NSTrackingArea alloc] initWithRect:[self generateNSTrackingAreaSize:[self frame]] options: (NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:self userInfo:nil];
	[self addTrackingArea:trackingArea];
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	viewFrame.origin.x = viewFrame.origin.y = 0;
	return viewFrame;
}

- (void) applyRefreshSizeReaderChecks
{
	bool isReaderMode;
	[Prefs getPref:PREFS_GET_IS_READER_MT :&isReaderMode];
	
	if(!readerMode && isReaderMode)
	{
		readerMode = true;
		[self readerIsOpening];
	}
	else if(readerMode && isReaderMode)
	{
		[self resizeReaderCatchArea];
	}
	else
	{
		readerMode = false;
		if(trackingArea != NULL)
		{
			[trackingArea release];
			trackingArea = NULL;
		}
	}
}

- (BOOL) isStillCollapsedReaderTab
{
	return true;
}

- (void) releaseReaderCatchArea
{
	if(trackingArea != NULL)
	{
		[self removeTrackingArea:trackingArea];
		[trackingArea release];
		trackingArea = NULL;
	}
}

/**		Events		**/

-(BOOL) isCursorOnMe
{
	NSPoint mouseLoc = [NSEvent mouseLocation], selfLoc = self.frame.origin;
	NSSize selfSize = self.frame.size;
	
	if(selfLoc.x < mouseLoc.x && selfLoc.x + selfSize.width >= mouseLoc.x &&
		selfLoc.y < mouseLoc.y && selfLoc.y + selfSize.height >= mouseLoc.y)
	{
		return true;
	}
	
	return false;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if([Prefs setPref:PREFS_SET_OWNMAINTAB:flag])
		[self refreshLevelViews : [self superview]];
}

- (void) mouseEntered:(NSEvent *)theEvent
{
	if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
		[self refreshLevelViews : [self superview]];
}

- (void)mouseExited:(NSEvent *)theEvent
{
	dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 0.25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
			if(![self isStillCollapsedReaderTab])
				{
					if([Prefs setPref:PREFS_SET_READER_TABS_STATE:STATE_READER_TAB_ALL_COLLAPSED])
						[self refreshLevelViews : [self superview]];
				}
		});
}


/*		Graphic Utilities		*/
- (NSRect) createFrame : (NSView*) superView
{
	NSRect frame;
	
	frame.origin.x = [self getRequestedViewPosX: superView.frame.size.width];
	frame.origin.y = [self getRequestedViewPosY: superView.frame.size.height];
	frame.size.width = [self getRequestedViewWidth: superView.frame.size.width];
	frame.size.height = [self getRequestedViewHeight: superView.frame.size.height];
	
	return frame;
}

- (int) convertTypeToPrefArg : (bool) getX
{
	return getX ? PREFS_GET_TAB_SERIE_POSX - PREFS_GET_TAB_SERIE_WIDTH : 0;
}

- (int) getRequestedViewPosX: (int) widthWindow
{
	int prefData;
	[Prefs getPref:[self convertTypeToPrefArg:YES]:&prefData];
	return widthWindow * prefData / 100;
}

- (int) getRequestedViewPosY: (int) heightWindow
{
	return 0;
}

- (int) getRequestedViewWidth:(int) widthWindow
{
	int prefData;
	[Prefs getPref:[self convertTypeToPrefArg:NO]:&prefData];
	return widthWindow * prefData / 100;
}

- (int) getRequestedViewHeight:(int) heightWindow
{
	return heightWindow;
}

@end