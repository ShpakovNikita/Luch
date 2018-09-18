#include <Husky/Vulkan/VulkanGlslShaderCompiler.h>
#include <Husky/Assert.h>
#include <SPIRV/GlslangToSpv.h>

#include <sstream>

namespace Husky::Vulkan
{

TBuiltInResource CreateDefaultResources()
{
    TBuiltInResource defaultResources;
    defaultResources.maxLights = 32;
    defaultResources.maxClipPlanes = 6;
    defaultResources.maxTextureUnits = 32;
    defaultResources.maxTextureCoords = 32;
    defaultResources.maxVertexAttribs = 64;
    defaultResources.maxVertexUniformComponents = 4096;
    defaultResources.maxVaryingFloats = 64;
    defaultResources.maxVertexTextureImageUnits = 32;
    defaultResources.maxCombinedTextureImageUnits = 80;
    defaultResources.maxTextureImageUnits = 32;
    defaultResources.maxFragmentUniformComponents = 4096;
    defaultResources.maxDrawBuffers = 32;
    defaultResources.maxVertexUniformVectors = 128;
    defaultResources.maxVaryingVectors = 8;
    defaultResources.maxFragmentUniformVectors = 16;
    defaultResources.maxVertexOutputVectors = 16;
    defaultResources.maxFragmentInputVectors = 15;
    defaultResources.minProgramTexelOffset = -8;
    defaultResources.maxProgramTexelOffset = 7;
    defaultResources.maxClipDistances = 8;
    defaultResources.maxComputeWorkGroupCountX = 65535;
    defaultResources.maxComputeWorkGroupCountY = 65535;
    defaultResources.maxComputeWorkGroupCountZ = 65535;
    defaultResources.maxComputeWorkGroupSizeX = 1024;
    defaultResources.maxComputeWorkGroupSizeY = 1024;
    defaultResources.maxComputeWorkGroupSizeZ = 64;
    defaultResources.maxComputeUniformComponents = 1024;
    defaultResources.maxComputeTextureImageUnits = 16;
    defaultResources.maxComputeImageUniforms = 8;
    defaultResources.maxComputeAtomicCounters = 8;
    defaultResources.maxComputeAtomicCounterBuffers = 1;
    defaultResources.maxVaryingComponents = 60;
    defaultResources.maxVertexOutputComponents = 64;
    defaultResources.maxGeometryInputComponents = 64;
    defaultResources.maxGeometryOutputComponents = 128;
    defaultResources.maxFragmentInputComponents = 128;
    defaultResources.maxImageUnits = 8;
    defaultResources.maxCombinedImageUnitsAndFragmentOutputs = 8;
    defaultResources.maxCombinedShaderOutputResources = 8;
    defaultResources.maxImageSamples = 0;
    defaultResources.maxVertexImageUniforms = 0;
    defaultResources.maxTessControlImageUniforms = 0;
    defaultResources.maxTessEvaluationImageUniforms = 0;
    defaultResources.maxGeometryImageUniforms = 0;
    defaultResources.maxFragmentImageUniforms = 8;
    defaultResources.maxCombinedImageUniforms = 8;
    defaultResources.maxGeometryTextureImageUnits = 16;
    defaultResources.maxGeometryOutputVertices = 256;
    defaultResources.maxGeometryTotalOutputComponents = 1024;
    defaultResources.maxGeometryUniformComponents = 1024;
    defaultResources.maxGeometryVaryingComponents = 64;
    defaultResources.maxTessControlInputComponents = 128;
    defaultResources.maxTessControlOutputComponents = 128;
    defaultResources.maxTessControlTextureImageUnits = 16;
    defaultResources.maxTessControlUniformComponents = 1024;
    defaultResources.maxTessControlTotalOutputComponents = 4096;
    defaultResources.maxTessEvaluationInputComponents = 128;
    defaultResources.maxTessEvaluationOutputComponents = 128;
    defaultResources.maxTessEvaluationTextureImageUnits = 16;
    defaultResources.maxTessEvaluationUniformComponents = 1024;
    defaultResources.maxTessPatchComponents = 120;
    defaultResources.maxPatchVertices = 32;
    defaultResources.maxTessGenLevel = 64;
    defaultResources.maxViewports = 16;
    defaultResources.maxVertexAtomicCounters = 0;
    defaultResources.maxTessControlAtomicCounters = 0;
    defaultResources.maxTessEvaluationAtomicCounters = 0;
    defaultResources.maxGeometryAtomicCounters = 0;
    defaultResources.maxFragmentAtomicCounters = 8;
    defaultResources.maxCombinedAtomicCounters = 8;
    defaultResources.maxAtomicCounterBindings = 1;
    defaultResources.maxVertexAtomicCounterBuffers = 0;
    defaultResources.maxTessControlAtomicCounterBuffers = 0;
    defaultResources.maxTessEvaluationAtomicCounterBuffers = 0;
    defaultResources.maxGeometryAtomicCounterBuffers = 0;
    defaultResources.maxFragmentAtomicCounterBuffers = 1;
    defaultResources.maxCombinedAtomicCounterBuffers = 1;
    defaultResources.maxAtomicCounterBufferSize = 16384;
    defaultResources.maxTransformFeedbackBuffers = 4;
    defaultResources.maxTransformFeedbackInterleavedComponents = 64;
    defaultResources.maxCullDistances = 8;
    defaultResources.maxCombinedClipAndCullDistances = 8;
    defaultResources.maxSamples = 4;
    defaultResources.limits.nonInductiveForLoops = 1;
    defaultResources.limits.whileLoops = 1;
    defaultResources.limits.doWhileLoops = 1;
    defaultResources.limits.generalUniformIndexing = 1;
    defaultResources.limits.generalAttributeMatrixVectorIndexing = 1;
    defaultResources.limits.generalVaryingIndexing = 1;
    defaultResources.limits.generalSamplerIndexing = 1;
    defaultResources.limits.generalVariableIndexing = 1;
    defaultResources.limits.generalConstantMatrixVectorIndexing = 1;
    return defaultResources;
}

EShLanguage ToLanguage(ShaderStage shaderStage)
{
    switch (shaderStage) {
    case ShaderStage::Vertex:
        return EShLangVertex;
    case ShaderStage::TessellationControl:
        return EShLangTessControl;
    case ShaderStage::TessellationEvaluation:
        return EShLangTessEvaluation;
    case ShaderStage::Geometry:
        return EShLangGeometry;
    case ShaderStage::Fragment:
        return EShLangFragment;
    case ShaderStage::Compute:
        return EShLangCompute;
    default:
        HUSKY_ASSERT_MSG(false, "Unknown shader stage");
        return EShLangVertex;
    }
}

static TBuiltInResource defaultResources = CreateDefaultResources();

void VulkanGLSLShaderCompiler::Initialize()
{
    glslang::InitializeProcess();
}

void VulkanGLSLShaderCompiler::Deinitialize()
{
    glslang::FinalizeProcess();
}

String VulkanGLSLShaderCompiler::GeneratePreamble(const UnorderedMap<String, String>& flags)
{
    std::stringstream ss;

    for (const auto &kv : flags)
    {
        ss << "#define " << kv.first << " " << kv.second << "\n";
    }

    return ss.str();
}

bool VulkanGLSLShaderCompiler::TryCompileShader(
    ShaderStage shaderStage,
    const Vector<Byte>& glslSource,
    Bytecode& spirvBytecode,
    const UnorderedMap<String, String>& flags)
{
    if (glslSource.empty())
    {
        return false;
    }

    HUSKY_ASSERT(glslSource.back() == Byte{ 0 });
    // TODO logging
    EShLanguage language = ToLanguage(shaderStage);
    glslang::TShader shader(language);
    glslang::TProgram program;

    TBuiltInResource resources{ defaultResources };

    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

    String preamble = GeneratePreamble(flags);
    Array<const char8*, 1> sources = { reinterpret_cast<const char8*>(glslSource.data()) };
    shader.setStrings(sources.data(), (int32)sources.size());
    shader.setPreamble(preamble.c_str());

    if (!shader.parse(&resources, 100, false, messages))
    {
        const auto& log = shader.getInfoLog();
        const auto& debugLog = shader.getInfoLog();
        HUSKY_ASSERT(false);
        return false;
    }

    program.addShader(&shader);

    if (!program.link(messages))
    {
        const auto& log = program.getInfoLog();
        const auto& debugLog = program.getInfoLog();
        HUSKY_ASSERT(false);
        return false;
    }

    glslang::GlslangToSpv(*program.getIntermediate(language), spirvBytecode);
    return true;
}

}
