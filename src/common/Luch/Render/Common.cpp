#include <Luch/Render/Common.h>
#include <Luch/Render/ShaderDefines.h>

namespace Luch::Render
{
    using namespace Luch::Graphics;

    const UnorderedMap<SceneV1::AttributeSemantic, int32> SemanticToLocation = 
    {
        { SceneV1::AttributeSemantic::Position, 0 },
        { SceneV1::AttributeSemantic::Normal, 1 },
        { SceneV1::AttributeSemantic::Tangent, 2 },
        { SceneV1::AttributeSemantic::Texcoord_0, 3 },
        { SceneV1::AttributeSemantic::Texcoord_1, 4 },
        { SceneV1::AttributeSemantic::Color_0, 5 },
    };

    const UnorderedMap<SceneV1::AttributeSemantic, String> SemanticToFlag =
    {
        { SceneV1::AttributeSemantic::Position, {} },
        { SceneV1::AttributeSemantic::Normal, "HAS_NORMAL" },
        { SceneV1::AttributeSemantic::Tangent, "HAS_TANGENT" },
        { SceneV1::AttributeSemantic::Texcoord_0, "HAS_TEXCOORD_0" },
    };
}
