#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../../common/Sample/SampleApplication.h"

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_events.h>
#include <SDL_main.h>

int main(int argc, char * argv[])
{
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    SDL_Window *window = SDL_CreateWindow("Luch Engine Sample", 0, 0, 500, 500, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

    int drawableWidth = 0;
    int drawableHeight = 0;
    SDL_RendererInfo rendererInfo;
    SDL_GetRendererInfo(renderer, &rendererInfo);
    SDL_GetRendererOutputSize(renderer, &drawableWidth, &drawableHeight);

    CAMetalLayer *metalLayer = (__bridge CAMetalLayer *)SDL_RenderGetMetalLayer(renderer);
    metalLayer.opaque = YES;
    metalLayer.contentsScale = 2.0f;
    metalLayer.pixelFormat =  MTLPixelFormatBGRA8Unorm_sRGB;
    metalLayer.drawableSize = CGSize{(float)drawableWidth, (float)drawableHeight};

    CGSize size = metalLayer.drawableSize;

    auto app = Luch::MakeUnique<SampleApplication>();
    app->SetViewSize(static_cast<Luch::int32>(size.width), static_cast<Luch::int32>(size.height));
    app->SetView((__bridge void*)metalLayer);
    [[maybe_unused]] bool initialized = app->Initialize({});
    LUCH_ASSERT(initialized);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while(true)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            app->HandleEvent(e);
        }

        if(app->ShouldQuit())
        {
            break;
        }
        else
        {
            app->Process();
        }
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

    [[maybe_unused]] bool deinitialized = app->Deinitialize();
    LUCH_ASSERT(deinitialized);
    app.reset();

    return 0;
}
