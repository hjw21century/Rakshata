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

#pragma mark	-	Reader

/** Reader itself **/

#define READER_BORDURE_VERT_PAGE 10

/** Constants used by pages **/

#define READER_PAGE_TOP_BORDER		50
#define READER_PAGE_BOTTOM_BORDER	(RD_CONTROLBAR_HEIGHT + RD_CONTROLBAR_POSY + 10)
#define READER_PAGE_BORDERS_HIGH (READER_PAGE_TOP_BORDER + READER_PAGE_BOTTOM_BORDER)

/** Bottom reader bar **/

#define RD_CONTROLBAR_HEIGHT		26
#define RD_CONTROLBAR_WIDHT_PERC	75
#define RD_CONTROLBAR_WIDHT_MIN		500
#define RD_CONTROLBAR_WIDHT_MAX		1000
#define RD_CONTROLBAR_POSY			20

/**	CT itself	**/

/** RakChapterView **/
#define CT_READERMODE_LATERAL_BORDER 5			//	%
#define CT_READERMODE_BOTTOMBAR_WIDTH	14
#define CT_READERMODE_WIDTH_PROJECT_NAME	25
#define CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE 20
#define CT_READERMODE_HEIGHT_PROJECT_IMAGE	150
#define CT_READERMODE_HEIGHT_HEADER_TAB (CT_READERMODE_WIDTH_PROJECT_NAME + 2 * CT_READERMODE_WIDTH_SPACE_NEXT_PROJECTIMAGE + CT_READERMODE_HEIGHT_PROJECT_IMAGE)
#define CT_READERMODE_HEIGHT_CT_BUTTON	25
#define CT_READERMODE_HEIGHT_BORDER_TABLEVIEW 10
#define CT_READERMODE_BORDER_TABLEVIEW	10

/** Serie itself **/

#define SR_PREF_BUTTON_BORDERS 25
#define SR_HEADER_INTERBUTTON_WIDTH 10

/** RakSerieView **/
#define SR_READERMODE_LATERAL_BORDER 5			//	%

/** RakSerieSubmenu **/
#define SR_READERMODE_MARGIN_ELEMENT_OUTLINE 8
#define SR_READERMODE_MARGIN_OUTLINE		7
#define SR_READERMODE_LBWIDTH_OUTLINE		10	//LB = Line Break
#define SR_READERMODE_ILBWIDTH_OUTLINE		13	//ILB = Intermediary LB

/** MDL itself **/

/** RakMDLView **/
#define MDL_READERMODE_LATERAL_BORDER 5			//	%
#define MDL_READERMODE_BOTTOMBAR_WIDTH	14

 
/**		Utils	**/

/*		RakBackButton		*/
#define RBB_BUTTON_HEIGHT	25
#define RBB_BUTTON_WIDTH	75						//	%
#define RBB_BUTTON_POSX		12.5f					//	%
#define RBB_TOP_BORDURE		10