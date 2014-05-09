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

@implementation RakProgressCircle

- (id)initWithRadius:(CGFloat) radius : (NSPoint) origin
{
	NSRect frame = NSMakeRect(origin.x, origin.y, radius * 2, radius * 2);
    self = [super initWithFrame:frame];
    
	if (self)
	{
		_radius = radius;
		_width = 1;
		
		_percentage = 0;
		_percText = [[RakText alloc] initWithText: frame : [NSString stringWithFormat:@"%.0f", _percentage] :[Prefs getSystemColor:GET_COLOR_INACTIVE]];
		
		[self addSubview:_percText];
		
		slotColor = [[Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_SLOT] retain];
		progressColor = [[Prefs getSystemColor:GET_COLOR_PROGRESSCIRCLE_PROGRESS] retain];
    }
    
	return self;
}

- (void) dealloc
{
	[slotColor release];
	[progressColor release];
	[_percText removeFromSuperview];	[_percText release];
	
	[super dealloc];
}

- (void) setWidth : (CGFloat) width
{
	_width = width;
}

- (void) setSlotColor : (NSColor *) color
{
	[slotColor release];
	slotColor = color;
	[slotColor retain];
}

- (void) setProgressColor : (NSColor *) color
{
	[progressColor release];
	progressColor = color;
	[progressColor retain];
}

- (RakText *) getText
{
	return _percText;
}

`{
	if(percentage < 0 || percentage > 100 || _percentage == percentage)
		return;
	
	_percentage = percentage;
	
	_percText.stringValue = [NSString stringWithFormat:@"%.0f", _percentage];
	[self centerText];
}

#pragma mark - Drawing

- (void) centerText
{
	[_percText sizeToFit];
	NSRect frame = _percText.frame;
	
	frame.origin.x = self.frame.size.width / 2 - frame.size.width / 2;
	frame.origin.y = self.frame.size.height / 2 - frame.size.height / 2;
	
	[_percText setFrameOrigin:frame.origin];
}

- (void) setupPath
{
	CGContextRef ctx = [[NSGraphicsContext currentContext] graphicsPort];
	CGFloat x = self.frame.size.width / 2, y = self.frame.size.height / 2;
	
	// "Full" Background Circle:
    CGContextBeginPath(ctx);
    CGContextAddArc(ctx, x, y, _radius, 0, 2*M_PI, 0);
    CGContextSetStrokeColorWithColor(ctx, slotColor.CGColor);
    CGContextStrokePath(ctx);
    
    // Progress Arc:
    CGContextBeginPath(ctx);
    CGContextAddArc(ctx, x, y, _radius, 0, 2 * M_PI * _percentage / 100, 0);
    CGContextSetStrokeColorWithColor(ctx, progressColor.CGColor);
    CGContextStrokePath(ctx);
}

- (void) drawRect:(NSRect)dirtyRect
{
	[self setupPath];
	
	[super drawRect:dirtyRect];
}

@end