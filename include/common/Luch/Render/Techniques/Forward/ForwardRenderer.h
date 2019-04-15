#include <Luch/Render/RenderForwards.h>
#include <Luch/SceneV1/SceneV1Forwards.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Render/Techniques/Forward/ForwardRendererContext.h>
#include <Luch/Types.h>
#include <Luch/VectorTypes.h>
#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>

namespace Luch::Render::Techniques::Forward
{
    using namespace Graphics;

    struct IBLTextures
    {
        RefPtr<Texture> diffuseIlluminanceCubemap;
        RefPtr<Texture> specularReflectionCubemap;
        RefPtr<Texture> specularBRDFTexture;
    };

    class ForwardRenderer
    {
    public:
        static UniquePtr<ForwardRendererPersistentContext> PrepareForwardRendererPersistentContext(
            const ForwardRendererPersistentContextCreateInfo& createInfo);

        static UniquePtr<ForwardRendererTransientContext> PrepareForwardRendererTransientContext(
            ForwardRendererPersistentContext* persistentContext,
            const ForwardRendererTransientContextCreateInfo& createInfo);

        ForwardRenderer(
            ForwardRendererPersistentContext* persistentContext,
            ForwardRendererTransientContext* transientContext);

        void PrepareScene(SceneV1::Scene* scene);
        void UpdateScene(SceneV1::Scene* scene);

        void DrawScene(
            SceneV1::Scene* scene,
            const IBLTextures& iblTextures,
            GraphicsCommandList* commandList);
    private:
        void PrepareNode(SceneV1::Node* node);
        void PrepareMeshNode(SceneV1::Node* node);
        void PrepareMesh(SceneV1::Mesh* mesh);
        void PreparePrimitive(SceneV1::Primitive* primitive);

        void UpdateNode(SceneV1::Node* node);
        void UpdateMesh(SceneV1::Mesh* mesh, const Mat4x4& transform);
        void UpdateLights(const RefPtrVector<SceneV1::Node>& lightNodes);

        void UpdateIndirectLightingDescriptorSet(
            const IBLTextures& iblTextures,
            DescriptorSet* descriptorSet);

        void BindMaterial(SceneV1::PbrMaterial* material, GraphicsCommandList* commandList);
        void DrawNode(SceneV1::Node* node, GraphicsCommandList* commandList);
        void DrawMesh(SceneV1::Mesh* mesh, GraphicsCommandList* commandList);
        void DrawPrimitive(SceneV1::Primitive* primitive, GraphicsCommandList* commandList);

        const String& GetPipelineStateName();

        static RefPtr<GraphicsPipelineState> CreatePipelineState(
            SceneV1::Primitive* primitive,
            bool useDepthPrepass,
            ForwardRendererPersistentContext* context);

        ForwardRendererPersistentContext* persistentContext = nullptr;
        ForwardRendererTransientContext* transientContext = nullptr;

        UnorderedMap<SceneV1::Mesh*, RefPtr<DescriptorSet>> meshDescriptorSets;
    };
}
