#pragma once
#include "TextureCache.h"

void TextureCache::preloadTextures(const std::vector<std::string>& filenames) {
    for (const auto& filename : filenames) {
        getTexture(filename);  // This will load and cache the texture
    }
}


Texture2D TextureCache::getTexture(const std::string& filename) {
	std::lock_guard<std::mutex> lock(cacheMutex);
	namespace fs = std::filesystem;

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
    
    std::string resolvedFilename = GetExecutablePath() + "/" + filename;
    logger(LogLevel::DEBUG, "Resolved texture filename : " + resolvedFilename);
    if (fs::exists(resolvedFilename)) {
	    // Texture not found in cache, load it
	    Texture2D texture = RaylibLoadTexture(resolvedFilename.c_str());
	    cache[filename] = texture;
	    return texture;
    }
    else {
		// Fallback: Create a checkered texture
		logger(LogLevel::ERR, "Failed to load texture from file : " + filename);
	    RaylibImage fallback = RaylibGenImageChecked(32, 32, 8, 8, RAYLIB_DARKGRAY, RAYLIB_LIGHTGRAY); // Checkerboard pattern
	    Texture2D fallbackTexture = RaylibLoadTextureFromImage(fallback);
	    RaylibUnloadImage(fallback);
	    return fallbackTexture;
	}
}

TextureCache::~TextureCache() {
    for (auto& pair : cache) {
        RaylibUnloadTexture(pair.second);
    }
}

