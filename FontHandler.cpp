#include "FontHandler.h"

FontHandler::FontHandler(SDL_Renderer* ren) {
	renderer = ren;
}

FontHandler::~FontHandler() {
	cout << "Font dc called" << endl;

	// Loop through all of the fonts and clear the memory taken by the texture
	for (auto const& font : fonts) {
		for (auto charTexturePair : font.second.textures) {
			SDL_DestroyTexture(charTexturePair.second);
			charTexturePair.second = NULL;
		}
	}
}

bool FontHandler::loadFont(string fontIdentifier, const char* fontFilename, int fontSize)
{
	// The font indentifier should be unique, so we check to see if we already have one with this name
	if (fonts.count(fontIdentifier) > 0) return false;

	// We need to load the font once at the start
	TTF_Font* font = NULL;
	font = TTF_OpenFont(fontFilename, fontSize);
	// Check for unsuccesful font loads
	if (font == NULL) return false;

	// Get the information of the font for later use
	FH_Font fhFont;
	TTF_SizeText(font, "G", &fhFont.width, &fhFont.height);

	// Now we go through every character and get a texture from it
	for (int i = 0; i < alphabet.length(); i++) {
		string singularChar(1, alphabet[i]);

		// FIrst, get the surface for the current character
		SDL_Color fontColor = { 0, 0, 0, 255 };
		SDL_Surface* characterSurface = TTF_RenderText_Solid(font, singularChar.c_str(), fontColor);

		// Now we can convert it to a texture for easy rendering
		SDL_Texture* characterTexture = SDL_CreateTextureFromSurface(renderer, characterSurface);

		// Now that we have a texture, we no longer need the old surface
		SDL_FreeSurface(characterSurface);
		characterSurface = NULL;

		// Add the texture to the map for rendering later
		fhFont.textures.insert(make_pair(alphabet[i], characterTexture));
	}

	fonts.insert(make_pair(fontIdentifier, fhFont));

	// We no longer need the font
	TTF_CloseFont(font);
	font = NULL;

	return true;
}

void FontHandler::renderFont(string fontIdentifier, string text, float x, float y) {
	// For FPS testing
	// return;

	if (fonts.count(fontIdentifier) < 1) return;
	FH_Font font = fonts[fontIdentifier];

	text += "\n";
	float originalX = x;

	// These will help split the string into substrings for multiline rendering
	vector<string> substrings;
	stringstream ss(text);
	string tempSubstring;

	while (getline(ss, tempSubstring, '\n'))
		substrings.push_back(tempSubstring);

	for (int i = 0; i < substrings.size(); i++) {
		// The text is to be drawn centered, so we want to set back its position by the correct amount
		x -= font.width * substrings[i].size() / 2;
		// Only set back the y for the first line, because the other lines will automatically be correctly aligned
		if(i == 0)
			y -= font.height * substrings.size() / 2;

		// Go through each letter of the text
		for (int c = 0; c < substrings[i].size(); c++) {
			// The rectangle that decides where to render the texture on the screen
			SDL_Rect destinationRect = { (int)x, (int)y, font.width, font.height };
			SDL_RenderCopy(renderer, font.textures[substrings[i][c]], NULL, &destinationRect);

			// Now that we've rendered, we need to increase the starting position for the next character
			x += font.width;
		}

		y += font.height;
		x = originalX;
	}
}