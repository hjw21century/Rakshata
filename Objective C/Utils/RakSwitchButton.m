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

enum
{
	BORDER = 2,
	RADIUS_LARGE = 3,
	RADIUS_INTERNAL = 2
};

@interface RakSwitchButtonCell : RakCustomButtonCell
{
	NSColor * borderColor;
	NSColor *backgroundMixed, *backgroundOff, *backgroundOn;
	
	NSPoint basePoint, inflectionPoint, finishPoint;
}

@end

@implementation RakSwitchButton

+ (Class) cellClass
{
	return [RakSwitchButtonCell class];
}

@end

@implementation RakSwitchButtonCell

#pragma mark - Color management

- (void) initColors
{
	borderColor = [Prefs getSystemColor:GET_COLOR_BORDER_SWITCH_BUTTON :nil];
	backgroundMixed = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_MIXED :nil];
	backgroundOff = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_OFF :nil];
	backgroundOn = [Prefs getSystemColor:GET_COLOR_BACKGROUND_SWITCH_BUTTON_ON :nil];
}

#pragma mark - Drawing

#define RADIUS_CHK 0.75f

- (void) drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
	//Border
	[borderColor setFill];
	
	cellFrame = NSInsetRect(cellFrame, BORDER, BORDER);
	
	[[NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:RADIUS_LARGE yRadius:RADIUS_LARGE] fill];
	
	cellFrame = NSInsetRect(cellFrame, 0.5, 0.5);
	
	if(self.isHighlighted)
	{
		[backgroundMixed setFill];
	}
	else
	{
		if(self.state == NSOffState)
			[backgroundOff setFill];
		else
			[backgroundOn setFill];
	}
	
	[[NSBezierPath bezierPathWithRoundedRect:cellFrame xRadius:RADIUS_INTERNAL yRadius:RADIUS_INTERNAL] fill];
	
	if(!haveSizeConfig)
	{
		NSSize size = cellFrame.size;
		basePoint = 		NSMakePoint(cellFrame.origin.x + size.width / 2 - 3.5, 	cellFrame.origin.y + size.height / 2);
		inflectionPoint = 	NSMakePoint(cellFrame.origin.x + size.width / 2 - 1, 	cellFrame.origin.y + size.height / 2 + 3);
		finishPoint = 		NSMakePoint(cellFrame.origin.x + size.width / 2 + 3.5, 	cellFrame.origin.y + size.height / 2 - 3.5);
		haveSizeConfig = YES;
	}
	
	//Checkmark drawing
	[[NSColor whiteColor] setFill];
	
	if(_animation != nil)
	{
		uint stage = _animation.stage;
		
		if(self.state != NSOnState)
			stage = _animation.animationFrame - 1 - stage;
		
		CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
		
		CGContextBeginPath(contextBorder);
		CGContextAddArc(contextBorder, basePoint.x, basePoint.y, RADIUS_CHK, M_PI * 0.835, -M_PI * 0.165, 0);
		
		const CGPoint currentPos = CGContextGetPathCurrentPoint(contextBorder);
		
		//Before the inflexion
		if(stage < 3)
		{
			CGPoint newPoint = currentPos;
			
			//We draw the progress line
			newPoint.x += stage * ((inflectionPoint.x - basePoint.x) / 3.0f);
			newPoint.y += stage * ((inflectionPoint.y - basePoint.y) / 3.0f);
			
			CGContextAddLineToPoint(contextBorder, newPoint.x, newPoint.y);
			
			//Roundish end of line
			newPoint.x -= currentPos.x - basePoint.x;
			newPoint.y -= currentPos.y - basePoint.y;
			
			CGContextAddArc(contextBorder, newPoint.x, newPoint.y, RADIUS_CHK, -M_PI * 0.165, M_PI * 0.835, 0);
		}
		else
		{
			//Full line
			CGContextAddLineToPoint(contextBorder, inflectionPoint.x, inflectionPoint.y - 1);
			
			if(stage > 4)
			{
				CGPoint newPoint = CGContextGetPathCurrentPoint(contextBorder);
				stage -= 3;
				
				//We draw the progress line
				newPoint.x += stage * ((finishPoint.x - inflectionPoint.x) / 6.0f);
				newPoint.y += stage * ((finishPoint.y - inflectionPoint.y) / 6.0f);
				
				CGContextAddLineToPoint(contextBorder, newPoint.x, newPoint.y);
				
				//Roundish end of line
				newPoint.x += currentPos.x - basePoint.x;
				newPoint.y -= currentPos.y - basePoint.y;
				
				CGContextAddArc(contextBorder, newPoint.x, newPoint.y, RADIUS_CHK, -M_PI * 0.828, M_PI * 0.172, 0);
				CGContextAddArc(contextBorder, inflectionPoint.x, inflectionPoint.y, RADIUS_CHK, M_PI * 0.172, M_PI * 0.835, 0);
			}
			else
			{
				CGContextAddArc(contextBorder, inflectionPoint.x, inflectionPoint.y, RADIUS_CHK, -M_PI_2, M_PI * 0.835, 0);
			}
		}
		
		CGContextFillPath(contextBorder);
	}
	else if(self.state == NSOnState)
	{
		CGContextRef contextBorder = [[NSGraphicsContext currentContext] graphicsPort];
		
		CGContextBeginPath(contextBorder);
		
		CGContextAddArc(contextBorder, basePoint.x, basePoint.y, RADIUS_CHK, M_PI * 0.835, -M_PI * 0.165, 0);
		CGContextAddLineToPoint(contextBorder, inflectionPoint.x, inflectionPoint.y - 1);
		CGContextAddArc(contextBorder, finishPoint.x, finishPoint.y, RADIUS_CHK, -M_PI * 0.828, M_PI * 0.172, 0);
		CGContextAddArc(contextBorder, inflectionPoint.x, inflectionPoint.y, RADIUS_CHK, M_PI * 0.172, M_PI * 0.835, 0);
		
		CGContextFillPath(contextBorder);
	}
	else if(self.state == NSMixedState)
	{
		NSSize size = cellFrame.size;
		NSRect frame;
		
		frame.size.height = 2;
		frame.size.width = size.width * 3 / 5;
		
		frame.origin = cellFrame.origin;
		frame.origin.x += size.width / 5;
		frame.origin.y += size.height / 2 - frame.size.height / 2;
		
		[[NSBezierPath bezierPathWithRoundedRect:frame xRadius:2 yRadius:2] fill];
	}
}

@end
