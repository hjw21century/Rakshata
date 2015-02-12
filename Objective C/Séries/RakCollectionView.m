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

@implementation RakCollectionView

- (instancetype) initWithFrame : (NSRect) frameRect : (RakSRContentManager *) manager
{
	self = [super initWithFrame:[self frameFromParent : frameRect]];
	
	if(self != nil)
	{
		_resized = YES;
		_manager = manager;
		
		self.selectable = YES;
		self.allowsMultipleSelection = NO;
		self.backgroundColors = @[[NSColor clearColor]];
		self.minItemSize = NSMakeSize(RCVC_MINIMUM_WIDTH_OFFSET, RCVC_MINIMUM_HEIGHT_OFFSET);
		[self bind:NSContentBinding toObject:_manager withKeyPath:@"sharedReference" options:nil];
		
		[self setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:YES];
		[self setDraggingSourceOperationMask:NSDragOperationMove | NSDragOperationCopy forLocal:NO];
	}
	
	return self;
}

- (void) viewDidMoveToSuperview
{
	NSView * superview = self.superview;
	
	while(superview != nil && ![superview isKindOfClass:[NSClipView class]])
		superview = superview.superview;
	
	_clipView = (id) superview;
	
	[super viewDidMoveToSuperview];
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:[self frameFromParent : frameRect]];
	_resized = YES;
}

- (void) resizeAnimation : (NSRect) frameRect
{
	[super.animator setFrame:[self frameFromParent : frameRect]];
	_resized = YES;
}

- (NSRect) frameFromParent : (NSRect) frame
{
	return frame;
}

- (void) mouseDown:(NSEvent *)theEvent
{
	_draggedSomething = NO;

	[super mouseDown:theEvent];
	
	if(!_draggedSomething && _clickedView != nil)
		[_clickedView mouseUp:theEvent];

	_clickedView = nil;
}

#pragma mark - Generate views

- (NSCollectionViewItem *) newItemForRepresentedObject:(id)object
{
	uint cacheCode;
	
	if(object == nil || ![object isKindOfClass:[NSNumber class]])
		return nil;
	
	cacheCode = [object unsignedIntValue];	
	PROJECT_DATA * project = [_manager getDataAtIndex:cacheCode];
	if(project == NULL)
		return nil;
	
	NSCollectionViewItem * output = [NSCollectionViewItem new];
	
	RakCollectionViewItem * item = [[RakCollectionViewItem alloc] initWithProject:*project];
	
	output.view = item;
	output.representedObject = object;
	
	return output;
}

#pragma mark - Drag & Drop tools

- (BOOL) isValidIndex : (uint) index
{
	return index < [_manager nbElement];
}

- (uint) cacheIDForIndex : (uint) index
{
	PROJECT_DATA * project = [_manager getDataAtIndex:index];
	
	if(project == NULL)
		return UINT_MAX;
	
	return project->cacheDBID;
}

#pragma mark - Handle mouse movement

/*
 *	NSScrollView and NSTrackingArea seems to love each other...
 *	As their repositionning is completly messed up in some pretty common cases, we can't rely on them
 *	Thanks to that, we have to do its job...
 */

- (void) mouseEntered:(NSEvent *)theEvent
{
	[self mouseMoved:theEvent];
}

//#define VERBOSE_HACK

- (void) mouseMoved:(NSEvent *)theEvent
{
#ifdef VERBOSE_HACK
	NSLog(@"Mouse moved, starting the work");
#endif
	
	if(_clipView == nil)
	{
#ifdef VERBOSE_HACK
		NSLog(@"Invalid context, re-building");
#endif
		[self viewDidMoveToSuperview];
	}
	
	const NSPoint originalPoint = [theEvent locationInWindow], pointInDocument = [_clipView convertPoint:originalPoint fromView:nil];
#ifdef VERBOSE_HACK
	NSLog(@"Cursor at x:%lf, y:%lf ~ Position in document: x:%lf, y:%lf", originalPoint.x, originalPoint.y, pointInDocument.x, pointInDocument.y);
#endif
	
	if(selectedItem != nil)
	{
#ifdef VERBOSE_HACK
		NSLog(@"Existing item: %@", selectedItem);
#endif
		
		if([self validateItem:selectedItem :originalPoint])
		{
#ifdef VERBOSE_HACK
			NSLog(@"Still in area, we're good");
#endif
			return;
		}
		
#ifdef VERBOSE_HACK
		NSLog(@"Left, notificating");
#endif
		[selectedItem mouseExited:theEvent];
		selectedItem = nil;
	}
	
	if(_resized)
	{
		nbColumn = [self updateNumberColumn];
#ifdef VERBOSE_HACK
		NSLog(@"Invalidated nbColumn, new number: %d", nbColumn);
#endif
		_resized = NO;
	}
	
	const uint nbElem = [_manager nbElement];
	if(nbElem == 0)
	{
#ifdef VERBOSE_HACK
		NSLog(@"No element, aborting");
#endif
		return;
	}
	
	int64_t rowMin = 0, rowMax = nbElem / nbColumn, currentRow = rowMax / 2;
	RakCollectionViewItem * item;
	
	//Every cell touch each other, so we must be over a cell
	while (rowMin <= rowMax)
	{
		item = (id) [self itemAtIndex:currentRow * nbColumn].view;
		if(item.frame.origin.y > pointInDocument.y)		//The goal is higher than this cell
		{
			rowMax = currentRow - 1;
#ifdef VERBOSE_HACK
			NSLog(@"Element %lld (row %lld) too high", currentRow * nbColumn, currentRow);
#endif
		}
		
		else if(NSMaxY(item.frame) > pointInDocument.y)	//We found the row
		{
#ifdef VERBOSE_HACK
			NSLog(@"Element %lld (row %lld) seems okay", currentRow * nbColumn, currentRow);
#endif
			break;
		}
		
		else											//The goal is lower than this cell
		{
			rowMin = currentRow + 1;
#ifdef VERBOSE_HACK
			NSLog(@"Element %lld (row %lld) too low", currentRow * nbColumn, currentRow);
#endif
		}
		
		currentRow = (rowMin + rowMax) / 2;
	}
	
	if(rowMin > rowMax)
	{
#ifdef VERBOSE_HACK
		NSLog(@"Invalid cursor position");
#endif
		return;
	}
	
	//The number of row can be pretty high and DB grow, but the number of columns is limited by the size of the screen.
	//Also, a big screen require significant horsepower, so we can settle to a more naive algorithm

	for(uint i = 0, index = currentRow * nbColumn; i < nbColumn && index < nbElem; i++, index++)
	{
		item = (id) [self itemAtIndex:index].view;

		if(item.frame.origin.x > pointInDocument.x)		//We went too far, this is an invalid clic
		{
#ifdef VERBOSE_HACK
			NSLog(@"Element %d (column %d) too far :/", index, i);
#endif
			return;
		}
		
		else if(NSMaxX(item.frame) > pointInDocument.x)	//Good element?
		{
#ifdef VERBOSE_HACK
			NSLog(@"Element %d (column %d) may be good", index, i);
#endif

			if([self validateItem:item :originalPoint])
			{
#ifdef VERBOSE_HACK
				NSLog(@"Yep, confirmed");
#endif
				selectedItem = item;
				[item mouseEntered:theEvent];
			}
			else	//In the view area, but out of the actual content
			{
#ifdef VERBOSE_HACK
				NSLog(@"And, nop, we're out of it");
#endif
				return;
			}
		}
	}
}

- (BOOL) validateItem : (RakCollectionViewItem *) item : (const NSPoint) originalPoint
{
	return NSPointInRect([item convertPoint:originalPoint fromView:nil], item.workingArea);
}

- (void) mouseExited:(NSEvent *)theEvent
{
	if(selectedItem != nil)
	{
		[selectedItem mouseExited:theEvent];
		selectedItem = nil;
	}
}


- (uint) updateNumberColumn
{
	NSView * scrollview = self.superview;
	
	while(scrollview != nil && ![scrollview isKindOfClass:[NSScrollView class]])
		scrollview = scrollview.superview;
	
	if(scrollview == nil)
		return UINT_MAX;
	
	const NSSize minimumSize = self.minItemSize;
	const NSRect documentFrame = ((NSScrollView *) scrollview).visibleRect;
	uint nbCol = MIN(floor(documentFrame.size.width / minimumSize.width), [_manager nbElement] - 1);
	
	if(nbCol == UINT_MAX || nbCol < 3)	//nbElement = 0, ou une seule ligne
		return nbCol;
	
	//We validate the number of columns
	NSView * first = [self itemAtIndex:0].view, *nextLine = [self itemAtIndex:nbCol].view, *lastOfFirstLine = [self itemAtIndex:nbCol - 1].view;
	if(first.frame.origin.y == lastOfFirstLine.frame.origin.y && first.frame.origin.y != nextLine.frame.origin.y)
		return nbCol;
	
	//We, we're messed up... Awesome
	
	//Too high
	if(first.frame.origin.y != lastOfFirstLine.frame.origin.y)
	{
		while (--nbCol > 1 && first.frame.origin.y != lastOfFirstLine.frame.origin.y)
		{
			lastOfFirstLine = [self itemAtIndex:nbCol - 1].view;
		}
	}
	else	//Too low
	{
		const uint nbElem = [_manager nbElement];
		while(++nbCol < nbElem && first.frame.origin.y == nextLine.frame.origin.y)
		{
			nextLine = [self itemAtIndex:nbCol].view;
		}
		
		if(nbCol == nbElem)
			nbCol--;	//Une seule ligne, mais une très longue ligne
	}

	return nbCol;
}



@end
