#include <Husky/Render/RenderUtils.h>
#include <Husky/Graphics/GraphicsDevice.h>
#include <Husky/SceneV1/Camera.h>
#include <Husky/SceneV1/PbrMaterial.h>
#include <Husky/SceneV1/Light.h>
#include <Husky/FileStream.h>

namespace Husky::Render::RenderUtils
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

    Vector<Byte> LoadShaderSource(const FilePath& path)
    {
        FileStream fileStream{ path, FileOpenModes::Read };
        auto fileSize = fileStream.GetSize();
        Vector<Byte> result;
        result.resize(fileSize + 1); // +1 for null termination
        [[maybe_unused]] auto bytesRead = fileStream.Read(result.data(), fileSize, sizeof(Byte));
        HUSKY_ASSERT(bytesRead == fileSize);
        return result;
    }

    ResultValue<bool, RefPtr<ShaderLibrary>> CreateShaderLibrary(
        GraphicsDevice* device,
        const String& path,
        const UnorderedMap<String, Variant<int32, String>>& defines)
    {
        auto shaderSource = LoadShaderSource(path);

        auto [createLibraryResult, library] = device->CreateShaderLibraryFromSource(shaderSource, defines);
        if(createLibraryResult != GraphicsResult::Success && createLibraryResult != GraphicsResult::CompilerWarning)
        {
            HUSKY_ASSERT(false);
            return { false };
        }
        else
        {
            return { true, library };
        }
    }
}
