#pragma once
#ifndef TEXTURE_HOLDER_H
#define TEXTURE_HOLDER_H
#include<SFML/Graphics.hpp>
#include<map>

using namespace sf;

//SINGLETON DESIGN PATTERN!

class TextureHolder {
private:
	//A map container from STL that holds pairs of string and texture
	std::map<std::string, Texture> m_Textures;
	//A pointer of the same type as the class itself, the one and only instance
	//No matter how many zombies we spawn, there will be a maximum of three textures in the memory of the GPU
	static TextureHolder* m_s_Instance;
public:
	TextureHolder();
	static Texture& GetTexture(std::string const& filename);
};

#endif