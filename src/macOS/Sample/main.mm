#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../../common/Sample/SampleApplication.h"

#include <SDL.h>
#include <SDL_syswm.h>

@interface MetalView : NSView

@property (nonatomic, assign) CAMetalLayer *metalLayer;

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

- (instancetype)initWithFrame:(CGRect)frame
{
    if ((self = [super initWithFrame:frame])) {
        self.wantsLayer = YES;
        _metalLayer = (CAMetalLayer *) self.layer;
        [self updateDrawableSize];
    }

    return self;
}

-(CALayer*) makeBackingLayer
{
    CAMetalLayer* layer = [self.class.layerClass layer];
    layer.opaque = YES;
    layer.contentsScale = [[NSScreen mainScreen] backingScaleFactor];
    layer.pixelFormat =  MTLPixelFormatBGRA8Unorm_sRGB;
    return layer;
}

- (void)updateDrawableSize
{
    _metalLayer.drawableSize = [self convertSizeToBacking:self.bounds.size];
}

@end

int main(int argc, const char * argv[])
{
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window *window = SDL_CreateWindow("Luch Engine Sample", 0, 0, 500, 500, SDL_WINDOW_RESIZABLE);

    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    SDL_GetWindowWMInfo(window, &info);

    NSView* sdlView = info.info.cocoa.window.contentView;
    MetalView *metalView = [[MetalView alloc] initWithFrame:sdlView.frame];
    CGSize size = metalView.metalLayer.drawableSize;
    [sdlView addSubview:metalView];

    auto app = Luch::MakeUnique<SampleApplication>();
    app->SetViewSize(static_cast<Luch::int32>(size.width), static_cast<Luch::int32>(size.height));
    app->SetView(metalView.metalLayer);
    [[maybe_unused]] bool result = app->Initialize({});
    LUCH_ASSERT(result);

    while(true)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            /* Handle SDL events. */
        }

        app->Process();
    }

    [metalView removeFromSuperview];

    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}
