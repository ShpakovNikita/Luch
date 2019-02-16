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
* Light probes (image-based lighting)
* Vulkan Backend (thanks to @mlknz)
* Shadow mapping

## Planned
In no particular order:
* Forward+ (tiled forward, forward with light culling)
* Area lights
* Keyframe Animations
* Skeletal Animations
* Shader reloading

## Compiling and Running

Generate a project (with commands that are listed below) and then open it in Xcode and run

### macOS

```
mkdir build
cd build
cmake -G Xcode -DUSE_METAL=true -DPLATFORM=macOS ..
```

When the application is running, you can use WASD to control camera position.
To control camera view direction, click and drag.

You can switch between different rendering pipelines:

| Key | Action |
| --- | --- |
| 1 | forward rendering |
| 2 | deferred rendering |
| 3 | deferred rendering with compute resolve |

Also

| Key | Action |
| --- | --- |
| Z | turn on depth prepass |
| X | turn off depth prepass |

### iOS

```
mkdir build
cd build
cmake -G Xcode -DUSE_METAL=true -DPLATFORM=iOS -DIOS_PLATFORM=OS64 -DENABLE_ARC=0 -DENABLE_BITCODE=0 -DIOS_DEPLOYMENT_TARGET=12.0 -DIOS_ARCH=arm64 -DCMAKE_TOOLCHAIN_FILE=../ios.toolchain.cmake ..
```
