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
 ********************************************************************************************/

@implementation CTSelec

- (id)init : (NSView*)contentView : (NSString *) state
{
    self = [super init];
    if (self)
	{
		flag = GUI_THREAD_CT;
		self = [self initView:contentView : state];
		
		self.layer.borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_TABS].CGColor;
		self.layer.borderWidth = 2;
		
		backButton = [[RakBackButton alloc] initWithFrame:[self bounds]: true];
		[backButton setTarget:self];
		[backButton setAction:@selector(backButtonClicked)];
		[backButton setHidden:!readerMode];
		[self addSubview:backButton];
		[backButton release];

		if(state != nil && [state isNotEqualTo:STATE_EMPTY])
		{
			NSArray *componentsWithSpaces = [state componentsSeparatedByCharactersInSet:[NSCharacterSet newlineCharacterSet]];
			NSArray *dataState = [componentsWithSpaces filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"length > 0"]];
			
			if([dataState count] == 7)
			{
				do
				{
					uint nbElem, indexTeam;
					
					//We first get the index of the team, to perform a search in the DB
					const TEAMS_DATA **tmpData = getDirectAccessToKnownTeams(&nbElem);
					
					if(tmpData == NULL || nbElem == 0)
						break;
					
					const char * URLRepo = [[dataState objectAtIndex:0] cStringUsingEncoding:NSASCIIStringEncoding];
					
					for (indexTeam = 0; indexTeam < nbElem; indexTeam++)
					{
						if(tmpData[indexTeam] != NULL && !strcmp(tmpData[indexTeam]->URL_depot, URLRepo))
							break;
					}
					
					if(indexTeam == nbElem)
					{
						NSLog(@"Couldn't find the repo to restore, abort :/");
						break;
					}
					
					//We have a valid index, now, let's query the database to get the project
					
					const char * mangaNameCourt = [[dataState objectAtIndex:1] cStringUsingEncoding:NSASCIIStringEncoding];
					
					MANGAS_DATA * project = getDataFromSearch (indexTeam, mangaNameCourt, RDB_CTXCT, false);
					
					if(project == NULL)
					{
						NSLog(@"Couldn't find the project to restore, abort :/");
						break;
					}
					else
					{
						checkChapitreValable(project, NULL);
						checkTomeValable(project, NULL);
					}
					
					//Perfect! now, all we have to do is to sanitize last few data :D
					bool isTome = [[dataState objectAtIndex:2] boolValue] != 0;
					long context[4];
					context[0] = [[dataState objectAtIndex:3] floatValue];		//elemSelectedChapter
					context[1] = [[dataState objectAtIndex:4] floatValue];		//scrollerPosChapter
					context[2] = [[dataState objectAtIndex:5] floatValue];		//elemSelectedVolume
					context[3] = [[dataState objectAtIndex:6] floatValue];		//scrollerPosVolume
						
					coreView = [[RakChapterView alloc] initContent:[self calculateContentViewSize] : *project : isTome : context];
					free(project);
					
				} while (0);
			}
		}
		else
		{
			[self noContent];
		}
		
		if(coreView != nil)
		{
			[self addSubview:coreView];
			[coreView release];
		}
	}
    return self;
}

- (void) noContent
{
	long context[4] = {-1, -1, -1, -1};
	MANGAS_DATA *mangaData = getCopyCache(RDB_LOADALL | SORT_NAME, NULL);	//17 = Fairy tail
	coreView = [[RakChapterView alloc] initContent:[self calculateContentViewSize] : mangaData[21] : false : context];
}

- (void) dealloc
{
	[backButton removeFromSuperview];
	[backButton release];
	
	[coreView removeFromSuperview];
	[coreView release];
	
	[super dealloc];
}

- (void) backButtonClicked
{
	noDrag = true;
	[self mouseUp:NULL];
}

- (int) getCodePref : (int) request
{
	int output;
	
	switch (request)
	{
		case CONVERT_CODE_POSX:
		{
			output = PREFS_GET_TAB_CT_POSX;
			break;
		}
			
		case CONVERT_CODE_POSY:
		{
			output = PREFS_GET_TAB_CT_POSY;
			break;
		}
			
		case CONVERT_CODE_HEIGHT:
		{
			output = PREFS_GET_TAB_CT_HEIGHT;
			break;
		}
			
		case CONVERT_CODE_WIDTH:
		{
			output = PREFS_GET_TAB_CT_WIDTH;
			break;
		}
			
		case CONVERT_CODE_FRAME:
		{
			output = PREFS_GET_TAB_CT_FRAME;
			break;
		}
		
		default:
			output = 0;
	}
	
	return output;
}

- (void) setUpViewForAnimation : (BOOL) reader
{
	[backButton setHidden:!reader];
}

- (NSString *) byebye
{
	NSString * string;
	if(coreView == nil || (string = [coreView getContextToGTFO]) == nil)
	{
		return [super byebye];
	}
	else
		[self removeFromSuperview];
	   
	return string;
}

#pragma mark - Reader code
/**		Reader		**/

- (BOOL) isStillCollapsedReaderTab
{
	int state;
	[Prefs getPref:PREFS_GET_READER_TABS_STATE :&state];
	return (state & STATE_READER_TAB_CT_FOCUS) == 0;
}

- (NSRect) generateNSTrackingAreaSize : (NSRect) viewFrame
{
	CGFloat posCT, posReader;
	NSRect frame = viewFrame;
	[Prefs getPref:PREFS_GET_TAB_CT_POSX :&posCT];
	[Prefs getPref:PREFS_GET_TAB_READER_POSX :&posReader];
	frame.size.width = (posReader - posCT) * self.superview.frame.size.width / 100;
	frame.origin.x = frame.origin.y = 0;
	return frame;
}

#pragma mark - Self code used in reader mode

- (NSRect) calculateContentViewSize
{
	NSRect frame = [self frame];

	frame.size.height -= 2 * (frame.size.height - backButton.frame.origin.y) - backButton.frame.size.height + CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.origin.x = CT_READERMODE_LATERAL_BORDER * frame.size.width / 100.0f;
	frame.origin.y = CT_READERMODE_BOTTOMBAR_WIDTH;
	frame.size.width -= 2* frame.origin.x;	//Pas obligé de recalculer

	return frame;
}

- (void) setFrame:(NSRect)frameRect
{
	[super setFrame:frameRect];
	[backButton setFrame:[self bounds]];
	[coreView setFrame:[self calculateContentViewSize]];
}

- (NSRect) getFrameOfNextTab
{
	NSRect output;
	[Prefs getPref:PREFS_GET_TAB_READER_FRAME :&output];
	
	NSSize sizeSuperView = [self.superview frame].size;
	
	output.origin.x *= sizeSuperView.width / 100.0f;
	output.origin.y *= sizeSuperView.height / 100.0f;
	output.size.width *= sizeSuperView.width / 100.0f;
	output.size.height *= sizeSuperView.height / 100.0f;
	
	return output;
}

#pragma mark - Communication with other tabs

- (void) updateContextNotification:(MANGAS_DATA)project :(BOOL)isTome :(int)element
{
	if(element == VALEUR_FIN_STRUCTURE_CHAPITRE && project.team != NULL)
	{
		NSView * superview = self.superview;
		Reader *readerTab = nil;
		MDL * MDLTab = nil;
		id elem;
		
		for(NSInteger i = 0; i < [superview.subviews count]; i++)
		{
			if([(elem = [superview.subviews objectAtIndex:i]) class] == [Reader class])
				readerTab = elem;
			else if([elem class] == [MDL class])
			{
				MDLTab = elem;
				break;
			}
		}
		
		if(readerTab != nil)
		{
			__block NSRect readerFrame = readerTab.frame, MDLFrame = MDLTab.frame;
			CGFloat widthCTTab = readerFrame.origin.x - self.frame.origin.x;
			
			readerFrame.origin.x -= widthCTTab;
			readerFrame.size.width += widthCTTab;
			MDLFrame.size.width -= widthCTTab;
			
			[NSAnimationContext runAnimationGroup:^(NSAnimationContext *context) {
				[context setDuration:0.1];
				
				[readerTab.animator setFrame:readerFrame];
				
				if(MDLTab != nil)
					[MDLTab.animator setFrame:MDLFrame];

			} completionHandler:^{
				
				[coreView updateContext:project];
				if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
					[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
				
				else	//Fuck, we have to cancel the animation...
				{
					readerFrame.origin.x += widthCTTab;
					readerFrame.size.width -= widthCTTab;
					MDLFrame.size.width += widthCTTab;
					
					if(readerTab != nil)
						[readerTab setFrame:readerFrame];
					
					if(MDLTab != nil)
						[MDLTab setFrame:MDLFrame];
				}
			}];
			
		}
		else	//We bypass the fancy animation
		{
			[coreView updateContext:project];
			if([Prefs setPref:PREFS_SET_READER_TABS_STATE_FROM_CALLER :flag])
				[self refreshLevelViews : [self superview] : REFRESHVIEWS_CHANGE_READER_TAB];
		}
	}
}

@end
