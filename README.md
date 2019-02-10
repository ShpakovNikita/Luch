# Luch

Luch means ray in Russian

Luch is my ongoing 3D graphics engine playground project.

Right now only has deferred pipeline.

## Features
* Abstract Rendering API
* Metal Backend
* Deferred Pipeline
* Tiled Deferred Pipeline (on iOS devices with A11+)
* Forward Pipeline (no Forward+ yet)
* Compute
* Loading glTF 2.0 scenes
* Punctual Lights
* Render graph (graphics and compute passes)
* iOS Support (kinda)

Supports loading glTF 2.0 static scenes (no keyframe animations, skeletal animations and morphing).

Supports punctual lights: directional, point and spot.

## Work in Progress
* Tile-Based Deferred Rendering
* Vulkan Backend (thanks to @mlknz)
* Shadow mapping

## Planned
In no particular order:
* Light probes (image-based lighting)
* Forward+ (tiled forward, forward with light culling)
* Area lights
* Keyframe Animations
* Skeletal Animations
* Shader reloading

## Compiling
## #macOS

```
mkdir build
cd build
cmake -G Xcode -DUSE_METAL=true -DPLATFORM=macOS ..
```

### iOS

```
mkdir build
cd build
cmake -G Xcode -DUSE_METAL=true -DPLATFORM=iOS -DIOS_PLATFORM=OS64 -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DIOS_DEPLOYMENT_TARGET=12.0 -DIOS_ARCH=arm64 -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake ..
```
