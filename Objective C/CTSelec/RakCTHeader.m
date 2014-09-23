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

@implementation RakCTHeader

- (id) initWithData : (NSRect) frame : (PROJECT_DATA) data
{
	self = [self initWithFrame:[self frameByParent:frame]];
	
	if(self != nil)
	{
		_background = [[RakCTHImage alloc] initWithProject : self.bounds : data];
	}
	
	return self;
}

- (NSRect) frameByParent : (NSRect) parentFrame
{
	//We take half of the width, and the top 40% of the view
	parentFrame.size.width /= 2;
	parentFrame.origin.y = parentFrame.size.height * 2 / 5;
	parentFrame.size.height -= parentFrame.origin.y;

	return parentFrame;
}

@end
