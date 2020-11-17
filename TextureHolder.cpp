#include"TextureHolder.h"
//Include the "assert feature"
#include<assert.h>

TextureHolder* TextureHolder::m_s_Instance = nullptr;
TextureHolder::TextureHolder() {
	assert(m_s_Instance == nullptr);	//ensures that m_s_Instance is a nullptr, otherwise game exits execution
	m_s_Instance = this;				// now we have a pointer to a TextureHolder that points to the one and only instance of itself
}

Texture& TextureHolder::GetTexture(std::string const& filename) {
	auto& m = m_s_Instance->m_Textures;		//auto == map<string, Texture>
	//Create an iterator to hold a kvp and search for the required kvp using the passed in filename
	auto keyValuePair = m.find(filename);	//auto == map<string, Texture>::iterator
	if (keyValuePair != m.end())
	{
		//If found, return the texture
		return keyValuePair->second;
	}
	else
	{
		//Create a new kvp using the filename
		auto& texture = m[filename];
		//Load the texture from file in the usual way
		texture.loadFromFile(filename);
		//Return the texture to the calling code
		return texture;
	}
}
