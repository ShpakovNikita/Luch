#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Stream.h>
#include <json.hpp>

namespace Husky::glTF
{

using json = nlohmann::json;

Map<String, AlphaMode> alphaModeLookup = 
{
    { "OPAQUE", AlphaMode::Opaque },
    { "MASK", AlphaMode::Mask },
    { "BLEND", AlphaMode::Blend },
};

Map<String, AnimationInterpolation> animationInterpolationLookup =
{
    { "LINEAR", AnimationInterpolation::Linear },
    { "STEP", AnimationInterpolation::Step },
    { "CATMULLROMSPLINE", AnimationInterpolation::CatmullRomSpline },
    { "CUBICSPLINE", AnimationInterpolation::CubicSpline },
};

Map<String, AttributeSemantic> attributeSemanticLookup =
{
    { "POSITION", AttributeSemantic::Position },
    { "NORMAL", AttributeSemantic::Normal },
    { "TANGENT", AttributeSemantic::Tangent },
    { "TEXCOORD_0", AttributeSemantic::Texcoord_0 },
    { "TEXCOORD_1", AttributeSemantic::Texcoord_1 },
    { "COLOR_0", AttributeSemantic::Color_0 },
    { "JOINTS_0", AttributeSemantic::Joints_0 },
};

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

Map<String, CameraType> cameraTypeLookup = 
{
    {"perspective", CameraType::Perspective},
    {"orthographic", CameraType::Orthographic}
};

Map<String, TargetPath> targetPathLookup =
{
    { "translation", TargetPath::Translation },
    { "rotation", TargetPath::Rotation },
    { "scale", TargetPath::Scale },
    { "weights", TargetPath::Weights },
};

void from_json(const json& j, Vec3& v)
{
    v.x = j[0].get<float32>();
    v.y = j[1].get<float32>();
    v.z = j[2].get<float32>();
}

void from_json(const json& j, Vec4& v)
{
    v.x = j[0].get<float32>();
    v.y = j[1].get<float32>();
    v.z = j[2].get<float32>();
    v.w = j[3].get<float32>();
}

String ParseStringOrDefault(const json& j, const char8* key, const char8* defaultValue)
{
    return j.value(key, defaultValue);
}

String ParseStringOrEmpty(const json& j, const char8* key)
{
    return ParseStringOrDefault(j, key, "");
}

String ParseName(const json& j)
{
    return ParseStringOrEmpty(j, "name");
}

Optional<int32> ParseOptionalIndex(const json& j, const char8* key)
{
    auto it = j.find(key);
    if(it != j.end())
    {
        return j.value<int32>(key, -1);
    }
    else
    {
        return {};
    }
}

MagFilter ParseMagFilter(const json& j)
{
    return (MagFilter)j.get<uint32>();
}

MinFilter ParseMinFilter(const json& j)
{
    return (MinFilter)j.get<uint32>();
}

Vec3 ParseVec3(const json& j)
{
    Vec3 v;
    v.x = j[0].get<float32>();
    v.y = j[1].get<float32>();
    v.z = j[2].get<float32>();
    return v;
}

Vec4 ParseVec4(const json& j)
{
    Vec4 v;
    v.x = j[0].get<float32>();
    v.y = j[1].get<float32>();
    v.z = j[2].get<float32>();
    v.w = j[3].get<float32>();
    return v;
}

Mat4x4 ParseMat4x4(const json& j)
{
    Mat4x4 m;
    for (int32 column = 0; column < 4; column++)
    {
        for (int32 row = 0; row < 4; row++)
        {
            m[column][row] = j[column*4 + row].get<float32>();
        }
    }
    return m;
}

Quaternion ParseQuaternion(const json& j)
{
    Quaternion q;
    q.x = j[0].get<float32>();
    q.y = j[1].get<float32>();
    q.z = j[2].get<float32>();
    q.w = j[3].get<float32>();
    return q;
}

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

template<typename T, T(*Function)(const json&)>
Vector<T> ParseArrayOrEmpty(const json& j, const char8* key)
{
    auto it = j.find(key);
    if (it != j.end())
    {
        return ParseArray<T, Function>(*it);
    }
    else
    {
        return {};
    }
}

template<typename T>
Vector<T> ParseBuiltinArrayOrEmpty(const json& j, const char8* key)
{
    return ParseArrayOrEmpty<T, ParseBuiltin<T>>(j, key);
}

template<typename T, T(*Function)(const json&)>
T ParseOrDefault(const json& j, const char8* key, const T& defaultValue)
{
    auto it = j.find(key);
    if (it != j.end())
    {
        return Function(*it);
    }
    else
    {
        return defaultValue;
    }
}

template<typename T>
T ParseBuiltinOrDefault(const json& j, const char8* key, const T& defaultValue)
{
    return ParseOrDefault<T, ParseBuiltin<T>>(j, key, defaultValue);
}

template<typename T, T(*Function)(const json&)>
Optional<T> ParseOptional(const json& j, const char8* key)
{
    auto it = j.find(key);
    if (it != j.end())
    {
        return Function(*it);
    }
    else
    {
        return {};
    }
}

template<typename T>
Optional<T> ParseBuiltinOptional(const json& j, const char8* key)
{
    return ParseOptional<T, ParseBuiltin<T>>(j, key);
}

template<typename T>
T ParseBuiltin(const json& j, const char8* key)
{
    return j[key].get<T>();
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
    HUSKY_ASSERT(false);
    return sparse;
}

AccessorValueHolder ParseAccessorMinMax(ComponentType componentType, AttributeType type, const json& j)
{
    AccessorValueHolder valueHolder{};
    return valueHolder;
}

Accessor ParseAccessor(const json& j)
{
    Accessor accessor;
    accessor.bufferView = ParseOptionalIndex(j, "bufferView");
    accessor.byteOffset = ParseBuiltinOrDefault<int32>(j, "byteOffset", 0);
    accessor.componentType = (ComponentType)ParseBuiltin<uint32>(j, "componentType");
    accessor.normalized = ParseBuiltinOrDefault<bool>(j, "normalized", false);
    accessor.count = ParseBuiltin<int32>(j, "count");
    accessor.type = attributeTypeLookup[ParseBuiltin<String>(j, "type")];

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

    accessor.sparse = ParseOptional<Sparse, ParseSparse>(j, "sparse");

    return accessor;
}

AnimationSampler ParseAnimationSampler(const json& j)
{
    AnimationSampler animationSampler;
    animationSampler.input = ParseBuiltin<int32>(j, "input");
    animationSampler.interpolation = animationInterpolationLookup[ParseBuiltinOrDefault<String>(j, "interpolation", "LINEAR")];
    animationSampler.output = ParseBuiltin<int32>(j, "output");
    return animationSampler;
}

Animation ParseAnimation(const json& j)
{
    Animation animation;
    animation.channels = ParseBuiltinArray<int32>(j["channels"]);
    animation.samplers = ParseArray<AnimationSampler, ParseAnimationSampler>(j["samplers"]);
    animation.name = ParseName(j);
    return animation;
}

Asset ParseAsset(const json& j)
{
    Asset result;
    result.copyright = ParseStringOrEmpty(j, "copyright");
    result.generator = ParseStringOrEmpty(j, "generator");
    result.version = ParseBuiltin<String>(j, "version");
    result.minVersion = ParseStringOrEmpty(j, "minVersion");
    return result;
}

Vector<Attribute> ParseAttributes(const json& j)
{
    Vector<Attribute> attributes;

    for (auto it = j.begin(); it != j.end(); ++it)
    {
        auto semantic = attributeSemanticLookup[it.key()];
        auto accessor = it->get<int32>();
        attributes.push_back(Attribute{ semantic, accessor });
    }

    return attributes;
}

Buffer ParseBuffer(const json& j)
{
    Buffer buffer;
    buffer.uri = ParseStringOrEmpty(j, "uri");
    buffer.byteLength = ParseBuiltin<int32>(j, "byteLength");
    buffer.name = ParseName(j);
    return buffer;
}

BufferView ParseBufferView(const json& j)
{
    BufferView bufferView;
    bufferView.buffer = ParseBuiltin<int32>(j, "buffer");
    bufferView.byteOffset = ParseBuiltinOrDefault<int32>(j, "byteOffset", 0);
    bufferView.byteLength = ParseBuiltin<int32>(j, "byteLength");
    bufferView.byteStride = ParseOptionalIndex(j, "byteStride");

    Optional<uint32> target = ParseBuiltinOptional<uint32>(j, "target");
    if (target.has_value())
    {
        bufferView.target = (BufferType)target.value();
    }

    bufferView.name = ParseName(j);

    return bufferView;
}

Orthographic ParseOrthographic(const json& j)
{
    Orthographic orthographic;
    orthographic.xmag = ParseBuiltin<float32>(j, "xmag");
    orthographic.ymag = ParseBuiltin<float32>(j, "ymag");
    orthographic.zfar = ParseBuiltin<float32>(j, "zfar");
    orthographic.znear = ParseBuiltin<float32>(j, "znear");
    return orthographic;
}

Perspective ParsePerspective(const json& j)
{
    Perspective perspective;
    perspective.aspectRatio = ParseBuiltinOptional<float32>(j, "aspectRatio");
    perspective.yfov = ParseBuiltin<float32>(j, "yfov");
    perspective.zfar = ParseBuiltinOptional<float32>(j, "zfar");
    perspective.znear = ParseBuiltin<float32>(j, "znear");
    return perspective;
}

Camera ParseCamera(const json& j)
{
    Camera camera;
    camera.type = cameraTypeLookup[ParseBuiltin<String>(j, "type")];
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
    image.uri = ParseStringOrEmpty(j, "uri");
    image.mimeType = ParseStringOrEmpty(j, "mimeType");
    if (image.uri.empty())
    {
        image.bufferView = ParseBuiltin<uint32>(j, "bufferView");
    }
    return image;
}

Indices ParseIndices(const json& j)
{
    Indices indices;
    indices.bufferView = ParseBuiltin<int32>(j, "bufferView");
    indices.byteOffset = ParseBuiltinOrDefault<int32>(j, "byteOffset", 0);
    indices.componentType = (ComponentType)ParseBuiltin<uint32>(j, "componentType");
    return indices;
}

TextureInfo ParseTextureInfo(const json& j)
{
    TextureInfo textureInfo;
    textureInfo.index = ParseBuiltin<int32>(j, "index");
    textureInfo.texCoord = ParseBuiltinOrDefault<int32>(j, "texCoord", 0);
    return textureInfo;
}

NormalTextureInfo ParseNormalTextureInfo(const json& j)
{
    NormalTextureInfo textureInfo;
    textureInfo.index = ParseBuiltin<int32>(j, "index");
    textureInfo.texCoord = ParseBuiltinOrDefault<int32>(j, "texCoord", 0);
    textureInfo.scale = ParseBuiltinOrDefault<float32>(j, "sacle", 1.0f);
    return textureInfo;
}

OcclusionTextureInfo ParseOcclusionTextureInfo(const json& j)
{
    OcclusionTextureInfo textureInfo;
    textureInfo.index = ParseBuiltin<int32>(j, "index");
    textureInfo.texCoord = ParseBuiltinOrDefault<int32>(j, "texCoord", 0);
    textureInfo.strength = ParseBuiltinOrDefault<float32>(j, "strength", 1.0f);
    return textureInfo;
}

PBRMetallicRoughness ParsePBRMetallicRoughness(const json& j)
{
    PBRMetallicRoughness pbrMetallicRoughness;
    pbrMetallicRoughness.baseColorFactor = ParseOrDefault<Vec4, ParseVec4>(j, "baseColorFactor", { 1, 1, 1, 1 });
    pbrMetallicRoughness.baseColorTexture = ParseOptional<TextureInfo, ParseTextureInfo>(j, "baseColorTexture");
    pbrMetallicRoughness.metallicFactor = ParseBuiltinOrDefault<float32>(j, "metallicFactor", 1.0f);
    pbrMetallicRoughness.roughnessFactor = ParseBuiltinOrDefault<float32>(j, "roughnessFactor", 1.0f);
    pbrMetallicRoughness.metallicRoughnessTexture = ParseOptional<TextureInfo, ParseTextureInfo>(j, "metallicRoughnessTexture");
    return pbrMetallicRoughness;
}

Material ParseMaterial(const json& j)
{
    Material material;
    material.name = ParseName(j);
    material.pbrMetallicRoughness = ParseOrDefault<PBRMetallicRoughness, ParsePBRMetallicRoughness>(j, "pbrMetallicRoughness", {});
    material.normalTexture = ParseOptional<NormalTextureInfo, ParseNormalTextureInfo>(j, "normalTexture");
    material.occlusionTexture = ParseOptional<OcclusionTextureInfo, ParseOcclusionTextureInfo>(j, "occlusionTexture");
    material.emissiveTexture = ParseOptional<TextureInfo, ParseTextureInfo>(j, "emissiveTexture");
    material.emissiveFactor = ParseOrDefault<Vec3, ParseVec3>(j, "emissiveFactor", { 0, 0, 0 });
    material.alphaMode = alphaModeLookup[ParseStringOrDefault(j, "alphaMode", "OPAQUE")];
    material.alphaCutoff = ParseBuiltinOrDefault<float32>(j, "alphaCutoff", 0.5f);
    material.doubleSided = ParseBuiltinOrDefault<bool>(j, "doubleSided", false);
    return material;
}

Primitive ParsePrimitive(const json& j)
{
    Primitive primitive;
    primitive.attributes = ParseAttributes(j["attributes"]);
    primitive.indices = ParseOptionalIndex(j, "indices");
    primitive.material = ParseOptionalIndex(j, "material");
    primitive.mode = (PrimitiveMode)ParseBuiltinOrDefault<uint32>(j, "mode", 4);
    return primitive;
}

Sampler ParseSampler(const json& j)
{
    Sampler sampler;
    sampler.magFilter = ParseOptional<MagFilter, ParseMagFilter>(j, "magFilter");
    sampler.minFilter = ParseOptional<MinFilter, ParseMinFilter>(j, "minFilter");
    sampler.wrapS = (WrapMode)ParseBuiltinOrDefault<uint32>(j, "wrapS", 10497);
    sampler.wrapT = (WrapMode)ParseBuiltinOrDefault<uint32>(j, "wrapT", 10497);
    sampler.name = ParseName(j);
    return sampler;
}

Mesh ParseMesh(const json& j)
{
    Mesh mesh;
    mesh.primitives = ParseArray<Primitive, ParsePrimitive>(j["primitives"]);
    mesh.weights = ParseBuiltinArrayOrEmpty<float32>(j, "weights");
    mesh.name = ParseName(j);
    return mesh;
}

Scene ParseScene(const json& j)
{
    Scene scene;
    scene.nodes = ParseBuiltinArrayOrEmpty<int32>(j, "nodes");
    scene.name = ParseName(j);
    return scene;
}

Skin ParseSkin(const json& j)
{
    Skin skin;
    skin.inverseBindMatrices = ParseOptionalIndex(j, "inverseBindMatrices");
    skin.skeleton = ParseOptionalIndex(j, "skeleton");
    skin.joints = ParseBuiltinArray<int32>(j["joints"]);
    skin.name = ParseName(j);
    return skin;
}

Node ParseNode(const json& j)
{
    Node node;
    node.camera = ParseOptionalIndex(j, "camera");
    node.children = ParseBuiltinArrayOrEmpty<int32>(j, "children");
    node.skin = ParseOptionalIndex(j, "skin");
    node.matrix = ParseOptional<Mat4x4, ParseMat4x4>(j, "matrix");
    node.mesh = ParseOptionalIndex(j, "mesh");
    node.rotation = ParseOptional<Quaternion, ParseQuaternion>(j, "rotation");
    node.scale = ParseOptional<Vec3, ParseVec3>(j, "scale");
    node.translation = ParseOptional<Vec3, ParseVec3>(j, "translation");
    node.weights = ParseBuiltinArrayOrEmpty<float32>(j, "weights");
    node.name = ParseName(j);
    return node;
}

Target ParseTarget(const json& j)
{
    Target target;
    target.node = ParseOptionalIndex(j, "node");
    target.path = targetPathLookup[ParseBuiltin<String>(j, "path")];
    return target;
}

Texture ParseTexture(const json& j)
{
    Texture texture;
    texture.sampler = ParseOptionalIndex(j, "sampler");
    texture.source = ParseOptionalIndex(j, "source");
    return texture;
}

SharedPtr<glTFRoot> glTFParser::ParseJSON(Stream* stream)
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

    SharedPtr<glTFRoot> root = MakeShared<glTFRoot>();

    root->extensionsUsed = ParseBuiltinArrayOrEmpty<String>(j, "extensionsUsed");
    root->extensionsRequired = ParseBuiltinArrayOrEmpty<String>(j, "extensionsRequired");
    root->accessors = ParseArrayOrEmpty<Accessor, ParseAccessor>(j, "accessors");
    root->asset = ParseAsset(j["asset"]);
    root->buffers = ParseArrayOrEmpty<Buffer, ParseBuffer>(j, "buffers");
    root->bufferViews = ParseArrayOrEmpty<BufferView, ParseBufferView>(j, "bufferViews");
    root->cameras = ParseArrayOrEmpty<Camera, ParseCamera>(j, "cameras");
    root->images = ParseArrayOrEmpty<Image, ParseImage>(j, "images");
    root->materials = ParseArrayOrEmpty<Material, ParseMaterial>(j, "materials");
    root->meshes = ParseArrayOrEmpty<Mesh, ParseMesh>(j, "meshes");
    root->nodes = ParseArrayOrEmpty<Node, ParseNode>(j, "nodes");
    root->samplers = ParseArrayOrEmpty<Sampler, ParseSampler>(j, "samplers");
    root->scenes = ParseArrayOrEmpty<Scene, ParseScene>(j, "scenes");
    root->scene = ParseOptionalIndex(j, "scene");
    root->skins = ParseArrayOrEmpty<Skin, ParseSkin>(j, "skins");
    root->textures = ParseArrayOrEmpty<Texture, ParseTexture>(j, "textures");

    return root;
}

}
