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

#import "RakSRContentManager.h"

enum
{
	CELL_INTERTEXT_OFFSET = 2,
	CELL_BULLET_OFFSET = 8,
	CELL_BULLET_WIDTH = 12,
	CELL_IMAGE_OFFSET = 15,
	CELL_BULLET_FULL_WIDTH = 45
};

@interface Series : RakTabView <UITableViewDataSource, UITableViewDelegate>
{
	IBOutlet UITableView * _tableView;
	IBOutlet UINavigationBar * _navigationBar;
	
	RakSRContentManager * contentManager;
}

@property BOOL shouldDisplaySource;

@end