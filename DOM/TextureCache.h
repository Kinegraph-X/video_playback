#pragma once
#include "player_headers.h"
#include "Node.h"

class TextureCache {
private:
    std::map<std::string, Texture2D> cache;
    std::mutex cacheMutex;
    const size_t MAX_CACHE_SIZE = 50;

public:
	void preloadTextures(const std::vector<std::string>& filenames);
    Texture2D getTexture(const std::string& filename);

    ~TextureCache();
};

