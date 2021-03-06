/*********************************************************************************************
 **	__________         __           .__            __                 ________   _______   	**
 **	\______   \_____  |  | __  _____|  |__ _____ _/  |______   	___  _\_____  \  \   _  \  	**
 **	 |       _/\__  \ |  |/ / /  ___/  |  \\__  \\   __\__  \  	\  \/ //  ____/  /  /_\  \ 	**
 **	 |    |   \ / __ \|    <  \___ \|   Y  \/ __ \|  |  / __ \_  \   //       \  \  \_/   \	**
 **	 |____|_  /(____  /__|_ \/____  >___|  (____  /__| (____  /	  \_/ \_______ \ /\_____  /	**
 **	        \/      \/     \/     \/     \/     \/          \/ 	              \/ \/     \/ 	**
 **                                                                                         **
 **			This Source Code Form is subject to the terms of the Mozilla Public				**
 **			License, v. 2.0. If a copy of the MPL was not distributed with this				**
 **			file, You can obtain one at https://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

@implementation RakSRTagRail

- (instancetype) initWithFrame : (NSRect) frameRect : (CGFloat) baseSearchBar
{
	self = [super initWithFrame:frameRect];
	
	if(self != nil)
	{
		_noReorder = NO;
		
		if(baseSearchBar == 0)
		{
			_baseSearchBar = 0;
			_reducedWidth = frameRect.size.width;
		}
		else
		{
			_baseSearchBar = baseSearchBar - 5;
			_reducedWidth = _baseSearchBar - frameRect.origin.x;
		}
		
		_currentX = _currentRow = 0;
		tagList = [NSMutableArray array];
		tagNames = [NSMutableArray array];
		
		_nbRow = _currentRow + 1;
		
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifTag:) name:SR_NOTIFICATION_TAG object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifType:) name:SR_NOTIFICATION_TYPE object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifAuthor:) name:SR_NOTIFICATION_AUTHOR object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotifSource:) name:SR_NOTIFICATION_SOURCE object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(flushTags) name:SR_NOTIFICATION_FLUSH object:nil];
	}
	
	return self;
}

- (void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void) receiveNotifType : (NSNotification *) notification
{
	NSString * string;
	NSNumber * ID, * opType;
	
	if(notification == nil || (string = notification.object) == nil || notification.userInfo == nil || (ID = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];
	
	[self performOperation:string : [ID unsignedLongLongValue] : RDBS_TYPE_CAT : insertion];
}

- (void) receiveNotifTag : (NSNotification *) notification
{
	NSString * string;
	NSNumber * ID, * opType;
	
	if(notification == nil || (string = notification.object) == nil || notification.userInfo == nil || (ID = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];
	
	[self performOperation:string : [ID unsignedLongLongValue] : RDBS_TYPE_TAG : insertion];
}

- (void) receiveNotifAuthor : (NSNotification *) notification
{
	NSString * string;
	NSNumber * ID, * opType;
	
	if(notification == nil || (string = notification.object) == nil || notification.userInfo == nil || (ID = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];
	
	[self performOperation: string : [ID unsignedLongLongValue] : RDBS_TYPE_AUTHOR : insertion];
}

- (void) receiveNotifSource : (NSNotification *) notification
{
	NSString * string;
	NSNumber * ID, * opType;
	
	if(notification == nil || (string = notification.object) == nil || notification.userInfo == nil || (ID = [notification.userInfo objectForKey:SR_NOTIF_CACHEID]) == nil || ![ID isKindOfClass:[NSNumber class]])
		return;
	
	BOOL insertion = (opType = [notification.userInfo objectForKey:SR_NOTIF_OPTYPE]) == nil || ![opType isKindOfClass:[NSNumber class]] || [opType boolValue];
	
	[self performOperation: string : [ID unsignedLongLongValue] : RDBS_TYPE_SOURCE : insertion];
}

- (void) performOperation : (NSString *) object : (uint64_t) ID : (byte) dataType : (BOOL) insertion
{
	if(insertion)
	{
		if(![object isKindOfClass:[NSString class]] || haveRestriction(ID, dataType))
			return;
		
		[self addTag:object];
		
		RakSRTagItem * item = [tagList lastObject];
		if(item != nil && [item class] == [RakSRTagItem class])
		{
			item._dataType = dataType;
			item._dataID = ID;
		}
		
		insertRestriction(ID, dataType);
	}
	else
	{
		uint index = [object isKindOfClass:[NSNumber class]] ? [(NSNumber *) object unsignedIntValue] : [tagNames indexOfObject:object];
		[self removeTag:index];
		removeRestriction(ID, dataType);
	}
}

#pragma mark - Tag management

- (BOOL) addTag : (NSString *) tagName
{
	uint tagLength = [tagList count], length;
	NSRect frame;
	BOOL newRow = NO;
	
	//Create animation and update the model
	[NSAnimationContext beginGrouping];
	
	[self insertTags:@[tagName] :self.bounds];
	[self updateContext];
	
	//Update the UI
	length = [tagList count];
	for(RakSRTagItem * item; tagLength < length; tagLength++)
	{
		item = [tagList objectAtIndex:tagLength];
		frame = item.frame;
		
		if(newRow || frame.origin.x == 0 || frame.origin.y == 0)
		{
			item.alphaValue = 0;
			item. alphaAnimated = 1;
			
			if(frame.origin.x != 0)			newRow = YES;
		}
		else
		{
			item.frame = NSMakeRect(self.bounds.size.width, frame.origin.y, frame.size.width, frame.size.height);
			item.frameAnimated = frame;
		}
	}
	
	[NSAnimationContext endGrouping];
	
	return YES;
}

- (void) updateContext
{
	if(_nbRow != _currentRow + 1)
	{
		_nbRow = _currentRow + 1;
		
		Series * tabSerie = RakApp.serie;
		if(tabSerie != nil)
		{
			_noReorder = YES;
			[tabSerie resetFrameSize : YES];
			_noReorder = NO;
		}
	}
}

- (void) insertTags : (NSArray *) tags : (NSRect) frame
{
	uint _width = frame.size.width, width = _baseSearchBar != 0 && _currentRow == 0 ? _reducedWidth : _width, index = [tagList count];
	
	RakSRTagItem * tag;
	for(NSString * tagString in tags)
	{
		tag = [[RakSRTagItem alloc] initWithTag : tagString];
		if(tag == nil)
			continue;
		else
		{
			[tagList addObject:tag];
			[tagNames addObject:tagString];
			tag.parent = self;
		}
		
		if(_currentX + tag.bounds.size.width > width)
		{
			if(!_currentRow)
				width = _width;
			
			_currentX = 0;
			_currentRow++;
		}
		
		[tag setFrameOrigin:NSMakePoint(_currentX, _currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER))];
		[self addSubview:tag];
		
		tag.index = index++;
		_currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
}

- (void) reorderTags: (NSRect) frame : (BOOL) animated
{
	uint _width = frame.size.width, width = _baseSearchBar != 0 ? _reducedWidth : _width, index = 0;
	NSPoint point;
	
	_currentX = _currentRow = 0;
	
	for(RakSRTagItem * tag in tagList)
	{
		if(_currentX + tag.bounds.size.width > width)
		{
			if(!_currentRow)
				width = _width;
			
			_currentX = 0;
			_currentRow++;
		}
		
		point = NSMakePoint(_currentX, _currentRow * (TAG_BUTTON_HEIGHT + TAG_RAIL_INTER_RAIL_BORDER));
		
		if(animated)
			[tag setFrameOriginAnimated:point];
		else
			[tag setFrameOrigin:point];
		
		tag.index = index++;
		_currentX += tag.bounds.size.width + TAG_RAIL_INTER_ITEM_BORDER;
	}
	
	if(_nbRow != _currentRow + 1)
	{
		_nbRow = _currentRow + 1;
		
		Series * tabSerie = RakApp.serie;
		if(tabSerie != nil)
		{
			_noReorder = YES;
			[tabSerie resetFrameSize : YES];
			_noReorder = NO;
		}
	}
}

- (void) removeTag : (uint) index
{
	if(index >= [tagList count])
		return;
	
	__block RakSRTagItem * tag = [tagList objectAtIndex:index];
	
	[tagList removeObjectAtIndex:index];
	[tagNames removeObjectAtIndex:index];
	
	[NSAnimationContext beginGrouping];
	
	[[NSAnimationContext currentContext] setCompletionHandler:^{
		[tag removeFromSuperview];
		[tag setAlphaValue:1];
	}];
	
	[tag setAlphaAnimated:0];
	
	[self reorderTags:self.bounds : YES];
	
	[NSAnimationContext endGrouping];
}

- (void) flushTags
{
	while([tagList count])
		[self removeTag:0];
	
	flushRestriction();
}

#pragma mark - Properties

- (void) setBaseSearchBar : (CGFloat) baseSearchBar
{
	if(baseSearchBar == 0)
	{
		_baseSearchBar = 0;
		_reducedWidth = self.frame.size.width;
	}
	else
	{
		_baseSearchBar = baseSearchBar - 5;
		_reducedWidth = _baseSearchBar - self.frame.origin.x;
	}
}

#pragma mark - View management

- (void) _resize:(NSRect)frameRect : (BOOL) animated
{
	BOOL haveToReorder = !_noReorder && frameRect.size.width != self.bounds.size.width && _nbRow > 1;
	
	_reducedWidth = _baseSearchBar != 0 ? _baseSearchBar - frameRect.origin.x : frameRect.size.width;
	
	if(animated)
		[self setFrameAnimated:frameRect];
	else
		[super setFrame:frameRect];
	
	if(haveToReorder)
		[self reorderTags:frameRect :animated];
}

- (void) setFrame:(NSRect)frameRect
{
	[self _resize:frameRect :NO];
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[self _resize:frameRect :YES];
}

- (BOOL) isFlipped
{
	return YES;
}

@end
