#ifndef OBJECTBASE_H__
#define OBJECTBASE_H__

#include <iostream>
#include <set>

#include "utils.h"

class ObjectBase {
public:
  ObjectBase(int imageID, int x, int y, int direction, int layer, double size);
  ObjectBase(const ObjectBase& other) = delete;
  ObjectBase(ObjectBase&& other) = delete;
  ObjectBase& operator=(const ObjectBase& other) = delete;
  ObjectBase& operator=(ObjectBase&& other) = delete;
  virtual ~ObjectBase();

  virtual bool operator==(const ObjectBase& other);

  virtual void Update() = 0;

  int GetX() const;
  int GetY() const;
  int GetDirection() const;
  int GetLayer() const;
  double GetSize() const;

  void MoveTo(int x, int y);
  void SetDirection(int direction);
  void SetSize(double size);

private:
  int m_imageID;
  int m_x;
  int m_y;
  int m_direction;
  int m_layer;
  double m_size;

public:
  template<typename Func>
  static void DisplayAllObjects(Func displayFunc) {
    for (int layer = MAX_LAYERS - 1; layer >= 0; layer--) {
      for (auto& obj : GetObjects(layer)) {
        displayFunc(obj->m_imageID, obj->m_x, obj->m_y, obj->m_direction, obj->m_size);
      }
    }
  }
private:
  static std::set<ObjectBase*>& GetObjects(int layer);

};


#endif // !OBJECTBASE_H__
