#include "ObjectBase.h"
#include "GameManager.h"

ObjectBase::ObjectBase(int imageID, int x, int y, int direction, int layer, double size)
  : m_imageID(imageID), m_x(x), m_y(y), m_direction(direction), m_layer(layer), m_size(size) {
  GetObjects(m_layer).insert(this);
}

//ObjectBase::ObjectBase(const ObjectBase& other)
//  : m_imageID(other.m_imageID), m_x(other.m_x), m_y(other.m_y), m_direction(other.m_direction), m_layer(other.m_layer), m_size(other.m_size) {
//  GetObjects(m_layer).insert(this);
//}
//
//ObjectBase::ObjectBase(ObjectBase&& other) noexcept
//  : m_imageID(other.m_imageID), m_x(other.m_x), m_y(other.m_y), m_direction(other.m_direction), m_layer(other.m_layer), m_size(other.m_size) {
//  GetObjects(m_layer).insert(this);
//}
//
//ObjectBase& ObjectBase::operator=(const ObjectBase& other) {
//  GetObjects(m_layer).erase(this);
//  m_imageID = other.m_imageID; 
//  m_x = other.m_x; 
//  m_y = other.m_y;
//  m_direction = other.m_direction;
//  m_layer = other.m_layer;
//  m_size = other.m_size;
//  GetObjects(m_layer).insert(this);
//  return *this;
//}
//
//ObjectBase& ObjectBase::operator=(ObjectBase&& other) noexcept {
//  GetObjects(m_layer).erase(this);
//  m_imageID = other.m_imageID; 
//  m_x = other.m_x; 
//  m_y = other.m_y;
//  m_direction = other.m_direction;
//  m_layer = other.m_layer;
//  m_size = other.m_size;
//  GetObjects(m_layer).insert(this);
//  return *this;
//}

ObjectBase::~ObjectBase() {
  GetObjects(m_layer).erase(this);
}

bool ObjectBase::operator==(const ObjectBase& other) {
  return this == &other;
}

int ObjectBase::GetX() const {
  return m_x;
}

int ObjectBase::GetY() const {
  return m_y;
}

int ObjectBase::GetDirection() const {
  return m_direction % 360;
}

int ObjectBase::GetLayer() const {
  return m_layer;
}

double ObjectBase::GetSize() const {
  return m_size;
}

void ObjectBase::MoveTo(int x, int y) {
  m_x = x;
  m_y = y;
}

void ObjectBase::SetDirection(int direction) {
  m_direction = direction % 360;
}

void ObjectBase::SetSize(double size) {
  m_size = size;
}

std::set<ObjectBase*>& ObjectBase::GetObjects(int layer) {
  static std::set<ObjectBase*> gameObjects[MAX_LAYERS];
  if (layer < MAX_LAYERS) {
    return gameObjects[layer];
  }
  else {
    return gameObjects[0];
  }
}
