#include <Luch/Render/RenderUtils.h>
#include <Luch/Render/ShaderDefines.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/Graphics/CommandQueue.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/FileStream.h>
#include <regex>

namespace Luch::Render::RenderUtils
{
    using namespace Graphics;

    CameraUniform GetCameraUniform(SceneV1::Camera* camera, const Mat4x4& transform)
    {
        CameraUniform cameraUniform;
        cameraUniform.view = glm::inverse(transform);
        cameraUniform.inverseView = glm::inverse(cameraUniform.view);
        cameraUniform.projection = camera->GetCameraProjectionMatrix();
        cameraUniform.inverseProjection = glm::inverse(cameraUniform.projection);
        cameraUniform.viewProjection = cameraUniform.projection * cameraUniform.view;
        cameraUniform.inverseViewProjection = glm::inverse(cameraUniform.viewProjection);
        cameraUniform.positionWS = transform * Vec4{ 0.0, 0.0, 0.0, 1.0 };

        Vec2 zMinMax;
        auto cameraType = camera->GetCameraType();
        if (cameraType == SceneV1::CameraType::Orthographic)
        {
            auto orthographicCamera = (SceneV1::OrthographicCamera*)camera;
            zMinMax.x = orthographicCamera->GetZNear();
            zMinMax.y = orthographicCamera->GetZFar();
        }
        else if(cameraType == SceneV1::CameraType::Perspective)
        {
            auto perspectiveCamera = (SceneV1::PerspectiveCamera*)camera;
            zMinMax.x = perspectiveCamera->GetZNear();
            zMinMax.y = perspectiveCamera->GetZFar().value_or(Limits<float32>::infinity());
        }

        cameraUniform.zMinMax = zMinMax;

        return cameraUniform;
    }

    MaterialUniform GetMaterialUniform(SceneV1::PbrMaterial* material)
    {
        MaterialUniform materialUniform;

        materialUniform.baseColorFactor = material->GetProperties().metallicRoughness.baseColorFactor;
        materialUniform.metallicFactor = material->GetProperties().metallicRoughness.metallicFactor;
        materialUniform.roughnessFactor = material->GetProperties().metallicRoughness.roughnessFactor;
        materialUniform.normalScale = material->GetProperties().normalTextureInfo.scale;
        materialUniform.occlusionStrength = material->GetProperties().occlusionTextureInfo.strength;
        materialUniform.emissiveFactor = material->GetProperties().emissiveFactor;
        materialUniform.alphaCutoff = material->GetProperties().alphaCutoff;
        materialUniform.unlit = material->GetProperties().unlit;

        return materialUniform;
    }

    LightUniform GetLightUniform(SceneV1::Light* light, const Mat4x4& worldTransform)
    {
        LightUniform lightUniform;

        lightUniform.positionWS = worldTransform * Vec4{ 0.0, 0.0, 0.0, 1.0 };
        lightUniform.directionWS = glm::normalize(worldTransform * Vec4{ 0.0, 0.0, -1.0, 0.0 });
        lightUniform.color = Vec4{ light->GetColor().value_or(Vec3{1.0, 1.0, 1.0}), 1.0 };
        lightUniform.enabled = light->IsEnabled() ? 1 : 0;
        lightUniform.type = static_cast<int32>(light->GetType());
        lightUniform.innerConeAngle = light->GetInnerConeAngle().value_or(0.0);
        lightUniform.outerConeAngle = light->GetOuterConeAngle().value_or(0.0);
        lightUniform.range = light->GetRange().value_or(0.0);
        lightUniform.intensity = light->GetIntensity();

        return lightUniform;
    }

    String LoadShaderSource(const FilePath& path)
    {
        FileStream fileStream{ path, FileOpenModes::Read };
        auto fileSize = fileStream.GetSize();
        String result;
        result.resize(fileSize);
        [[maybe_unused]] auto bytesRead = fileStream.Read(result.data(), fileSize, sizeof(Byte));
        LUCH_ASSERT(bytesRead == fileSize);
        return result;
    }

    ResultValue<bool, RefPtr<ShaderLibrary>> CreateShaderLibrary(
        GraphicsDevice* device,
        const String& includeDir,
        const String& dir,
        const String& filename,
        const UnorderedMap<String, Variant<int32, String>>& defines)
    {
        #if LUCH_USE_METAL
            String extension = ".metal";
        #elif LUCH_USE_VULKAN
            String extension = ".glsl";
        #else
            String extension = "";
        #endif

        String path = dir + filename + extension;

        auto shaderSource = LoadShaderSource(path);

        // TODO handle files that don't end with blank line

        auto [substituteSucceeded, sourceWithIncludes] = SubstituteIncludes(includeDir, shaderSource);
        if(!substituteSucceeded)
        {
            return { false };
        }

        auto [createLibraryResult, library] = device->CreateShaderLibraryFromSource(sourceWithIncludes, defines);
        if(createLibraryResult != GraphicsResult::Success && createLibraryResult != GraphicsResult::CompilerWarning)
        {
            LUCH_ASSERT(false);
            return { false };
        }
        else
        {
            return { true, library };
        }
    }

    ResultValue<bool, String> SubstituteIncludesImpl(
        const String& includeDir,
        String source,
        UnorderedSet<String>& alreadyIncluded)
    {
        const std::regex includeRegex { R"###(#include\s+\"(.*)\"\s*)###", std::regex::ECMAScript };
        const std::regex pragmaOnceRegex { R"###(#pragma\s+once\s*)###", std::regex::ECMAScript };

        std::smatch includeMatch;
        while(std::regex_search(source, includeMatch, includeRegex))
        {
            int32 position = includeMatch.position();
            int32 length = includeMatch.length();
            String includeFilename = includeMatch[1];

            auto includeSource = LoadShaderSource(includeDir + includeFilename);

            std::smatch pragmaOnceMatch;
            bool pragmaOnce = std::regex_search(includeSource, pragmaOnceMatch, pragmaOnceRegex);

            if(pragmaOnce)
            {
                if(alreadyIncluded.count(includeFilename))
                {
                    source.replace(position, length, "");
                    continue;
                }
                else
                {
                    includeSource = std::regex_replace(includeSource, pragmaOnceRegex, "");
                    alreadyIncluded.insert(includeFilename);
                }
            }

            auto [substituteSucceeded, includeWithIncludes] = SubstituteIncludesImpl(includeDir, includeSource, alreadyIncluded);
            if(!substituteSucceeded)
            {
                return { false };
            }
            includeSource = std::move(includeWithIncludes);

            source.replace(position, length, includeSource);
        }

        return { true, source };
    }

    ResultValue<bool, String> SubstituteIncludes(
        const String& includeDir,
        String source)
    {
        UnorderedSet<String> alreadyIncluded;
        return SubstituteIncludesImpl(includeDir, source, alreadyIncluded);
    }

    void SubmitCommandLists(
        const RefPtr<CommandQueue>& queue,
        const RefPtrVector<CommandList>& commandLists,
        const std::function<void()> completedHandler)
    {
        for(uint32 i = 0; i < commandLists.size(); i++)
        {
            if(i == commandLists.size() - 1)
            {
               queue->Submit(commandLists[i], completedHandler); 
            }
            else
            {
                queue->Submit(commandLists[i], {});
            }
        }
    }

    GraphicsPipelineVertexInputStateCreateInfo GetPrimitiveVertexInputStateCreateInfo(
        SceneV1::Primitive* primitive)
    {
        GraphicsPipelineVertexInputStateCreateInfo inputAssembler;

        const auto& vertexBuffers = primitive->GetVertexBuffers();
        LUCH_ASSERT(vertexBuffers.size() == 1);

        inputAssembler.bindings.resize(vertexBuffers.size());
        for (uint32 i = 0; i < vertexBuffers.size(); i++)
        {
            const auto& vertexBuffer = vertexBuffers[i];
            auto& bindingDescription = inputAssembler.bindings[i];
            bindingDescription.stride = vertexBuffer.stride;
            // TODO instancing
            bindingDescription.inputRate = VertexInputRate::PerVertex;
        }

        const auto& attributes = primitive->GetAttributes();
        inputAssembler.attributes.resize(SemanticToLocation.size());

        for (const auto& attribute : attributes)
        {
            auto& attributeDescription = inputAssembler.attributes[SemanticToLocation.at(attribute.semantic)];
            attributeDescription.binding = attribute.vertexBufferIndex;
            attributeDescription.format = attribute.format;
            attributeDescription.offset = attribute.offset;
        }

        // TODO
        inputAssembler.primitiveTopology = PrimitiveTopology::TriangleList;

        return inputAssembler;
    }

    void AddPrimitiveVertexShaderDefines(SceneV1::Primitive* primitive, ShaderDefines& shaderDefines)
    {
        LUCH_ASSERT(primitive != nullptr);

        const auto& attributes = primitive->GetAttributes();
        for (const auto& attribute : attributes)
        {
            shaderDefines.AddFlag(SemanticToFlag.at(attribute.semantic));
        }
    }

    void AddMaterialShaderDefines(SceneV1::PbrMaterial* material, ShaderDefines& shaderDefines)
    {
        LUCH_ASSERT(material != nullptr);

        if (material->HasBaseColorTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasBaseColorTexture);
        }

        if (material->HasMetallicRoughnessTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasMetallicRoughnessTexture);
        }

        if (material->HasNormalTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasNormalTexture);
        }

        if (material->HasOcclusionTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasOcclusionTexture);
        }

        if (material->HasEmissiveTexture())
        {
            shaderDefines.AddFlag(MaterialShaderDefines::HasEmissiveTexture);
        }

        if (material->GetProperties().alphaMode == SceneV1::AlphaMode::Mask)
        {
            shaderDefines.AddFlag(MaterialShaderDefines::AlphaMask);
        }
    }
}
