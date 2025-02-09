#ifndef SPRITEMANAGER_H__
#define SPRITEMANAGER_H__

#include <string>
#include <map>

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif

using ImageID = int;

class SpriteManager {
public:
  // Mayers' singleton pattern
  virtual ~SpriteManager() {}
  SpriteManager(const SpriteManager& other) = delete;
  SpriteManager& operator=(const SpriteManager& other) = delete;
  static SpriteManager& Instance() { static SpriteManager instance; return instance; }

  GLuint GetTexture(ImageID imageID);


private:
  SpriteManager();

  bool LoadSprites();

  std::map<ImageID, std::string> m_filenameMap;
  std::map<ImageID, GLuint> m_textureMap;


};
#endif // !SPRITEMANAGER_H__
