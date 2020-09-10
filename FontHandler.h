#pragma once

#include <unordered_map>
#include <vector>
#include <sstream>
#include <string>
#include <iostream>

#include <SDL.h>
#include <SDL_ttf.h>

using namespace std;

// A struct for holding data about a font
struct FH_Font {
	int width;
	int height;
	unordered_map<char, SDL_Texture*> textures;
};

class FontHandler
{
public:
	FontHandler(SDL_Renderer* ren);
	~FontHandler();
	bool loadFont(string fontIdentifier, const char* fontFilename, int fontSize);
	void renderFont(string fontIdentifier, string text, float x, float y);

private:
	// This will hold the texture for each character of each font that is loaded
	unordered_map<string, FH_Font> fonts;

	// The renderer pointer that is needed for creating textures and rendering them
	SDL_Renderer* renderer = NULL;

	// An alphabet (with numbers) that we can loop through to get stuff
	string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ 1234567890?!-:/.";
};