#include <Husky/Metal/MetalSurface.h>
#import <MetalKit/MetalKit.h>

namespace Husky::Metal
{
    MetalSurface::MetalSurface(void* aView)
        : view(aView)
    {
        [(MTKView*)view retain];
    }

    MetalSurface::~MetalSurface()
    {
        [(MTKView*)view release];
    }
}
