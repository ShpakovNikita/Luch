#pragma once

#include <Luch/RefPtr.h>
#include <Luch/UniquePtr.h>
#include <Luch/Types.h>
#include <Luch/Graphics/BufferUsageFlags.h>
#include <Luch/Graphics/Format.h>
#include <Luch/Graphics/GraphicsForwards.h>
#include <Luch/Graphics/TextureUsageFlags.h>
#include <Luch/Render/Graph/RenderGraphForwards.h>
#include <Luch/Render/Graph/RenderGraphResources.h>

namespace Luch::Render::Graph
{
    using namespace Graphics;

    class RenderGraphBuilder;

    class RenderGraphNode
    {
        friend class RenderGraphBuilder;
    public:
        RenderMutableResource CreateRenderTarget(const RenderTargetInfo& info);
        RenderMutableResource ImportRenderTarget(const RefPtr<Texture>& texture);

        void ReadsTexture(RenderResource textureResource);
        RenderMutableResource WritesToRenderTarget(
            RenderMutableResource renderTargetResource,
            TextureUsageFlags usageFlags);
    private:
        RenderGraphPass* pass = nullptr;
        String name;
        RenderGraphResourceManager* resourceManager = nullptr;
        Vector<RenderMutableResource> createdRenderTargets;
        Vector<RenderMutableResource> importedRenderTargets;
        Vector<RenderResource> readTextures;
        Vector<RenderMutableResource> writeTextures;
    };

    struct RenderGraphBuildResult
    {
        bool resourcesCreated = false;
        Vector<RenderGraphNode*> nodes;
        UnorderedMultimap<int32, int32> edges;
        Vector<RenderResource> unusedResources;
        // TODO unused nodes
    };

    struct RenderGraphExecuteResult
    {
        RefPtrVector<GraphicsCommandList> commandLists;
    };

    struct RenderGraphSubmitResult
    {
    };

    class RenderGraphBuilder
    {
    public:
        RenderGraphBuilder() = default;
        ~RenderGraphBuilder();

        bool Initialize(GraphicsDevice* device, CommandQueue* queue);
        bool Deinitialize();

        RenderGraphNode* AddRenderPass(const String& name, RenderGraphPass* pass);

        RenderGraphBuildResult Build();
        RenderGraphExecuteResult Execute(RenderGraphBuildResult& buildResult);
        RenderGraphSubmitResult Submit(RenderGraphExecuteResult& executeResult);
    private:
        static UnorderedMultimap<int32, int32> CalculateEdges(Vector<RenderGraphNode> nodes);

        GraphicsDevice* device = nullptr;
        CommandQueue* queue = nullptr;
        RefPtr<CommandPool> commandPool;
        UniquePtr<RenderGraphResourceManager> resourceManager;
        Vector<RenderGraphNode> nodes;
    };
}
