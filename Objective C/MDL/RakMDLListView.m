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

- (id) init : (CGFloat) width : (CGFloat) height : (RakButton *) pause : (RakButton *) read : (RakButton *) remove : (id) controller : (uint) rowID
{
	self = [self initWithFrame: NSMakeRect(0, 0, width, height)];
	
	if(self != nil)
	{
		isSecondTextHidden = YES;
		_controller = controller;
		_row = rowID;
		
		todoList = [_controller getData: _row : YES];
		if(todoList == NULL)
		{
			[self release];
			return nil;
		}
		else
			previousStatus = MDL_CODE_UNUSED;
		
		requestName = [[RakText alloc] initWithText:self.bounds : [self getName] : [Prefs getSystemColor:GET_COLOR_INACTIVE]];
		if(requestName != nil)		{		[requestName sizeToFit];		[self addSubview:requestName];		}
		
		statusText = [[RakText alloc] initWithText:self.bounds : @"Terminé" : [Prefs getSystemColor:GET_COLOR_ACTIVE]];
		if(statusText != nil)		{		[statusText sizeToFit];		}
		
		_pause = [pause copy];
		if(_pause != nil)	{	[self addSubview:_pause];		[_pause setHidden:YES];	}
		
		_read = [read copy];
		if(_read != nil)	{	[self addSubview:_read];		[_read setHidden:YES];	}
		
		_remove = [remove copy];
		if(_remove != nil)	{	[self addSubview:_remove];		[_remove setHidden:NO]; }
		
		DLprogress = [[RakProgressCircle alloc] initWithRadius:11 : NSMakePoint(0, 0)];
		if(DLprogress != nil){	[self addSubview:DLprogress];	[DLprogress setHidden:YES];	}
		
		iconWidth = [_remove frame].size.width;
		
		[_pause setTarget:self];		[_pause setAction:@selector(sendPause)];
		[_read setTarget:self];			[_read setAction:@selector(sendRead)];
		[_remove setTarget:self];		[_remove setAction:@selector(sendRemove)];
		
		[self setPositionsOfStuffs];
	}
	
	return self;
}

- (NSString *) getName
{
	NSString * name;
	
	if((*todoList)->partOfTome == VALEUR_FIN_STRUCTURE_CHAPITRE)
	{
		name = [NSString stringWithFormat:@"%s chapitre %d", (*todoList)->datas->mangaName, (*todoList)->chapitre / 10];
	}
	else
	{
		if((*todoList)->tomeName != NULL && (*todoList)->tomeName[0] != 0)
			name = [NSString stringWithFormat:@"%s %s", (*todoList)->datas->mangaName, (*todoList)->tomeName];
		else
			name = [NSString stringWithFormat:@"%s tome %d", (*todoList)->datas->mangaName, (*todoList)->partOfTome];
	}
	
	return [name stringByReplacingOccurrencesOfString:@"_" withString:@" "];
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
	
	//Text at extreme left
	if (requestName != nil)
	{
		curFrame = requestName.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x = 5;
		
		[requestName setFrameOrigin:newPoint];
	}
	
	newPoint.x = frame.size.width - 3;
	
	
	//Icon at extreme right
	if (_remove != nil)
	{
		curFrame = _remove.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x -= 5 + curFrame.size.width;
		
		[_remove setFrameOrigin:newPoint];
	}
	
	// Complementary icon
	if(_read != nil)
	{
		curFrame = _read.frame;

		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x -= 5 + curFrame.size.width;
		
		[_read setFrameOrigin:newPoint];
	}
	
	
	if (_pause != nil)
	{
		curFrame = _pause.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		
		if(_read == nil)
		{
			newPoint.x -= 5 + curFrame.size.width;
		}
		
		[_pause setFrameOrigin:newPoint];
	}
	
	if(DLprogress != nil)
	{
		curFrame = DLprogress.frame;
		
		newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
		newPoint.x -= curFrame.size.width + 5;
		
		[DLprogress setFrameOrigin:newPoint];
	}
	
	if(statusText != nil)
	{
		if(frame.size.width > 300)
		{
			if(previousStatus == MDL_CODE_INSTALL_OVER && isSecondTextHidden)
			{
				isSecondTextHidden = NO;
				[statusText setHidden:NO];
			}
			
			curFrame = statusText.frame;
			
			newPoint.y = frame.size.height / 2 - curFrame.size.height / 2;
			newPoint.x -= curFrame.size.width;
			
			[statusText setFrameOrigin:newPoint];
		}
		else if(!isSecondTextHidden)
		{
			isSecondTextHidden = YES;
			[statusText setHidden:YES];
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

- (void) updateData : (uint) data
{
	if(data != _row)
	{
		_row = data;
		todoList = [_controller getData : _row : YES];
		if(todoList == NULL)
			return;
		
		[requestName setStringValue : [self getName]];
		
		[self updateContext];
	}
	[self setPositionsOfStuffs];
}

#pragma mark - View management

- (void) updateContext
{
	int8_t newStatus = [_controller statusOfID : _row : YES];
	
	if(newStatus == previousStatus)
		return;
	
	if(![statusText isHidden])		[statusText setHidden:YES];
	if(![_pause isHidden])			[_pause setHidden:YES];
	if(![_read isHidden])			[_read setHidden:YES];
	if(![DLprogress isHidden])		[DLprogress setHidden:YES];
	
	[_read setState:RB_STATE_STANDARD];
	[_pause setState:RB_STATE_STANDARD];
	[_remove setState:RB_STATE_STANDARD];
	
	previousStatus = newStatus;

	switch (newStatus)
	{
		case MDL_CODE_DL:
		{
			[_pause setHidden:NO];
			[DLprogress setHidden:NO];
			[DLprogress updatePercentage:0];
			break;
		}
			
		case MDL_CODE_INSTALL:
		{
			[statusText setStringValue:@"Installation"];
			[statusText setHidden:NO];
			[self setPositionsOfStuffs];
			break;
		}
			
		case MDL_CODE_INSTALL_OVER:
		{
			[_read setHidden:NO];
			break;
		}
	}
	
	[self display];
}

#pragma mark - Proxy

- (void) updatePercentage : (CGFloat) percentage
{
	if(DLprogress != nil)
	{
		[DLprogress updatePercentage:percentage];
		[DLprogress performSelectorOnMainThread:@selector(notifyNeedDisplay:) withObject:nil waitUntilDone:YES];
	}
}

- (void) sendRemove
{
	if(todoList == nil)
		return;
	
	(*todoList)->downloadSuspended |= DLSTATUS_ABORT;	//Send the code to stop the download
	
	if(previousStatus == MDL_CODE_INSTALL_OVER)
	{
		//Deletion
		internalDeleteCT(*(*todoList)->datas, (*todoList)->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE, (*todoList)->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE ? (*todoList)->partOfTome : (*todoList)->chapitre);
	}
	
	NSView * view = self;
	
	while (view != nil && [view class] != [NSTableView class])
	{
		view = view.superview;
	}
	
	if(view == nil)
		return;
	
	NSInteger row = [(NSTableView*) view rowForView:self];
	
    if (row != -1)
        [(NSTableView*) view removeRowsAtIndexes:[NSIndexSet indexSetWithIndex:row] withAnimation:NSTableViewAnimationEffectFade];
	
	[_controller setStatusOfID: _row : MDL_CODE_ABORTED : YES];
	[_controller discardElement: _row];
	(*todoList)->rowViewResponsible = NULL;
	
	[(NSTableView *) view reloadData];
	
	while(view != nil && [view class] != [MDL class])
	{
		view = view.superview;
	}
	
	if(view == nil)
		return;
	
	if([(MDL*) view wouldFrameChange:[(MDL*) view createFrame]])
	{
		[NSAnimationContext beginGrouping];
		
		[(MDL*) view refreshLevelViews:view.superview :REFRESHVIEWS_NO_CHANGE];
		
		[NSAnimationContext endGrouping];
	}
}

- (void) sendPause
{
	if((*todoList)->curlHandler != NULL)
	{
		if((*todoList)->downloadSuspended & DLSTATUS_SUSPENDED)
		{
			(*todoList)->downloadSuspended &= ~DLSTATUS_SUSPENDED;
			curl_easy_pause((*todoList)->curlHandler, CURLPAUSE_CONT);
		}
		else
		{
			(*todoList)->downloadSuspended |= DLSTATUS_SUSPENDED;
			curl_easy_pause((*todoList)->curlHandler, CURLPAUSE_ALL);
		}
	}
}

- (void) sendRead
{
	NSView * view = self;
	
	while (view != nil && [view class] != [MDL class])
	{
		view = view.superview;
	}
	
	if(view == nil)
		return;
	
	updateIfRequired((*todoList)->datas, RDB_CTXMDL);

	[(MDL*) view propagateContextUpdate:*(*todoList)->datas :(*todoList)->partOfTome != VALEUR_FIN_STRUCTURE_CHAPITRE :(*todoList)->partOfTome == VALEUR_FIN_STRUCTURE_CHAPITRE ? (*todoList)->chapitre : (*todoList)->partOfTome];
}

@end
