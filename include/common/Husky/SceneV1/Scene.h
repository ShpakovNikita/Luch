#pragma once

#include <Husky/BaseObject.h>
#include <Husky/RefPtr.h>
#include <Husky/VectorTypes.h>
#include <Husky/Graphics/GraphicsForwards.h>
#include <Husky/SceneV1/SceneV1Forwards.h>
#include <Husky/SceneV1/SceneProperties.h>

namespace Husky::SceneV1
{
    class Scene : public BaseObject
    {
        friend class Node;
        friend class Mesh;
        friend class Primitive;
        friend class PbrMaterial;
    public:
        Scene() = default;
        ~Scene();

        inline const String& GetName() const { return name; }
        inline void SetName(const String& aName) { name = aName; }

        inline const SceneProperties& GetSceneProperties() const { return sceneProperties; }

        inline const RefPtrVector<Node>& GetNodes() const { return nodes; }
        inline void SetNodes(RefPtrVector<Node> aNodes) { nodes = std::move(aNodes); }

        void AddNode(const RefPtr<Node>& node);
        //void RemoveNode(const RefPtr<Node>& node);

        void Update();
private:
        void UpdateNodeRecursive(SceneV1::Node* node, const Mat4x4& transform);

        void AddNodeProperties(const RefPtr<Node>& node);
        void AddMeshProperties(const RefPtr<Mesh>& mesh);
        void AddPrimitiveProperties(const RefPtr<Primitive>& primitive);
        void AddMaterialProperties(const RefPtr<PbrMaterial>& material);
        void AddLightProperties(const RefPtr<Light>& light, const RefPtr<Node>& node);
        void AddCameraProperties(const RefPtr<Camera>& camera, const RefPtr<Node>& node);

        String name;
        RefPtrVector<Node> nodes;
        SceneProperties sceneProperties;
    };
}
