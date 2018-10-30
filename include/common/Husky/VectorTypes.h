#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <glm/mat2x2.hpp>
#include <glm/mat2x3.hpp>
#include <glm/mat2x4.hpp>

#include <glm/mat3x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat3x4.hpp>

#include <glm/mat4x2.hpp>
#include <glm/mat4x3.hpp>
#include <glm/mat4x4.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_operation.hpp>

namespace Husky
{
    using Vec2 = glm::vec2;
    using Vec3 = glm::vec3;
    using Vec4 = glm::vec4;

    using Mat2x2 = glm::mat2x2;
    using Mat2x3 = glm::mat2x3;
    using Mat2x4 = glm::mat2x4;

    using Mat3x2 = glm::mat3x2;
    using Mat3x3 = glm::mat3x3;
    using Mat3x4 = glm::mat3x4;

    using Mat4x2 = glm::mat4x2;
    using Mat4x3 = glm::mat4x3;
    using Mat4x4 = glm::mat4x4;

    using Quaternion = glm::fquat;
}
