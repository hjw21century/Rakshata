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

#define LAST_BLOCK_POSITION 5

@implementation RakSRSearchTab

- (instancetype) initWithFrame:(NSRect)frameRect
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_isVisible = NO;
		_collapsed = YES;
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
		
		[Prefs getCurrentTheme:self];

		self.wantsLayer = YES;
		self.layer.borderWidth = 1;
		self.layer.borderColor = [self getBorderColor].CGColor;
		self.layer.backgroundColor = [self getBACKGROUNDColor].CGColor;
		
		placeholder = [[RakText alloc] initWithText:frameRect :NSLocalizedString(@"PROJ-EXPAND-FILTER-AREA", nil) : [self placeholderTextColor]];
		if(placeholder != nil)
		{
			placeholder.font = [self placeholderFont];

			[placeholder sizeToFit];
			[placeholder setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
			
			[self addSubview:placeholder];
		}
		
		[self initContent];
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(searchWasTriggered:) name:SR_NOTIF_NAME_SEARCH_TRIGGERED object:nil];
	}
	
	return self;
}

- (void) initContent
{
	author = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame : _bounds : 1] :SEARCH_BAR_ID_AUTHOR];
	if(author != nil)
	{
		author.alphaValue = 0;
		author.hidden = YES;
		[self addSubview:author];
	}
	
	source = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame:_bounds :2] :SEARCH_BAR_ID_SOURCE];
	if(source != nil)
	{
		source.alphaValue = 0;
		source.hidden = YES;
		[self addSubview:source];
	}
	
	type = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame:_bounds :3] :SEARCH_BAR_ID_TYPE];
	if(type != nil)
	{
		type.alphaValue = 0;
		type.hidden = YES;
		[self addSubview:type];
	}
	
	tag = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame:_bounds :4] :SEARCH_BAR_ID_TAG];
	if(tag != nil)
	{
		tag.alphaValue = 0;
		tag.hidden = YES;
		[self addSubview:tag];
	}
	
	extra = [[RakSRSearchTabGroup alloc] initWithFrame:[self getBlockFrame:_bounds :5] :SEARCH_BAR_ID_EXTRA];
	if(extra != nil)
	{
		extra.alphaValue = 0;
		extra.hidden = YES;
		[self addSubview:extra];
	}
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Drawing and view manipulation

- (void) setFrame : (NSRect) frameRect
{
	CGFloat oldWidth = self.bounds.size.width;
	
	[super setFrame:frameRect];
	
	[placeholder setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
	
	if(oldWidth != frameRect.size.width)
	{
		[author setFrame:[self getBlockFrame:frameRect :1]];
		[source setFrame:[self getBlockFrame:frameRect :2]];
		[type setFrame:[self getBlockFrame:frameRect :3]];
		[tag setFrame:[self getBlockFrame:frameRect :4]];
		[extra setFrame:[self getBlockFrame:frameRect :5]];
	}
}

- (void) resizeAnimation : (NSRect) frameRect
{
	CGFloat oldWidth = self.bounds.size.width;

	[self.animator setFrame:frameRect];
	[placeholder.animator setFrameOrigin:NSCenterPoint(frameRect, placeholder.bounds)];
	
	if(oldWidth != frameRect.size.width)
	{
		[author resizeAnimation:[self getBlockFrame:frameRect :1]];
		[source resizeAnimation:[self getBlockFrame:frameRect :2]];
		[type resizeAnimation:[self getBlockFrame:frameRect :3]];
		[tag resizeAnimation:[self getBlockFrame:frameRect :4]];
		[extra resizeAnimation:[self getBlockFrame:frameRect :5]];
	}
}

- (void) drawRect:(NSRect)dirtyRect
{
	if(_isVisible && _collapsed)
	{
		[[[NSColor blackColor] colorWithAlphaComponent:0.3] setFill];
		NSRectFill(dirtyRect);
	}
}

#pragma mark - Frame

#define BORDER_HORIZON 	6
#define BORDER_VERT 	25
#define BORDER_INTER	10
#define LAST_BLOCK_WIDTH 125

- (NSRect) getBlockFrame : (NSRect) frame : (byte) position
{
	CGFloat blockWidth;
	
	frame.size.height = SR_SEARCH_TAB_EXPANDED_HEIGHT - 2 * BORDER_HORIZON;
	
	frame.size.width -= BORDER_VERT + LAST_BLOCK_WIDTH;
	frame.size.width /= 4;
	blockWidth = frame.size.width;
	
	if(position < LAST_BLOCK_POSITION)
		frame.size.width -= BORDER_INTER;
	else
		frame.size.width = LAST_BLOCK_WIDTH;
	
	
	frame.origin.x = (position - 1) * blockWidth + BORDER_INTER;
	frame.origin.y = BORDER_HORIZON;
	
	return frame;
}

#pragma mark - Colors

- (NSColor *) getBorderColor
{
	if(!_isVisible)
		return [Prefs getSystemColor:GET_COLOR_SEARCHTAB_BORDER_BAR :nil];

	else if(_collapsed)
		return [Prefs getSystemColor:GET_COLOR_SEARCHTAB_BORDER_COLLAPSED :nil];

	return [Prefs getSystemColor:GET_COLOR_SEARCHTAB_BORDER_DEPLOYED :nil];
}

- (NSColor *) getBACKGROUNDColor
{
	return [Prefs getSystemColor:GET_COLOR_SEARCHTAB_BACKGROUND :nil];
}

- (NSColor *) placeholderTextColor
{
	return [Prefs getSystemColor:GET_COLOR_SRPLACEHOLDER_TEXT :nil];
}

- (NSFont *) placeholderFont
{
	return [NSFont fontWithName:[Prefs getFontName:GET_FONT_PLACEHOLDER] size:14];
}

- (void) observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
	if([object class] != [Prefs class])
		return;

	placeholder.font = [self placeholderFont];
	[placeholder sizeToFit];
	[placeholder setFrameOrigin:NSCenterPoint(self.bounds, placeholder.bounds)];

	placeholder.textColor = [self placeholderTextColor];
	self.layer.borderColor = [self getBorderColor].CGColor;
	self.layer.backgroundColor = [self getBACKGROUNDColor].CGColor;
}

#pragma mark Interactions

- (void) mouseDown:(NSEvent *)theEvent
{
	if (!_isVisible || !_collapsed)
		return;

	[self updateCollapseState:!_collapsed : NO];
	[self updateGeneralFrame];
}

- (void) updateCollapseState : (BOOL) newIsCollapsed : (BOOL) silentUpdate
{
	if(_collapsed != newIsCollapsed)
		_collapsed = newIsCollapsed;
	else
		return;
	
	//We update the state of those 5 linked views
	for(NSView * view in @[author, source, type, tag, extra])
	{
		if(silentUpdate)
		{
			view.hidden = _collapsed;
			view.alphaValue = !_collapsed;
		}
		else
		{
			if(!_collapsed)
				view.hidden = NO;
				
			view.animator.alphaValue = !_collapsed;
		}
	}

	placeholder.hidden = !_collapsed;

	if(silentUpdate)
	{
		placeholder.alphaValue = _collapsed;

		self.layer.borderColor = [self getBorderColor].CGColor;
		return;
	}
	
	[NSAnimationContext beginGrouping];
	
	placeholder.animator.alphaValue = _collapsed;
	self.layer.borderColor = [self getBorderColor].CGColor;
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		for(NSView * view in @[author, source, type, tag, extra, placeholder])
		{
			if(view.alphaValue == 0)
				view.hidden = YES;
		}
	}];

	[NSAnimationContext endGrouping];
	
	if(_collapsed)
		_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
	else
		_height = SR_SEARCH_TAB_EXPANDED_HEIGHT;
}

- (void) updateGeneralFrame
{
	if([self.superview class] == [Series class])
		[(Series *) self.superview resetFrameSize:YES];
}

#pragma mark - Interface with header

- (void) searchWasTriggered : (NSNotification *) notification
{
	NSNumber * ID = notification.object;
	if(ID == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	NSDictionary * dict = notification.userInfo;
	NSNumber * number;
	
	if(dict != nil 	&& (number = [dict objectForKey:SR_NOTIF_NEW_STATE]) != nil && [number isKindOfClass:[NSNumber class]] && _isVisible != number.boolValue)
	{
		if(((!_isVisible || _collapsed) && [ID unsignedCharValue] == SEARCH_BAR_ID_MAIN_TRIGGERED) || [ID unsignedCharValue] == SEARCH_BAR_ID_FORCE_CLOSE)
			[self mainSearchWasTriggered:number.boolValue];
	}
}

- (void) mainSearchWasTriggered : (BOOL) isVisible
{
	if(isVisible == _isVisible)
		return;
	
	//We need to check if the focus is not just changing to an other search bar
	if(!isVisible)
	{
		//Change to a sub-search bar
		if([((RakWindow *)self.window).imatureFirstResponder class] == [RakSRSearchBar class])
			return;
		
		//Selection of element in a sub-list
		if([((RakWindow *)self.window).imatureFirstResponder class] == [RakTableView class])
			return;
	}
	
	_isVisible = isVisible;
	
	if(_isVisible)
	{
		self.layer.borderColor = [self getBorderColor].CGColor;
		[self updateCollapseState : YES : YES];
		_height = SR_SEARCH_TAB_INITIAL_HEIGHT;
	}
	else
		_height = SRSEARCHTAB_DEFAULT_HEIGHT;
	
	[NSAnimationContext beginGrouping];
	
	[self updateGeneralFrame];
	
	if(!_isVisible)
	{
		[[NSAnimationContext currentContext] setCompletionHandler:^{
			self.layer.borderColor = [self getBorderColor].CGColor;
		}];
	}
	
	[NSAnimationContext endGrouping];
}

@end
