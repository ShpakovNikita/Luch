#include <Luch/Render/RenderUtils.h>
#include <Luch/Graphics/GraphicsDevice.h>
#include <Luch/SceneV1/Camera.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Light.h>
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

        #if LUCH_PLATFORM_IOS
            String path = filename + extension;
        #else
            String path = dir + filename + extension;
        #endif

        auto shaderSource = LoadShaderSource(path);

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
}
