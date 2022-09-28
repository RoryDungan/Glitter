#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class ArcCamera {
public:
    ArcCamera(const glm::vec3& centre, float distance, float initialPitch, float initialYaw) 
        : centre(centre), distance(distance), pitch(initialPitch), yaw(initialYaw) {}

    glm::mat4 GetViewMatrix() const;

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
