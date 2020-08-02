#pragma once

#include <SDL.h>
#include <SDL_image.h>
#include <box2d.h>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include <string>
#include <iostream>
#include <unordered_map>

#define DANGEROUS_TILE 3
#define LADDER 4
#define FINISH_POINT 5
#define MOVING_PLATFORM 6
#define BUTTON 7
#define ENTITY 8

using namespace std;

// This is a datatype for a tile.
struct Tile {
	// We need to know the GID of the tileset this tile belongs to
	int tilesetGID;

	// These hold the tile position. The position isnt really where the tile is, but its the number of the tile.
	// For example x=0, y=0 would be the first tile in the map.
	int x;
	int y;

	// This is the rect used for extracting the tile out of the image
	SDL_Rect spriteRect;
};

// An entity is something that the player can interact with, like a box or a ball.
// An entity has a b2Body in the physics world, and it has a tile that is rendered
struct Entity {
	// We need to know the GID of the tileset this tile belongs to
	int tilesetGID;
	// This is the rect used for extracting the tile out of the image
	SDL_Rect spriteRect;

	// We need this for getting the position and angle of the entity when rendering
	b2Body* entityBody;
};

// Similar to an entity but also has movement boundaries
struct MovingPlatform {
	// We need to know the GID of the tileset this tile belongs to
	int tilesetGID;
	// This is the rect used for extracting the tile out of the image
	SDL_Rect spriteRect;

	// We need this for getting the position and angle of the entity when rendering
	b2Body* entityBody;
	// 1 for horizontal, 2 for vertical and 3 for diagonal
	int movementType;

	bool usesButton;
	// Used by the boundary checker to decide if it should check the platform
	bool active;

	// Need this to check if the platform should reverse direction. The first vector are the leftand right boundaries respectively.
	// Same thing for the second vector, but it's top and bottom (respectively).
	b2Vec2 xMovementBoundaries;
	b2Vec2 yMovementBoundaries;

	// The horizontal and vertical speed need to be separate because of diagonal movement. If they are the smae, the platform may reach its vertical
	// destination at a different time to the horizotnal destination which makes it move in a wierd pattern
	b2Vec2 speed;
	// The direction will just hold the current direction of the platform. 1 if it is moving right/up and -1 if it is moving left/down.
	// The horizontal and vertical directions are separate.
	b2Vec2 direction;
};

class GameLevel {
public:
	GameLevel();
	~GameLevel();

	bool load(int screenWidth, int screenHeight, SDL_Renderer* ren, const char* filename, b2World* world);
	void render(float camXOffset, float camYOffset);
	void createHitboxes(b2World* world);

	// This will change the direction of the platform if it has reached its boundaries, and stop/start the platform if needed
	void doMovingPlatformLogic(unordered_map<int, int> buttons);
	// The direction is needed for setting the platforms velocity and adding that velocity to the player
	enum class MPDirections {NOT_SET, HORIZONTAL, VERTICAL, DIAGONAL};

	void dumpMovingPlatformData(bool param, MovingPlatform* mp);

private:
	// The map data
	int width = 0;
	int height = 0;
	bool useObjectCollisions = false;

	vector<Tile> tiles;
	vector<Entity> entities;
	unordered_map<int, MovingPlatform> movingPlatforms;
	vector<tmx::Object> collisionObjects;

	// This will be an map of tilesets that this level contains. A tileset element will have a first GID, and then a pair of last GID and sprite sheet texture
	map<int, pair<int, SDL_Texture*>> tilesets;

	// Dimensions of screen
	int SCREEN_WIDTH = 0;
	int SCREEN_HEIGHT = 0;

	// We need a pointer to the renderer to render and create textures for this level
	SDL_Renderer* renderer = NULL;

	void createEntity(tmx::Object* entityObject, b2World* world, bool movingPlatform, unordered_map<string, tmx::Property> objectProperties);

	// Checks if a tile is inside the camera boundaries
	bool isTileInRect(SDL_Rect* tileRect);
	// When rendering from a tilesheet we need coordinates to extract a specific tile. Thats what this function returns
	bool getTileSourceRect(int tileGID, int* tset_gid, SDL_Rect* outputRect);
};