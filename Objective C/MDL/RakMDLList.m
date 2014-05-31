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

@implementation RakMDLList

- (id) init : (NSRect) frame : (RakMDLController *) _controller
{
	if(_controller == nil)
		return nil;
	
	self = [super init];
	
	if(self != nil)
	{
		pause = [[RakButton allocForSeries : nil : @"pause" : NSMakePoint(0, 0) : nil : nil] retain];
		read = [[RakButton allocForSeries : nil : @"voir" : NSMakePoint(0, 0) : nil : nil] retain];
		remove = [[RakButton allocForSeries : nil : @"X" : NSMakePoint(0, 0) : nil : nil] retain];
		
		if(pause == nil || read == nil || remove == nil)
		{
			[self release];
			return nil;
		}
		else
		{
			[pause setBordered:YES];		[read setBordered:YES];			[remove setBordered:YES];
		}
		
		controller = _controller;
		cellHeight = MDLLIST_CELL_HEIGHT;
				
		[self applyContext:frame : -1 : -1];
		[_tableView setRowHeight:cellHeight];
		[scrollView setHasHorizontalScroller:NO];
	}
	
	return self;
}

- (void) needToQuit
{
	[controller needToQuit];
}

- (void) wakeUp
{
	[_tableView reloadData];
}

- (CGFloat) contentHeight
{
	if(scrollView == nil)
		return 0;
	
	CGFloat output = [controller getNbElem:YES] * (cellHeight + _tableView.intercellSpacing.height);
	
	if(output != 0)
		output += _tableView.intercellSpacing.height;
	
	return output;
}

- (void) resizeAnimation: (NSRect) frameRect
{
	[scrollView.animator setFrame:[self getTableViewFrame:frameRect]];
}

- (void) setScrollerHidden : (BOOL) hidden
{
	if(scrollView != nil)
		[scrollView setHasVerticalScroller:!hidden];
}

- (bool) didInitWentWell
{
	return true;
}

#pragma mark - Methods to deal with tableView

- (NSInteger)numberOfRowsInTableView:(NSTableView *)aTableView
{
	return [controller getNbElem : YES];
}

- (NSNumber*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= [controller getNbElem : YES])
		return nil;
	
	return @(rowIndex);
}

- (void) tableView:(NSTableView *)tableView didAddRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	DATA_LOADED ** todoList = [controller getData:row : YES];

	if(todoList != NULL && *todoList != NULL)
	{
		(*todoList)->rowViewResponsible = [tableView viewAtColumn:0 row:row makeIfNecessary:NO];
		[(RakMDLListView*) (*todoList)->rowViewResponsible updateContext];
	}
}

- (void) tableView:(NSTableView *)tableView didRemoveRowView:(NSTableRowView *)rowView forRow:(NSInteger)row
{
	if (row == -1)
		return;
	
	DATA_LOADED ** todoList = [controller getData:row : YES];

	if(todoList != NULL && *todoList != NULL)
	{
		(*todoList)->rowViewResponsible = NULL;
	}
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    // Get an existing cell with the MyView identifier if it exists
    RakMDLListView *result = [tableView makeViewWithIdentifier:@"Mane 6" owner:self];
	
    // There is no existing cell to reuse so create a new one
    if (result == nil)
	{
		// Create the new NSTextField with a frame of the {0,0} with the width of the table.
		// Note that the height of the frame is not really relevant, because the row height will modify the height.
		result = [[RakMDLListView alloc] init:_tableView.frame.size.width :cellHeight :pause :read :remove : controller : row];

		[result setFont:[NSFont fontWithName:[Prefs getFontName:GET_FONT_STANDARD] size:13]];
		
		result.identifier = @"Mane 6";
	}
	else
	{
		[result updateData : row : _tableView.frame.size.width];
	}
	
	// Return the result
	return result;
	
}

- (BOOL) tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row { return NO; }

#pragma mark - Drag'n Drop support

- (NSDragOperation)tableView:(NSTableView*)tv validateDrop:(id <NSDraggingInfo>)info proposedRow:(NSInteger)row proposedDropOperation:(NSTableViewDropOperation)op
{
	uint tab = [self getOwnerOfTV:[info draggingSource]];
	
	if(tab == GUI_THREAD_SERIES || tab == GUI_THREAD_CT)
		return NSDragOperationMove;

	return NSDragOperationNone;
}

@end
