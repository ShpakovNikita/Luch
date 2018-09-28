#import <QuartzCore/CAMetalLayer.h>
#include "SampleViewController.h"
#include "../../common/Sample/SampleApplication.h"

#pragma mark -
#pragma mark SampleViewController

@implementation SampleViewController
{
    CVDisplayLinkRef displayLink;
    SampleApplication* app;
}

-(void) dealloc
{
    CVDisplayLinkRelease(displayLink);
    delete app;
    [super dealloc];
}

-(void) viewDidLoad
{
    [super viewDidLoad];

    self.view.wantsLayer = YES;

    NSRect bounds = self.view.bounds;

    app = new SampleApplication();
    app->SetViewSize(static_cast<Husky::int32>(bounds.size.width), static_cast<Husky::int32>(bounds.size.height));
    app->SetView(self.view.layer);
    bool result = app->Initialize({});
    HUSKY_ASSERT(result);

    CVDisplayLinkCreateWithActiveCGDisplays(&displayLink);
    CVDisplayLinkSetOutputCallback(displayLink, &DisplayLinkCallback, app);
    CVDisplayLinkStart(displayLink);
}


#pragma mark Display loop callback function

/** Rendering loop callback function for use with a CVDisplayLink. */
static CVReturn DisplayLinkCallback(CVDisplayLinkRef displayLink,
    const CVTimeStamp* now,
    const CVTimeStamp* outputTime,
    CVOptionFlags flagsIn,
    CVOptionFlags* flagsOut,
    void* target)
{
    SampleApplication* application = static_cast<SampleApplication*>(target);
    if(application)
    {
        application->Process();
    }
	return kCVReturnSuccess;
}

@end


#pragma mark -
#pragma mark DemoView

@implementation SampleView

/** Indicates that the view wants to draw using the backing layer instead of using drawRect:.  */
-(BOOL) wantsUpdateLayer
{
    return YES;
}

+(Class) layerClass
{
    return [CAMetalLayer class];
}

-(CALayer*) makeBackingLayer
{
    CALayer* layer = [self.class.layerClass layer];
    CGSize viewScale = [self convertSizeToBacking: CGSizeMake(1.0, 1.0)];
    layer.contentsScale = MIN(viewScale.width, viewScale.height);
    return layer;
}

@end
