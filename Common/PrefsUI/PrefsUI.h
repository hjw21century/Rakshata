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
 **			file, You can obtain one at http://mozilla.org/MPL/2.0/.						**
 **                                                                                         **
 **                     			© Taiki 2011-2016                                       **
 **                                                                                         **
 *********************************************************************************************/

//Allow to create NSViewController without NIB
@interface RakPrefsPopover : NSViewController
{
	RakView * mainView;
	
	uint mainThread;
	
	NSMutableArray * textFields;
	
	//UI Elements
	//	Reader
	
	RakSwitchButton * forcePDFBackground, * saveMagnification, *overrideDirection;
}

@property (nonatomic, weak) NSPopover * popover;

- (instancetype) initWithFrame : (NSRect) frame;
- (void) popoverClosed;

@end

@interface PrefsUI : RakView <NSPopoverDelegate>
{
    RakPrefsPopover *viewControllerHUD;
    NSPopover *popover;

	NSButton *anchor;
}

- (void) showPopover;
- (void) setAnchor : (NSButton *) newAnchor;

@end
