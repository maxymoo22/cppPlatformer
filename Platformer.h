#pragma once

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <box2d.h>

#include "GameLevel.h"
#include "FontHandler.h"
#include "Box2dOverrides.h"
#include "AudioHandler.h"

#define PLAYER_BODY 1
#define PLAYER_SENSOR 2

// These constants are the minimum distances that need to be between the playerand the edge of the viewport
// They will be used in calculating when and by how much to scroll the vieport
#define PLAYER_SPRITE_LR_MARGIN (int)(0.3 * SCREEN_WIDTH)
#define PLAYER_SPRITE_BOTTOM_MARGIN (int)(0.34 * SCREEN_HEIGHT)
#define PLAYER_SPRITE_TOP_MARGIN (int)(0.266666 * SCREEN_HEIGHT)

//#define MOBILE
#undef MOBILE

using namespace std;

class Platformer {
public:
	//The window we'll be rendering to
	SDL_Window* window;

	bool init();
	bool loadAssets();
	void loop();

	Platformer();
	// When the app ends this destructor will be called and release all of the used memory
	~Platformer();

private:
	int SCREEN_WIDTH = 1000;
	int SCREEN_HEIGHT = 750;
	int TILE_SIZE;
	int REFRESH_RATE;

	// The window renderer. Needed for rendering textures
	SDL_Renderer* renderer;
	// Event handler
	SDL_Event eventHandler;

	// The player texture
	SDL_Texture* player;
	// Since the playe rhas multiple animation frames, we need to store an index to the current one. This is incremented by 32 (each texture is 32x32 px)
	int playerTextureXOffset;
	float animationFrameIndex;
	bool playerDirection;

	SDL_Texture* particleTexture;
	// This texture has the sprites that are rendered on the level selection screen and the instructions screen
	SDL_Texture* menuSprites;
	// The texture that holds the touchscreen controls
	SDL_Texture* controlsSpritesheet;

	// All of the levels will be in this array, including the level selection level
	GameLevel maps[4];
	int currentLevel;

	// The font handler class loads fonts and can draw them to the screen
	FontHandler* fontHandler;
	// This handles audio (obviously)
	AudioHandler audioHandler;

	// Camera offset. These dont hold the position of the camera, but rather just specify an offset that we need to se when rendering things. This gives the illusion of a camera
	float camXOffset = 0;
	float camYOffset = 0;

	// This world will contain all of the physics things and stuff. It does the heavy lifting
	b2World* physicsWorld;
	// We need a pointer the the player's body so we can get the coordinates from it
	b2Body* playerBody;
	// The instance to the collision callback class
	CollisionListener* collisionListener;
	// This class handles drawing the box2d fixtures and shapes for debugging
	Box2dDraw debugDrawer;

	bool debugDrawHitboxes;

	// These particles make a nice effect when the player dies.
	struct DeathParticle {
		b2Body* body;
		char colorIndex;
	};
	vector<Platformer::DeathParticle> deathParticles;

	// We need to keep a map of all of the fingers currently pressing down
	#ifdef MOBILE
	unordered_map<SDL_FingerID, b2Vec2> fingerLocations;
	#endif

	// This is used for detecting if sufficient time has passed for the player to jump. We need this because when the player jumps, for a few milliseconds the sensor
	// is still touchnig the ground. This means that a few more impulses will be applied, making the player jump really high and fast.
	unsigned long playerJumpStartTime;

	unsigned long frameCount;

	bool muted;
	bool quit;
	bool paused;
	bool displayAreYouSure;
	short displayAreYouSure_Reason;
	bool playerDead;

	// This variable will hold the current screen type that the game is on, which is used to decide what game logic loop to do
	enum class screenTypes {
		MAIN_MENU,
		GAME,
		CREDITS,
		INSTRUCTIONS
	} currentScreenType;

	struct Button {
		string text;
		int x;
		int y;
		int width;
		int height;

		Button(string text, int x, int y, int width, int height) {
			this->text = text;
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
		}
	};

	// IDK y i still have this function tbh
	SDL_Texture* loadTexture(const char* filename);

	// The main loop for the screen where users actually play the game
	void gameScreenLoop(bool pendingMouseEvent, bool pendingKeyEvent);
	// Main menu
	void menuScreenLoop(bool pendingMouseEvent);

	void creditsScreenLoop(bool pendingMouseEvent);
	void instructionsScreenLoop(bool pendingMouseEvent);

	// Checks if any map scrolling is needed based on the players position
	void checkScrolling();

	void updatePlayerAnimation(bool movingSideways, bool movingVertical);

	// Checks if the given point is inside a button. Utility function
	bool isPointInButton(int x, int y, const Button& button);
	// Checks if the any of the given points are inside a button. Utility function
	bool arePointsInButton(const Button& button);

	void createPhysics();

	void writeUserData();


	//## ----- BUTTTON ACTIONS ----- ##//
	// These functions are called when buttons are clicked. They do the work of the buttons

	void playButton();
	void resumeButton();
	void exitButton();
	void yesButton();
	void noButton();
	void mainMenuButton();
	void restartLevelButton();
	void muteButton();
	void unmuteButton();
	void respawnButton();
	void levelSelectButton();
};