#pragma once

#include <Luch/Types.h>
#include <Luch/VectorTypes.h>

namespace Luch::glTF
{

enum class AlphaMode
{
    Opaque,
    Mask,
    Blend,
};

enum class AnimationInterpolation
{
    Linear,
    Step,
    CatmullRomSpline,
    CubicSpline,
};

enum class AttributeSemantic
{
    Position,
    Normal,
    Tangent,
    Texcoord_0,
    Texcoord_1,
    Color_0,
    Joints_0,
};

enum class AttributeType
{
    Scalar,
    Vec2,
    Vec3,
    Vec4,
    Mat2x2,
    Mat3x3,
    Mat4x4,
};

enum class CameraType
{
    Orthographic,
    Perspective,
};

enum class ComponentType
{
    Int8 = 5120,
    UInt8 = 5121,
    Int16 = 5122,
    UInt16 = 5123,
    UInt = 5125,
    Float = 5126,
};

enum class MagFilter
{
    Nearest = 9728,
    Linear = 9729,
};

enum class MinFilter
{
    Nearest = 9728,
    Linear = 9729,
    NearestMipMapNearest = 9984,
    LinearMipMapNearest = 9985,
    NearestMipMapLinear = 9986,
    LinearMipMapLinear = 9987,
};

enum class PrimitiveMode
{
    Points = 0,
    Lines = 1,
    LineLoop = 2,
    LineStrip = 3,
    Triangles = 4,
    TriangleStrip = 5,
    TriangleFan = 6,
};

enum class TargetPath
{
    Translation,
    Rotation,
    Scale,
    Weights,
};

enum class BufferType
{
    ArrayBuffer = 34962,
    ElementArrayBuffer = 34963,
};

enum class WrapMode
{
    ClampToEdge = 33071,
    MirroredRepeat = 33648,
    Repeat = 10497,
};

enum class LightType
{
    Directional,
    Point,
    Spot,
};

using AccessorValueHolder = Array<uint32, 16>;

struct SparseValues
{
    int32 bufferView;
    int32 byteOffset = 0;
};

struct Sparse
{
    int32 count = 0;
    int32 indices = -1;
    SparseValues values;
};

struct Accessor
{
    Optional<int32> bufferView;
    int32 byteOffset = 0;
    ComponentType componentType;
    bool normalized = false;
    int32 count = 0;
    AttributeType type;
    Optional<AccessorValueHolder> min;
    Optional<AccessorValueHolder> max;
    Optional<Sparse> sparse;
};

struct AnimationSampler
{
    int32 input = -1;
    AnimationInterpolation interpolation = AnimationInterpolation::Linear;
    int32 output = -1;
};

struct Animation
{
    Vector<int32> channels;
    Vector<AnimationSampler> samplers;
    String name;
};

struct Asset
{
    String copyright;
    String generator;
    String version;
    String minVersion;
};

struct Attribute
{
    AttributeSemantic semantic;
    int32 accessor;
};

struct Buffer
{
    String uri;
    int32 byteLength = 0;
    String name;
};

struct BufferView
{
    int32 buffer;
    int32 byteOffset = 0;
    int32 byteLength;
    Optional<int32> byteStride;
    Optional<BufferType> target;
    String name;
};

struct Orthographic
{
    float32 xmag;
    float32 ymag;
    float32 zfar;
    float32 znear;
};

struct Perspective
{
    Optional<float32> aspectRatio;
    float32 yfov;
    Optional<float32> zfar;
    float32 znear;
};

struct Camera
{
    Orthographic orthographic;
    Perspective perspective;
    CameraType type = CameraType::Perspective;
    String name;
};

struct Channel
{
    int32 sampler = -1;
    // target
};

struct Image
{
    String uri;
    String mimeType;
    Optional<int32> bufferView;
    String name;
};

struct Indices
{
    int32 bufferView = -1;
    int32 byteOffset = 0;
    ComponentType componentType = ComponentType::UInt16;
};

struct TextureInfo
{
    int32 index = -1;
    int32 texCoord = 0;
};

struct PBRMetallicRoughness
{
    Vec4 baseColorFactor = { 1, 1, 1, 1 };
    Optional<TextureInfo> baseColorTexture;
    float32 metallicFactor = 1;
    float32 roughnessFactor = 1;
    Optional<TextureInfo> metallicRoughnessTexture;
};

struct NormalTextureInfo : public TextureInfo
{
    float32 scale = 1;
};

struct OcclusionTextureInfo : public TextureInfo
{
    float32 strength = 1;
};

struct MaterialsUnlit
{
};

struct MaterialExtensions
{
    Optional<MaterialsUnlit> unlit;
};

struct Material
{
    String name;
    PBRMetallicRoughness pbrMetallicRoughness;
    Optional<NormalTextureInfo> normalTexture;
    Optional<OcclusionTextureInfo> occlusionTexture;
    Optional<TextureInfo> emissiveTexture;
    Vec3 emissiveFactor = { 0, 0, 0 };
    AlphaMode alphaMode = AlphaMode::Opaque;
    float32 alphaCutoff = 0.5f;
    bool doubleSided = false;
    Optional<MaterialExtensions> extensions;
};

struct Primitive
{
    Vector<Attribute> attributes;
    Optional<int32> indices;
    Optional<int32> material;
    PrimitiveMode mode = PrimitiveMode::Triangles;
    // Vector<> targets;
};

struct Sampler
{
    Optional<MagFilter> magFilter;
    Optional<MinFilter> minFilter;
    WrapMode wrapS = WrapMode::Repeat;
    WrapMode wrapT = WrapMode::Repeat;
    String name;
};

struct Mesh
{
    Vector<Primitive> primitives;
    Vector<float32> weights;
    String name;
};

struct Scene
{
    Vector<int32> nodes;
    String name;
};

struct Skin
{
    Optional<int32> inverseBindMatrices;
    Optional<int32> skeleton;
    Vector<int32> joints;
    String name;
};

struct NodeLightsPunctual
{
    int32 light = -1;
};

struct NodeLightsProbe
{
    int32 probe = -1;
};

struct NodeExtensions
{
    Optional<NodeLightsPunctual> lights;
    Optional<NodeLightsProbe> probes;
};

struct Node
{
    Optional<int32> camera;
    Vector<int32> children;
    Optional<int32> skin;
    Optional<Mat4x4> matrix;
    Optional<int32> mesh;
    Optional<Quaternion> rotation;
    Optional<Vec3> scale;
    Optional<Vec3> translation;
    Vector<float32> weights;
    Optional<NodeExtensions> extensions;
    String name;
};

struct Target
{
    Optional<int32> node;
    TargetPath path;
};

struct Texture
{
    Optional<int32> sampler;
    Optional<int32> source;
    String name;
};

struct Spot
{
    float32 innerConeAngle = 0;
    float32 outerConeAngle = glm::pi<float32>() / 4.0;
};

struct LightPunctual
{
    String name = "";
    Vec3 color = Vec3{ 1.0, 1.0, 1.0 };
    float32 intensity = 1.0;
    LightType type = LightType::Directional;
    Optional<float32> range;
    Optional<Spot> spot;
};

struct LightProbe
{
    String name = "";
    glm::ivec2 size = glm::ivec2{ 0, 0 };
    float32 znear = 0.0;
    float32 zfar = 0.0;
    bool specularReflection = true;
    bool diffuseIlluminance = true;
};

struct RootLightsPunctual
{
    Vector<LightPunctual> lights;
};

struct RootLightsProbe
{
    Vector<LightProbe> probes;
};

struct RootExtensions
{
    Optional<RootLightsPunctual> lights;
    Optional<RootLightsProbe> probe;
};

struct glTFRoot
{
    Vector<String> extensionsUsed;
    Vector<String> extensionsRequired;
    Vector<Accessor> accessors;
    Vector<Animation> animations;
    Asset asset;
    Vector<Buffer> buffers;
    Vector<BufferView> bufferViews;
    Vector<Camera> cameras;
    Vector<Image> images;
    Vector<Material> materials;
    Vector<Mesh> meshes;
    Vector<Node> nodes;
    Vector<Sampler> samplers;
    Vector<Scene> scenes;
    Optional<int32> scene;
    Vector<Skin> skins;
    Vector<Texture> textures;
    Optional<RootExtensions> extensions;
};

}
