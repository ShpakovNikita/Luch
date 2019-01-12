#if __linux__
#include "../../common/Sample/SampleApplication.h"
#include <memory>
#include <vector>

#include <SDL2/SDL.h>

using namespace Luch;

int32 main(int32 argc, char8** argv)
{
    Vector<String> args;
    args.reserve(argc);
    for (int32 i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }

    auto app = std::make_unique<SampleApplication>();

    bool initialized = app->Initialize(args);
    if (!initialized)
    {
        LUCH_ASSERT_MSG(false, "Application failed to initialize");
        return EXIT_FAILURE;
    }

    while (true)
    {
        SDL_Event evt;
        while (SDL_PollEvent(&evt))
        {
            app->HandleEvent(evt);
        }
        if (app->ShouldQuit())
        {
            break;
        }
        else
        {
            app->Process();
        }
    }

    bool deinitialized = app->Deinitialize();
    app.reset();
    LUCH_ASSERT_MSG(deinitialized, "Application failed to deinitialize");

    return EXIT_SUCCESS;
}
#endif // __linux__
