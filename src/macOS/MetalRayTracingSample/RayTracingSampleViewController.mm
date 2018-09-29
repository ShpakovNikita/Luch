#import <QuartzCore/CAMetalLayer.h>
#include "RayTracingSampleViewController.h"
#include "RayTracingSampleApplication.h"

@implementation RayTracingSampleViewController
{
    MTKView* _view;
    RayTracingSampleApplication* app;
}

-(void) dealloc
{
    delete app;
    [super dealloc];
}

-(void) viewDidLoad
{
    [super viewDidLoad];

    _view = (MTKView*)self.view;

    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceLinearSRGB);
    _view.colorspace = colorSpace;
    CGColorSpaceRelease(colorSpace);

    // Lookup high power GPU if this is a discrete GPU system
    NSArray<id <MTLDevice>> *devices = MTLCopyAllDevices();

    id <MTLDevice> device = devices[0];

    for (id <MTLDevice> potentialDevice in devices) {
        if (!potentialDevice.lowPower) {
            device = potentialDevice;
            break;
        }
    }

    _view.device = device;

    app = new RayTracingSampleApplication();

    //NSRect bounds = self.view.bounds;
    //app->SetViewSize(static_cast<Husky::int32>(bounds.size.width), static_cast<Husky::int32>(bounds.size.height));
    //app->SetView(self.view);
    bool result = app->Initialize({});
}


@end
