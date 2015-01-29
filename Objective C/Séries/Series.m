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

@implementation Series

- (id)init : (NSView*) contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = TAB_SERIES;
		
		self = [self initView : contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS:self].CGColor;
		self.layer.borderWidth = 2;
		
		[self initContent : state];
	}
    return self;
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;
	
	[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

- (void) initContent : (NSString *) state
{
	header = [[RakSRHeader alloc] initWithFrame:self.bounds : self.mainThread == TAB_SERIES];
	if(header != nil)
	{
		header.responder = self;
		[self addSubview:header];
	}
	
	coreView = [[RakSerieView alloc] initContent:[self getCoreviewFrame : self.frame] : state];
	[self addSubview:coreView];
}

- (NSString *) byebye
{
	NSString * output;
	
	if (coreView == nil || (output = [coreView getContextToGTFO]) == nil)
		return [super byebye];
	else
		[self removeFromSuperview];
	
	return output;
}

- (void) dealloc
{
	[header removeFromSuperview];
	[coreView removeFromSuperview];
	[self removeFromSuperview];
}

- (BOOL) abortCollapseReaderTab
{
	return header.prefUIOpen;
}

- (void)mouseExited:(NSEvent *)theEvent	//Appelé quand je sors
{
	if(!header.prefUIOpen)
		[super mouseExited:theEvent];
}

#pragma mark - Routine to setup and communicate with coreview

- (NSRect) getCoreviewFrame : (NSRect) frame
{
	frame.size.height -= header.height;
	frame.origin.x = SR_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.origin.y = header.height;
	frame.size.width -= 2 * frame.origin.x;
	
	return frame;
}

- (void) setUpViewForAnimation : (uint) mainThread
{
	[header updateFocus : mainThread];
	[coreView focusViewChanged : mainThread];
	
	[super setUpViewForAnimation : mainThread];
}

- (void) displayTypeUpdate : (uint) activeCell
{
	if(activeCell == SR_CELLTYPE_GRID)
		NSLog(@"Would update to grid");
	else if(activeCell == SR_CELLTYPE_REPO)
		NSLog(@"Would update to repo view");
	else if(activeCell == SR_CELLTYPE_LIST)
		NSLog(@"Would update to list");
	else
		NSLog(@"Would fail to update");
}

#pragma mark - RakTabView routines

- (BOOL) needToConsiderMDL
{
	BOOL isReader;
	[Prefs getPref : PREFS_GET_IS_READER_MT : &isReader];
	
	return isReader;
}

- (void) setFrame : (NSRect) frameRect
{
	if([self wouldFrameChange:frameRect])
	{
		[super setFrame:frameRect];
		
		frameRect.origin = NSZeroPoint;

		[header setFrame:frameRect];
		[coreView setFrame:[self getCoreviewFrame : frameRect]];
	}
}

- (void) resizeAnimation
{
	NSRect newFrame = [self createFrame];
	if([self wouldFrameChange:newFrame])
	{
		[self.animator setFrame:newFrame];
		
		newFrame.origin = NSZeroPoint;
		
		[header resizeAnimation:newFrame];
		[coreView resizeAnimation:[self getCoreviewFrame : newFrame]];
	}
}

- (void) refreshViewSize
{
	[super refreshViewSize];
	[self refreshLevelViews: self : REFRESHVIEWS_CHANGE_READER_TAB];
}

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_CT_FRAME :&output];
	
	NSSize sizeSuperView = [self.superview frame].size;
	
	output.origin.x *= sizeSuperView.width / 100.0f;
	output.origin.y *= sizeSuperView.height / 100.0f;
	output.size.width *= sizeSuperView.width / 100.0f;
	output.size.height *= sizeSuperView.height / 100.0f;
	
	return output;
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_SERIE_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_SERIE_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_SERIE_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_SERIE_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_SERIE_FRAME;
			break;
		}
			
		default:
			output = 0;
	}
	
	return output;
}

/**		Reader		**/
- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_SERIE_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat var;
	NSRect frame = viewFrame;
	NSSize svSize = self.superview.frame.size;
	
	[Prefs getPref:PREFS_GET_TAB_CT_POSX : &(frame.size.width) : &svSize];
	frame.origin.x = 0;
	
	MDL * tabMDL = [self getMDL : YES];

	if(tabMDL != nil)
	{
		var = [tabMDL lastFrame].size.height - [tabMDL lastFrame].origin.y - viewFrame.origin.y;
		
		if(var > 0)
		{
			frame.origin.y = var;
			frame.size.height -= var;
		}
		else
			frame.origin.y = 0;
	}
	else
		frame.origin.y = 0;
	
	return frame;
}

- (void) seriesIsOpening:(byte)context
{
	[((RakAppDelegate *)[NSApp delegate]).window resetTitle];
}

@end
