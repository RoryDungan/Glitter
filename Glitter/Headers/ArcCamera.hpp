#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ArcCamera {
public:
    ArcCamera(const glm::vec3& centre, float distance, float initialPitch, float initialYaw) 
        : centre(centre), distance(distance), pitch(initialPitch), yaw(initialYaw) {

    }

    glm::mat4 GetViewMatrix() const {
        auto initialPos = glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.f, distance));
        auto pitchMat = glm::rotate(glm::identity<glm::quat>(), -pitch, glm::vec3(1.f, 0.f, 0.f));
        auto yawMat = glm::rotate(glm::identity<glm::quat>(), yaw, glm::vec3(0.f, 1.f, 0.f));
        auto rotationMat = glm::mat4_cast(yawMat * pitchMat);
        auto finalPos = glm::translate(glm::mat4(1.f), centre) * rotationMat * initialPos;
        return glm::inverse(finalPos);
    }

    void Yaw(float delta) {
        yaw += delta;
    }

    void Pitch(float delta) {
        pitch += delta;
    }

private:
    glm::vec3 centre;
    float distance, pitch, yaw;
};
