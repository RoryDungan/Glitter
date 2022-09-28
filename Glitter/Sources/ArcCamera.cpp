#include "ArcCamera.hpp"

using namespace glm;

mat4 ArcCamera::GetViewMatrix() const {
    auto initialPos = translate(mat4(1.f), vec3(0.f, 0.f, distance));
    auto pitchMat = rotate(identity<quat>(), -pitch, vec3(1.f, 0.f, 0.f));
    auto yawMat = rotate(identity<quat>(), yaw, vec3(0.f, 1.f, 0.f));
    auto rotationMat = mat4_cast(yawMat * pitchMat);
    auto finalPos = translate(mat4(1.f), centre) * rotationMat * initialPos;
    return inverse(finalPos);
}