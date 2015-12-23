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

int launchApp(int argc, char *argv[])
{
	@autoreleasepool
	{
		return UIApplicationMain(argc, argv, nil, NSStringFromClass([RakAppiOSDelegate class]));
	}
}

@implementation PDFDocument

- (instancetype) initWithData:(NSData *)data
{
	if(data == nil)
		return nil;
	
	self = [self init];
	if(self != nil)
	{
		CFDataRef myPDFData = (__bridge CFDataRef) data;
		CGDataProviderRef provider = CGDataProviderCreateWithCFData(myPDFData);
		if(provider != nil)
		{
			internalDocument = CGPDFDocumentCreateWithProvider(provider);
			CFRelease(provider);
		}
	}
	
	return self;
}

- (PDFPage *) pageAtIndex : (NSUInteger) page
{
	return [[PDFPage alloc] initWithPage:CGPDFDocumentGetPage(internalDocument, page)];
}

- (NSUInteger) pageCount
{
	return internalDocument == NULL ? 0 : CGPDFDocumentGetNumberOfPages(internalDocument);
}

- (void) dealloc
{
	CGPDFDocumentRelease(internalDocument);
}

@end

@implementation PDFPage

- (instancetype) initWithPage : (CGPDFPageRef) page
{
	self = [self init];
	
	if(self != nil)
		_page = page;
	
	return self;
}

@end

@implementation NSObject (Comparaison)

- (BOOL)isNotEqualTo:(id)object
{
	return object != nil && ![self isEqual : object];
}

@end

@implementation NSBundle (MacCompatibility)

- (RakImage *)imageForResource:(NSString *)name
{
	return nil;
}

@end

@implementation RakColor (MacCompatibility)

+ (RakColor *)colorWithDeviceWhite:(CGFloat)white alpha:(CGFloat)alpha
{
	return [RakColor colorWithWhite:white alpha:alpha];
}

+ (RakColor *)colorWithSRGBRed:(CGFloat)red green:(CGFloat)green blue:(CGFloat)blue alpha:(CGFloat)alpha
{
	return [RakColor colorWithRed:red green:green blue:blue alpha:alpha];
}

@end

@implementation RakView (Redirection)

- (void) setNeedsDisplay : (BOOL) value
{
	if(value == YES)
		[self setNeedsDisplay];
}

@end

@implementation RakTabView
@end

@implementation Series

- (void) removeFromSuperview
{
	
}

- (NSString *) byebye
{
	return STATE_EMPTY;
}

@end

@implementation CTSelec

- (void) removeFromSuperview
{
	
}

- (NSString *) byebye
{
	return STATE_EMPTY;
}

- (void) ownFocus
{
	
}

@end

@implementation Reader

- (void) removeFromSuperview
{
	
}

- (NSString *) byebye
{
	return STATE_EMPTY;
}

@end

@implementation MDL

- (void) removeFromSuperview
{
	
}

- (NSString *) byebye
{
	return STATE_EMPTY;
}

- (void) proxyAddElement : (PROJECT_DATA) data  isTome : (BOOL) isTome element : (uint) newElem  partOfBatch : (BOOL) partOfBatch
{
	
}

- (BOOL) proxyCheckForCollision : (PROJECT_DATA) data : (BOOL) isTome : (uint) element
{
	return NO;
}

- (NSRect) createFrame
{
	return [self lastFrame];
}

- (NSRect) lastFrame
{
	return NSZeroRect;
}

@end