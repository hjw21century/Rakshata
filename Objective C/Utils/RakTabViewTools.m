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

@implementation RakTabForegroundView

- (id) initWithFrame: (NSRect) frameRect : (RakTabView *) father : (NSString *) detail
{
	self = [super initWithFrame:frameRect :nil];
	
	if(self != nil)
	{
		tabView = father;
		
		NSColor * textColor = [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT : nil];

		head = [[[RakText alloc] initWithText:frameRect :@"Connexion requise" : textColor] autorelease];
		[head setFont:[NSFont boldSystemFontOfSize:[NSFont systemFontSize]]];
		[head sizeToFit];
		
		main = [[[RakText alloc] initWithText:frameRect : detail : textColor] autorelease];
		[main setAlignment:NSCenterTextAlignment];
		[main sizeToFit];
		[main setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 -  head.bounds.size.height * 0.75)];
		[self addSubview:main];
		
		[head setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
		[self addSubview:head];
	}
	
	return self;
}

- (void) dealloc
{
	[head removeFromSuperview];
	[main removeFromSuperview];
	[super dealloc];
}

- (void) setFrame : (NSRect)frameRect
{
	frameRect.origin = NSZeroPoint;
	
	[super setFrame:frameRect];
	[main setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 - head.bounds.size.height * 0.75)];
	[head setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	frameRect.origin = NSZeroPoint;
	[self.animator setFrame:frameRect];
	[main.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - main.bounds.size.width / 2, frameRect.size.height / 2 - main.bounds.size.height / 2 - head.bounds.size.height * 0.75)];
	[head.animator setFrameOrigin:NSMakePoint(frameRect.size.width / 2 - head.bounds.size.width / 2, NSMaxY(main.frame) + head.bounds.size.height / 2)];
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	
	NSColor * textColor = [Prefs getSystemColor:GET_COLOR_CLICKABLE_TEXT : nil];

	for(RakText * view in self.subviews)
	{
		[view setTextColor:textColor];
	}
}

- (NSColor*) getBackgroundColor
{
	return [[super getBackgroundColor] colorWithAlphaComponent:0.6f];
}

- (void) mouseDown : (NSEvent*) theEvent
{
	[[NSApp delegate] openLoginPrompt];
}

@end