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

@implementation RakCTCoreContentView

#pragma mark - Classical initialization

- (id) init : (NSRect) frame : (MANGAS_DATA) project : (bool) isTomeRequest : (long) elemSelected : (long) scrollerPosition
{
	self = [super init];

	if(self != nil)
	{
		NSInteger row = -1, tmpRow = 0;

		//We check we have valid data
		isTome = isTomeRequest;
		
		if(isTome && project.tomes != NULL)
		{
			amountData = project.nombreTomes;
			data = malloc(amountData * sizeof(META_TOME));
			
			if(data != NULL)
			{
				memcpy(data, project.tomes, amountData * sizeof(META_TOME));
				if(elemSelected != -1)
				{
					for(; tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID < elemSelected; tmpRow++);

					if(tmpRow < amountData && ((META_TOME*)data)[tmpRow].ID == elemSelected)
						row = tmpRow;
				}
			}
		}
		else if(!isTome && project.chapitres != NULL)
		{
			amountData = project.nombreChapitre;
			data = malloc(amountData * sizeof(int));
			
			if(data != NULL)
			{
				memcpy(data, project.chapitres, amountData * sizeof(int));
				if(elemSelected != -1)
				{
					for(; tmpRow < amountData && ((int*)data)[tmpRow] < elemSelected; tmpRow++);
					
					if(tmpRow < amountData && ((int*)data)[tmpRow] == elemSelected)
						row = tmpRow;
				}
			}
		}
		
		[self applyContext:frame :row :scrollerPosition];
	}
	return self;
}

- (bool) didInitWentWell
{
	return data != NULL;
}

- (bool) reloadData : (int) nbElem : (void *) newData
{
	void * newDataBuf = NULL;
	
	if(isTome)
	{
		newDataBuf = malloc(nbElem * sizeof(META_TOME));
		if(newDataBuf != NULL)
		{
			memcpy(newDataBuf, newData, nbElem * sizeof(META_TOME));
		}
	}
	else
	{
		newDataBuf = malloc(nbElem * sizeof(int));
		if(newDataBuf != NULL)
		{
			memcpy(newDataBuf, newData, nbElem * sizeof(int));
		}
	}

	
	if(newDataBuf == NULL)
		return false;
		
	free(data);
	data = newDataBuf;
	
	return true;
}

- (NSRect) getTableViewFrame : (NSRect) superViewFrame
{
	NSRect frame = superViewFrame;
	
	frame.origin.x = CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.width -= 2 * CT_READERMODE_BORDER_TABLEVIEW;
	frame.size.height -= CT_READERMODE_HEIGHT_CT_BUTTON + CT_READERMODE_HEIGHT_BORDER_TABLEVIEW;
	
	return frame;
}

#pragma mark - Backup routine

- (NSInteger) getSelectedElement
{
	NSInteger row = [_tableView selectedRow];
	
	if(row < 0 || row > amountData)
		return -1;
	
	if(isTome)
		return ((META_TOME *) data)[row].ID;
	else
		return ((int *) data)[row];
}

#pragma mark - Methods to deal with tableView

- (NSString*) tableView:(NSTableView *)aTableView objectValueForTableColumn:(NSTableColumn *)aTableColumn row:(NSInteger)rowIndex
{
	if(rowIndex >= amountData)
		return nil;
	
	NSString * output;
	
	if(isTome)
	{
		META_TOME element = ((META_TOME *) data)[rowIndex];
		if(element.name[0])
			output = [NSString stringWithUTF8String:(char*)element.name];
		else
			output = [NSString stringWithFormat:@"Tome %d", element.ID];
		
	}
	else
	{
		int ID = ((int *) data)[rowIndex];
		if(ID % 10)
			output = [NSString stringWithFormat:@"Chapitre %d.%d", ID / 10, ID % 10];
		else
			output = [NSString stringWithFormat:@"Chapitre %d", ID / 10];
	}
	
	return output;
}

#pragma mark - Get result from NSTableView

- (void)tableViewSelectionDidChange:(NSNotification *)notification;
{
	uint result = [_tableView selectedRow];
	
	if(result < amountData)
	{
		[(RakCTContentTabView*) scrollView.superview gotClickedTransmitData: isTome : result];
	}
}

@end