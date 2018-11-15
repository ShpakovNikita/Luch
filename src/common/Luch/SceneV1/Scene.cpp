#include <Luch/SceneV1/Scene.h>
#include <Luch/SceneV1/Node.h>
#include <Luch/SceneV1/Mesh.h>
#include <Luch/SceneV1/Light.h>
#include <Luch/SceneV1/Primitive.h>
#include <Luch/SceneV1/PbrMaterial.h>
#include <Luch/SceneV1/Camera.h>

namespace Luch::SceneV1
{
    Scene::~Scene() = default;

    void Scene::AddNode(const RefPtr<Node>& node)
    {
        [[maybe_unused]] auto it = std::find(nodes.begin(), nodes.end(), node);
        LUCH_ASSERT(it == nodes.end());

        nodes.push_back(node);
        AddNodeProperties(node);
    }

    void Scene::Update()
    {
        Mat4x4 identity = glm::mat4(1.0f);

        for (const auto& node : GetNodes())
        {
            UpdateNodeRecursive(node, identity);
        }
    }

    void Scene::UpdateNodeRecursive(SceneV1::Node* node, const Mat4x4& parentTransform)
    {
        Mat4x4 localTransformMatrix;
        const auto& localTransform = node->GetLocalTransform();

        if (std::holds_alternative<Mat4x4>(localTransform))
        {
            localTransformMatrix = std::get<Mat4x4>(localTransform);
        }
        else if(std::holds_alternative<SceneV1::TransformProperties>(localTransform))
        {
            const auto& transformProperties = std::get<SceneV1::TransformProperties>(localTransform);

            localTransformMatrix
                = glm::translate(transformProperties.translation)
                * glm::toMat4(transformProperties.rotation)
                * glm::scale(transformProperties.scale);
        }

        Mat4x4 worldTransform = parentTransform * localTransformMatrix;
        node->SetWorldTransform(worldTransform);

        for (const auto& child : node->GetChildren())
        {
            UpdateNodeRecursive(child, worldTransform);
        }
    }

    void Scene::AddNodeProperties(const RefPtr<Node>& node)
    {
        LUCH_ASSERT(node->scene == nullptr);

        node->scene = this;

        const auto& mesh = node->GetMesh();
        if(mesh != nullptr)
        {
            AddMeshProperties(mesh);
        }

        const auto& light = node->GetLight();
        if(light != nullptr)
        {
            AddLightProperties(light, node);
        }

        const auto& camera = node->GetCamera();
        if(camera != nullptr)
        {
            AddCameraProperties(camera, node);
        }

        for(const auto& child : node->GetChildren())
        {
            AddNodeProperties(child);
        }
    }

    void Scene::AddMeshProperties(const RefPtr<Mesh>& mesh)
    {
        sceneProperties.meshes.insert(mesh);

        for (const auto& primitive : mesh->GetPrimitives())
        {
            AddPrimitiveProperties(primitive);
        }
    }

    void Scene::AddPrimitiveProperties(const RefPtr<Primitive>& primitive)
    {
        sceneProperties.primitives.insert(primitive);

        for(const auto& vertexBuffer : primitive->GetVertexBuffers())
        {
            sceneProperties.buffers.insert(vertexBuffer.backingBuffer);
        }

        if(primitive->GetIndexBuffer().has_value())
        {
            sceneProperties.buffers.insert(primitive->GetIndexBuffer()->backingBuffer);
        }

        const auto& material = primitive->GetMaterial();
        if (material != nullptr)
        {
            AddMaterialProperties(material);
        }
    }

    void Scene::AddMaterialProperties(const RefPtr<PbrMaterial>& material)
    {
        sceneProperties.materials.insert(material);

        if (material->HasBaseColorTexture())
        {
            sceneProperties.textures.insert(material->GetBaseColorTexture());
        }

        if (material->HasMetallicRoughnessTexture())
        {
            sceneProperties.textures.insert(material->GetMetallicRoughnessTexture());
        }

        if (material->HasNormalTexture())
        {
            sceneProperties.textures.insert(material->GetNormalTexture());
        }

        if (material->HasOcclusionTexture())
        {
            sceneProperties.textures.insert(material->GetOcclusionTexture());
        }

        if (material->HasEmissiveTexture())
        {
            sceneProperties.textures.insert(material->GetEmissiveTexture());
        }
    }

    void Scene::AddLightProperties(const RefPtr<Light>& light, const RefPtr<Node>& node)
    {
        sceneProperties.lights.insert(light);
        sceneProperties.lightNodes.insert(node);
    }

    void Scene::AddCameraProperties(const RefPtr<Camera>& camera, const RefPtr<Node>& node)
    {
        sceneProperties.cameras.insert(camera);
        sceneProperties.cameraNodes.insert(node);
    }
}
