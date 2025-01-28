#pragma once
#include "TextureCache.h"

void TextureCache::preloadTextures(const std::vector<std::string>& filenames) {
    for (const auto& filename : filenames) {
        getTexture(filename);  // This will load and cache the texture
    }
}


Texture2D TextureCache::getTexture(const std::string& filename) {
	std::lock_guard<std::mutex> lock(cacheMutex);

	// Cache size management, before adding new texture
	if (cache.size() >= MAX_CACHE_SIZE) {
	    // Implement cache eviction policy (e.g., remove least recently used)
	    auto it = cache.begin();
	    RaylibUnloadTexture(it->second);
	    cache.erase(it->first);
	}
	
    auto it = cache.find(filename);
    if (it != cache.end()) {
        return it->second;
    }
    
    // Texture not found in cache, load it
    Texture2D texture = RaylibLoadTexture(filename.c_str());
    cache[filename] = texture;
    return texture;
}

TextureCache::~TextureCache() {
    for (auto& pair : cache) {
        RaylibUnloadTexture(pair.second);
    }
}

