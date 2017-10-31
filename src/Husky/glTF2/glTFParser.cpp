#include <Husky/glTF2/glTFParser.h>
#include <Husky/glTF2/glTF.h>
#include <Husky/Stream.h>
#include <json.hpp>

namespace Husky::glTF
{

using json = nlohmann::json;

Vector<String> ParseStringArray(const json& j)
{
    Vector<String> strings;
    for (auto str : j)
    {
        strings.emplace_back(str.get<String>());
    }
    return strings;
}

Asset ParseAsset(const json& j)
{
    Asset result;
    result.copyright = j.value("copyright", "");
    result.generator = j.value("generator", "");
    result.version = j["version"].get<String>();
    result.minVersion = j.value("minVersion", "");
    return result;
}

UniquePtr<glTF> glTFParser::ParseJSON(Stream* stream)
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

    UniquePtr<glTF> root = MakeUnique<glTF>();

    auto assetIter = j.find("asset");

    HUSKY_ASSERT(assetIter != j.end());


    auto extensionsUsedIter = j.find("extensionsUsed");
    if (extensionsUsedIter != j.end())
    {
        root->extensionsUsed = ParseStringArray(*extensionsUsedIter);
    }

    auto extensionsRequiredIter = j.find("extensionsRequired");
    if (extensionsRequiredIter != j.end())
    {
        root->extensionsRequired = ParseStringArray(*extensionsRequiredIter);
    }




    root->extensionsUsed = 
    root->asset = ParseAsset(*assetIter);

}

}
