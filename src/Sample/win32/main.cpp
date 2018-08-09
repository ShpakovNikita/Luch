#if _WIN32

#include "../common/SampleApplication.h"

using namespace Husky;

int32 main(int32 argc, char8** argv)
{
    SampleApplication application;

    Vector<String> args;
    args.reserve(argc);
    for (int32 i = 0; i < argc; i++)
    {
        args.push_back(argv[i]);
    }

    bool initialized = application.Initialize(args);
    HUSKY_ASSERT_MSG(initialized, "Application failed to initialize");

    application.Run();

    bool deinitialized = application.Deinitialize();
    HUSKY_ASSERT_MSG(deinitialized, "Application failed to deinitialize");

    return 0;
}

#endif
