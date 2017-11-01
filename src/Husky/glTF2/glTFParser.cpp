#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Stream.h>
#include <json.hpp>

namespace Husky::glTF
{

using json = nlohmann::json;

Map<String, AttributeType> attributeTypeLookup =
{
    {"SCALAR", AttributeType::Scalar},
    {"VEC2", AttributeType::Vec2},
    {"VEC3", AttributeType::Vec3},
    {"VEC4", AttributeType::Vec4},
    {"MAT2", AttributeType::Mat2x2},
    {"MAT3", AttributeType::Mat3x3},
    {"MAT4", AttributeType::Mat4x4},
};

Map<String, AnimationInterpolation> animationInterpolationLookup =
{
    { "LINEAR", AnimationInterpolation::Linear },
    { "STEP", AnimationInterpolation::Step },
    { "CATMULLROMSPLINE", AnimationInterpolation::CatmullRomSpline},
    { "CUBICSPLINE", AnimationInterpolation::CubicSpline },
};

Map<String, CameraType> cameraTypeLookup = 
{
    {"perspective", CameraType::Perspective},
    {"orthographic", CameraType::Orthographic}
};

template<typename T, T(*Function)(const json&)>
Vector<T> ParseArray(const json& j)
{
    Vector<T> result;
    result.reserve(j.size());

    for (auto entry : j)
    {
        result.emplace_back(Function(entry));
    }

    return result;
}

template<typename T>
T ParseBuiltin(const json& j)
{
    return j.get<T>();
}

template<typename T>
Vector<T> ParseBuiltinArray(const json& j)
{
    return ParseArray<T, ParseBuiltin<T>>(j);
}

Sparse ParseSparse(const json& j)
{
    Sparse sparse;
    return sparse;
}

AccessorValueHolder ParseAccessorMinMax(ComponentType componentType, AttributeType type, const json& j)
{
    AccessorValueHolder valueHolder;
    return valueHolder;
}

Accessor ParseAccessor(const json& j)
{
    Accessor accessor;
    accessor.bufferView = j.value<Optional<int32>>("bufferView", {});
    accessor.bufferOffset = j.value<int32>("bufferOffset", 0);
    accessor.componentType = (ComponentType)j["componentType"].get<uint32>();
    accessor.normalized = j.value<bool>("normalized", false);
    accessor.count = j["count"].get<int>();
    accessor.type = attributeTypeLookup[j["type"].get<String>()];

    auto minIter = j.find("min");
    if (minIter != j.end())
    {
        accessor.min = ParseAccessorMinMax(accessor.componentType, accessor.type, *minIter);
    }

    auto maxIter = j.find("max");
    if (maxIter != j.end())
    {
        accessor.max = ParseAccessorMinMax(accessor.componentType, accessor.type, *maxIter);
    }

    auto sparseIter = j.find("sparse");
    if (sparseIter != j.end())
    {
        accessor.sparse = ParseSparse(*sparseIter);
    }

    return accessor;
}

AnimationSampler ParseAnimationSampler(const json& j)
{
    AnimationSampler animationSampler;
    animationSampler.input = j["input"].get<int32>();
    animationSampler.interpolation = animationInterpolationLookup[j.value("interpolation", "LINEAR")];
    animationSampler.output = j["output"].get<int32>();
    return animationSampler;
}

Animation ParseAnimation(const json& j)
{
    Animation animation;
    animation.channels = ParseBuiltinArray<int32>(j["channels"]);
    animation.samplers = ParseArray<AnimationSampler, ParseAnimationSampler>(j["samplers"]);
    animation.name = j.value("name", "");
    return animation;
}

Asset ParseAsset(const json& j)
{
    Asset result;
    result.copyright = j.value("copyright", "");
    result.generator = j.value("generator", "");
    result.version = j["version"].get<String>();
    result.minVersion = j.value("minVersion", "");
    return result;
}

Buffer ParseBuffer(const json& j)
{
    Buffer buffer;
    buffer.uri = j.value("uri", "");
    buffer.byteLength = j["byteLength"].get<int32>();
    buffer.name = j.value("name", "");
    return buffer;
}

BufferView ParseBufferView(const json& j)
{
    BufferView bufferView;
    bufferView.buffer = j["buffer"].get<int32>();
    bufferView.byteOffset = j.value<int32>("byteOffset", 0);
    bufferView.byteLength = j["byteLength"].get<int32>();
    bufferView.byteStride = j.value<Optional<int32>>("byteStride", {});
    return bufferView;
}

Orthographic ParseOrthographic(const json& j)
{
    Orthographic orthographic;
    orthographic.xmag = j["xmag"].get<float32>();
    orthographic.ymag = j["ymag"].get<float32>();
    orthographic.zfar = j["zfar"].get<float32>();
    orthographic.znear = j["znear"].get<float32>();
    return orthographic;
}

Perspective ParsePerspective(const json& j)
{
    Perspective perspective;
    perspective.aspectRatio = j.value<Optional<float32>>("aspectRatio", {});
    perspective.yfov = j["yfov"].get<float32>();
    perspective.zfar = j.value<Optional<float32>>("zfar", {});
    perspective.znear = j["znear"].get<float32>();
    return perspective;
}

Camera ParseCamera(const json& j)
{
    Camera camera;
    camera.type = cameraTypeLookup[j["type"].get<String>()];
    switch (camera.type)
    {
    case CameraType::Orthographic:
        camera.orthographic = ParseOrthographic(j["orthographic"]);
        break;
    case CameraType::Perspective:
        camera.perspective = ParsePerspective(j["perspective"]);
        break;
    default:
        HUSKY_ASSERT(false);
    }
    return camera;
}

Channel ParseChannel(const json& j)
{
    Channel channel;
    return channel;
}

Image ParseImage(const json& j)
{
    Image image;
    image.uri = j.value("uri", "");
    image.mimeType = j.value("mimeType", "");
    if (image.uri.empty())
    {
        image.bufferView = j["bufferView"].get<int32>();
    }
    return image;
}

Indices ParseIndices(const json& j)
{
    Indices indices;
    indices.bufferView = j["bufferView"].get<int32>();
    indices.byteOffset = j.value<int32>("byteOffset", 0);
    indices.componentType = (ComponentType)j["componentType"].get<int32>();
    return indices;
}

TextureInfo ParseTextureInfo(const json& j)
{
    TextureInfo textureInfo;
    textureInfo.index = j["index"].get<int32>();
    return textureInfo;
}

UniquePtr<glTF> glTFParser::ParseJSON(Stream* stream)
{
    json j;

    {
        int64 size = stream->GetSize();
        Vector<uint8> buffer;
        buffer.resize(size);

        stream->Read(buffer.data(), size);
        j = json::parse(buffer.begin(), buffer.end());
    }

    HUSKY_ASSERT(j.is_object());

    UniquePtr<glTF> root = MakeUnique<glTF>();

    auto assetIter = j.find("asset");

    HUSKY_ASSERT(assetIter != j.end());


    auto extensionsUsedIter = j.find("extensionsUsed");
    if (extensionsUsedIter != j.end())
    {
        root->extensionsUsed = ParseBuiltinArray<String>(*extensionsUsedIter);
    }

    auto extensionsRequiredIter = j.find("extensionsRequired");
    if (extensionsRequiredIter != j.end())
    {
        root->extensionsRequired = ParseBuiltinArray<String>(*extensionsRequiredIter);
    }

    root->asset = ParseAsset(*assetIter);

    return root;
}

}
