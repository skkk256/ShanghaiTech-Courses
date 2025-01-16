#include "camera.hpp"

#include <string>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

FirstPersonCamera::
FirstPersonCamera() {
    this->cameraPos = glm::vec3(0.0f, 0.0f, 6.0f);
    this->cameraForward = glm::vec3(0.0f, 0.0f, -1.0f);
    this->cameraRight = glm::vec3(1.0f, 0.0f, 0.0f);
    this->cameraLocalUp = glm::vec3(0.0f, 1.0f, 0.0f);
    this->cameraGlobalUp = glm::vec3(0.0f, 1.0f, 0.0f); // world up

    this->fov = 1.0f;
    this->aspect = 1;
    this->nearClipDist = 0.1f;
    this->farClipDist = 100.0f;
}

void FirstPersonCamera::
setAspect(float _aspect) {
    this->aspect = _aspect;
}

void FirstPersonCamera::
setCamera(float _fov, float _aspect, float _nearClipDist, float _farClipDist) {
    this->fov = _fov;
    this->aspect = _aspect;
    this->nearClipDist = _nearClipDist;
    this->farClipDist = _farClipDist;
}

glm::mat4 FirstPersonCamera::
getProjection() {
    return glm::perspective<float>(
        this->fov,
        this->aspect,
        this->nearClipDist,
        this->farClipDist
    );
}

glm::mat4 FirstPersonCamera::
getView() {
    return glm::lookAt<float> (
        this->cameraPos,
        this->cameraPos + this->cameraForward,
        this->cameraLocalUp
    );
}

glm::vec3 FirstPersonCamera::
getCameraPos() {
    return this->cameraPos;
}

void FirstPersonCamera::
moveForward(float _dist) {
    this->cameraPos = this->cameraPos + _dist * this->cameraForward;
}

void FirstPersonCamera::
moveBackward(float _dist) {
    this->cameraPos = this->cameraPos - _dist * this->cameraForward;
}

void FirstPersonCamera::
moveLeft(float _dist) {
    this->cameraPos = this->cameraPos - _dist * this->cameraRight;
}

void FirstPersonCamera::
moveRight(float _dist) {
    this->cameraPos = this->cameraPos + _dist * this->cameraRight;
}

void FirstPersonCamera::
moveUp(float _dist) {
    this->cameraPos = this->cameraPos + _dist * this->cameraGlobalUp;
}

void FirstPersonCamera::
moveDown(float _dist) {
   this->cameraPos = this->cameraPos - _dist * this->cameraGlobalUp;
}

void FirstPersonCamera::
lookRight(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), _angle, this->cameraGlobalUp);
    this->cameraForward = rotation * this->cameraForward;
    this->cameraRight = glm::cross(this->cameraForward, this->cameraGlobalUp);
    this->cameraLocalUp = glm::cross(this->cameraRight, this->cameraForward);
}

void FirstPersonCamera::
lookLeft(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), -_angle, this->cameraGlobalUp);
    this->cameraForward = rotation * this->cameraForward;
    this->cameraRight = glm::cross(this->cameraForward, this->cameraGlobalUp);
    this->cameraLocalUp = glm::cross(this->cameraRight, this->cameraForward);
}

void FirstPersonCamera::
lookUp(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), _angle, this->cameraRight);
    this->cameraForward = rotation * this->cameraForward;
    this->cameraLocalUp = glm::cross(this->cameraRight, this->cameraForward);
}

void FirstPersonCamera::
lookDown(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), -_angle, this->cameraRight);
    this->cameraForward = rotation * this->cameraForward;
    this->cameraLocalUp = glm::cross(this->cameraRight, this->cameraForward);
}

void FirstPersonCamera::
rotateRight(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), _angle, this->cameraForward);
    this->cameraLocalUp = rotation * this->cameraLocalUp;
    this->cameraRight = glm::cross(this->cameraForward, this->cameraLocalUp);
}

void FirstPersonCamera::
rotateLeft(float _angle) {
    glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), -_angle, this->cameraForward);
    this->cameraLocalUp = rotation * this->cameraLocalUp;
    this->cameraRight = glm::cross(this->cameraForward, this->cameraLocalUp);
}

void FirstPersonCamera::
print() {
//     std::cout << "Pos    :" << glm::to_string(this->cameraPos) << std::endl;
//     std::cout << "Right  :" << glm::to_string(this->cameraRight) << std::endl;
//     std::cout << "Up     :" << glm::to_string(this->cameraLocalUp) << std::endl;
//     std::cout << "Forward:" << glm::to_string(this->cameraForward) << std::endl;
//     std::cout << std::endl;
}