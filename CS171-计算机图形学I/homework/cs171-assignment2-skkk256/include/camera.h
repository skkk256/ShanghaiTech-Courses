#ifndef CAMERA_H
#define CAMERA_H

#include "defines.h"
#include <vector>

enum WalkDirection { Forward, Backward, Leftward, Rightward };

class Camera {
 public:
  glm::vec3 Position;
  glm::vec3 WorldUp;
  glm::vec3 Front;
  glm::vec3 Up;
  glm::vec3 Right;
  float yaw;
  float pitch;
  float walkSpeed;
  float mouseSensitivity;

  Camera()
      : Position(glm::vec3(0.0f, 0.0f, 20.0f)),
        WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        Front(glm::vec3(0.0f, 0.0f, -1.0f)),
        yaw(-90.0f),
        pitch(0.0f),
        walkSpeed(0.25f),
        mouseSensitivity(0.1f) {
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }

  glm::mat4 getViewMatrix() {
    return glm::lookAt(Position, Position + Front, Up);
  }

  void processWalkAround(WalkDirection direction) {
    switch (direction) {
      case Forward:
        Position += Front * walkSpeed;
        break;
      case Backward:
        Position -= Front * walkSpeed;
        break;
      case Leftward:
        Position -= Right * walkSpeed;
        break;
      case Rightward:
        Position += Right * walkSpeed;
        break;
      default:
        break;
    }
  }

  void processLookAround(float xoff, float yoff) {
    yaw += xoff * mouseSensitivity;
    pitch += yoff * mouseSensitivity;

    if (pitch > 89.9f) pitch = 89.9f;
    if (pitch < -89.0f) pitch = -89.0f;

    updateCameraCoordinate();
  }

 private:
  void updateCameraCoordinate() {
    Front = glm::normalize(
        glm::vec3(cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                  sin(glm::radians(pitch)),
                  sin(glm::radians(yaw)) * cos(glm::radians(pitch))));
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
  }
};

#endif