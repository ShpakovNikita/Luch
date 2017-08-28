#include "SampleApplication.h"

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

    application.Initialize(args);
    application.Run();
    application.Deinitialize();

    return 0;
}
