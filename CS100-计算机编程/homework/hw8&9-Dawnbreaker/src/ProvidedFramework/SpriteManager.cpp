#include "SpriteManager.h"

#ifdef __APPLE__
#include <GLUT/GLUT.h>
#else
#include <GL/glut.h>
#include <GL/freeglut.h>
#endif
#include <SOIL/SOIL.h>

#include "utils.h"
#include <iostream>

//const char* vertexSource = R"glsl(
//	#version 330 core
//	layout (location = 0) in vec3 Pos;
//	layout (location = 2) in vec2 VertexUV;
//
//	out vec2 UV;
//
//	void main()
//	{
//		gl_Position = vec4(Pos, 1.0);
//		UV = VertexUV;
//	}
//)glsl";
//
//const char* fragSource = R"glsl(
//	#version 330 core
//	out vec4 FragColor;
//	
//	in vec2 UV;
//
//	uniform sampler2D Texture;
//
//	void main()
//	{
//		FragColor = texture(Texture, UV);
//	}
//)glsl";

SpriteManager::SpriteManager() : m_filenameMap() {
	m_filenameMap.insert({ IMGID_DAWNBREAKER, ASSET_DIR + "dawnbreaker.png" });
	m_filenameMap.insert({ IMGID_STAR, ASSET_DIR + "star.png" });
	m_filenameMap.insert({ IMGID_ALPHATRON, ASSET_DIR + "alphatron.png" });
	m_filenameMap.insert({ IMGID_SIGMATRON, ASSET_DIR + "sigmatron.png" });
	m_filenameMap.insert({ IMGID_OMEGATRON, ASSET_DIR + "omegatron.png" });
	m_filenameMap.insert({ IMGID_BLUE_BULLET, ASSET_DIR + "blueBullet.png" });
	m_filenameMap.insert({ IMGID_RED_BULLET, ASSET_DIR + "redBullet.png" });
	m_filenameMap.insert({ IMGID_EXPLOSION, ASSET_DIR + "explosion.png" });
	m_filenameMap.insert({ IMGID_METEOR, ASSET_DIR + "meteor.png" });
	m_filenameMap.insert({ IMGID_POWERUP_GOODIE, ASSET_DIR + "powerUpGoodie.png" });
	m_filenameMap.insert({ IMGID_METEOR_GOODIE, ASSET_DIR + "meteorGoodie.png" });
	m_filenameMap.insert({ IMGID_HP_RESTORE_GOODIE, ASSET_DIR + "recoveryGoodie.png" });
	LoadSprites();
}

bool SpriteManager::LoadSprites(){
	glEnable(GL_DEPTH_TEST);
	for (auto& asset : m_filenameMap) {

		int width, height;
		unsigned char* image = SOIL_load_image(asset.second.c_str(), &width, &height, 0, SOIL_LOAD_RGBA);

		//GLuint texture;
		//glGenTextures(1, &texture);
		//glBindTexture(GL_TEXTURE_2D, texture);
		//glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		////glGenerateMipmaps(GL_TEXTURE_2D);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		//gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
		//SOIL_free_image_data(image);

		GLuint texture = SOIL_load_OGL_texture(asset.second.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
																					 SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT);
		if (0 == texture) {
			printf("SOIL loading error: '%s'\n", SOIL_last_result());
		}

		m_textureMap.insert({ asset.first, texture });
	}
	return true;
}

GLuint SpriteManager::GetTexture(ImageID imageID) {
	auto it = m_textureMap.find(imageID);
	if (it == m_textureMap.end()) {
		return 0;
	}
	else {
		return it->second;
	}
}

