/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \__ \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **    Licence propriétaire, code source confidentiel, distribution formellement interdite  **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakMDLListView

- (id) init : (CGFloat) width : (CGFloat) height : (RakButton *) pause : (RakButton *) read : (RakButton *) remove : (id) controller
{
	self = [self initWithFrame: NSMakeRect(0, 0, width, height)];
	
	if(self != nil)
	{
		isSecondTextHidden = YES;
		
		self.wantsLayer = YES;
		self.layer.backgroundColor = [NSColor grayColor].CGColor;
		
		requestName = [[RakText alloc] initWithText:self.bounds : @"Attack on Titan chapitre XX" : [Prefs getSystemColor:GET_COLOR_INACTIVE]];
		if(requestName != nil)		{		[requestName sizeToFit];		[self addSubview:requestName];		}
		
		statusText = [[RakText alloc] initWithText:self.bounds : @"Terminé" : [Prefs getSystemColor:GET_COLOR_ACTIVE]];
		if(statusText != nil)		{		[statusText sizeToFit];		}
		
		_pause = [pause copy];
		if(_pause != nil)		[self addSubview:_pause];
		
		_read = [read copy];
		if(_read != nil)	{	[self addSubview:_read];		[_read setHidden:YES];	}
		
		_remove = [remove copy];
		if(_remove != nil)		[self addSubview:_remove];
		
		iconWidth = [_remove frame].size.width;
		
		[_pause setTarget:self];		[_pause setAction:@selector(sendPause)];
		[_read setTarget:self];			[_read setAction:@selector(sendRead)];
		[_remove setTarget:self];		[_remove setAction:@selector(sendRemove)];
		
		[self setPositionsOfStuffs];
	}
	
	return self;
}

- (void) setFont : (NSFont*) font
{
	[requestName setFont:font];
	[statusText setFont:font];
}

- (void) setPositionsOfStuffs
{
	NSRect frame = [self bounds], curFrame;
	NSPoint newPoint;
	
	if (requestName != nil)
	{
		curFrame = requestName.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x = 5;
		
		[requestName setFrameOrigin:newPoint];
	}
	
	newPoint.x = frame.size.width - 3;
	
	if (_remove != nil)
	{
		curFrame = _remove.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x -= 5 + curFrame.size.width;
		
		[_remove setFrameOrigin:newPoint];
	}
	
	if (_pause != nil)
	{
		curFrame = _pause.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x -= 5 + curFrame.size.width;
		
		[_pause setFrameOrigin:newPoint];
	}
	
	if(statusText != nil)
	{
		if(frame.size.width > 300)
		{
			if(isSecondTextHidden)
			{
				isSecondTextHidden = NO;
				[self addSubview:statusText];
			}
			
			curFrame = statusText.frame;
			
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
			newPoint.x -= curFrame.size.width;
			
			[statusText setFrameOrigin:newPoint];
		}
		else if(!isSecondTextHidden)
		{
			isSecondTextHidden = YES;
			[statusText removeFromSuperview];
		}
	}
}

- (void) setFrame:(NSRect)frameRect
{
	bool reposition = frameRect.size.width != self.frame.size.width || frameRect.size.height != self.frame.size.height;
	
	[super setFrame:frameRect];

	if(reposition)
		[self setPositionsOfStuffs];
}

- (void) updateData : (id) data
{
	NSLog(@"Supposed to update stuffs");
}

#pragma mark - Proxy

- (void) sendRemove
{
	
}

- (void) sendPause
{
	
}

- (void) sendRead
{
	
}

@end
