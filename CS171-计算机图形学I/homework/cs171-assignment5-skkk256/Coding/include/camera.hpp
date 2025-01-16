#pragma once
#include <glm/glm.hpp>

class FirstPersonCamera {
private:
    glm::vec3 cameraPos;
    glm::vec3 cameraForward;
    glm::vec3 cameraRight;
    glm::vec3 cameraGlobalUp;
    glm::vec3 cameraLocalUp;

    float fov;
    float aspect;
    float nearClipDist;
    float farClipDist;

public:
    FirstPersonCamera();

    glm::mat4 getProjection();
    glm::mat4 getView();
    glm::vec3 getCameraPos();

    void setAspect(float aspect);
    void setCamera(float fov, float aspect, float nearClipDist, float farClipDist);

    void moveForward(float dist);
    void moveBackward(float dist);
    void moveLeft(float dist);
    void moveRight(float dist);
    void moveUp(float dist);
    void moveDown(float dist);

    void lookUp(float angle);
    void lookDown(float angle);
    void lookLeft(float angle);
    void lookRight(float angle);
    void rotateRight(float angle);
    void rotateLeft(float angle);

    void print();
};