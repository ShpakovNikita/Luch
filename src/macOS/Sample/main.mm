#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../../common/Sample/SampleApplication.h"

#include <SDL.h>
#include <SDL_syswm.h>

@interface MetalView : NSView

@property (nonatomic) CAMetalLayer *metalLayer;

@end

@implementation MetalView

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
    CAMetalLayer* layer = [self.class.layerClass layer];
    layer.device = MTLCreateSystemDefaultDevice();
    layer.opaque = YES;
    layer.contentsScale = [[NSScreen mainScreen] backingScaleFactor];
    layer.pixelFormat =  MTLPixelFormatBGRA8Unorm_sRGB;
    _metalLayer = layer;
    return layer;
}

@end

int main(int argc, const char * argv[])
{
    SDL_InitSubSystem(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("", 0, 0, 480, 320, SDL_WINDOW_RESIZABLE);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);

    // NSView *sdlView = info.info.;

    // MetalView *metalView = [[MetalView alloc] initWithFrame:sdlView.frame];
    // [sdlView addSubview:metalView];

    // NSRect bounds = metalView.bounds;
    // CGSize backingSize = [metalView convertSizeToBacking:bounds.size];

    // app = new SampleApplication();
    // app->SetViewSize(static_cast<Luch::int32>(backingSize.width), static_cast<Luch::int32>(backingSize.height));
    // app->SetView(metalView);
    // [[maybe_unused]] bool result = app->Initialize({});
    // LUCH_ASSERT(result);

    while(true)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            /* Handle SDL events. */
        }

        // application->Process();
    }

    //[metalview removeFromSuperview];

    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
