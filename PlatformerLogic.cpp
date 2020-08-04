#include "Platformer.h"

// Initialize physics, sdl and all of its libraries
bool Platformer::init() {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_DisplayMode DM;
	SDL_GetCurrentDisplayMode(0, &DM);

	// Create window
	window = SDL_CreateWindow("Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT,/*DM.w, DM.h,*/ SDL_WINDOW_SHOWN);
	if (window == NULL) {
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	// Create renderer for window, making it use hardware acceleration
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED/* | SDL_RENDERER_PRESENTVSYNC*/);
	if (renderer == NULL)
	{
		printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	// Store the screen dimensions for use in rendering the textures at the correct coordinates
	SDL_GetRendererOutputSize(renderer, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Initialize PNG loading
	if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG))
	{
		printf("SDL_image could not be initialized! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	// Initialize the ttf library
	if (TTF_Init() == -1) {
		printf("Couldn't initialize the ttf library! Error: %s\n", TTF_GetError());
		return false;
	}

	// Initialize SDL_mixer for music
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not be initialized! SDL_mixer error: %s\n", Mix_GetError());
		return false;
	}

	// This class will handle the stuff when the player touches the ground
	collisionListener = new CollisionListener();
	// For debugging
	debugDrawer = Box2dDraw(renderer, SCREEN_HEIGHT);
	debugDrawer.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);

	return true;
}

// Loads the maps and the player
bool Platformer::loadAssets() {
	bool result = true;

	menuSprites = loadTexture("resources/menuSpritesheet.png");

	for (int i = 0; i < 3; i++) {
		string mapName = "resources/maps/level " + to_string(i) + ".tmx";
		result = maps[i].load(SCREEN_WIDTH, SCREEN_HEIGHT, renderer, mapName.c_str(), physicsWorld);
		if (!result) return false;
	}

	// Load the player sprite
	player = loadTexture("resources/playerSpritesheet.png");

	// Setup the physics for the current level in advance
	createPhysics();
	maps[currentLevel].createHitboxes(physicsWorld);

	// Do some font stuff
	fontHandler = new FontHandler(renderer);
	result = fontHandler->loadFont("button_font", "resources/fonts/joystix.ttf", 18);
	if (!result) return false;
	result = fontHandler->loadFont("popup_font", "resources/fonts/joystix.ttf", 28);
	if (!result) return false;
	result = fontHandler->loadFont("heading_font", "resources/fonts/joystix.ttf", 100);
	if (!result) return false;

	result = audioHandler.loadMusic();
	if (!result) return false;

	audioHandler.playMusic(audioHandler.MAIN_MENU);

	return true;
}

// Keeps looping and calls the right draw+logic function until the user quits
void Platformer::loop() {
	// Keep looping until the user quits the game
	while (quit == false) {
		frameCount++;

		bool pendingMouseEvent = false;
		bool pendingKeyEvent = false;

		// If there is no event then we dont need to handle anything, so we should check
		if (SDL_PollEvent(&eventHandler) == 1) {
			// User requests to quit the application
			if (eventHandler.type == SDL_QUIT) {
				printf("Quitting\n");
				quit = true;
			}
			else if (eventHandler.type == SDL_MOUSEBUTTONUP && eventHandler.button.button == SDL_BUTTON_LEFT) {
				//maps[currentLevel].dumpMovingPlatformData(false, NULL);
				pendingMouseEvent = true;
			}
			else if (eventHandler.type == SDL_KEYDOWN) {
				// We don't want escape button repeats
				if (eventHandler.key.repeat == 0)
					pendingKeyEvent = true;

				// Get the key states
				const Uint8* keyStates = SDL_GetKeyboardState(NULL);
				// Toggle box2d debugging
				if (keyStates[SDL_SCANCODE_D] && keyStates[SDL_SCANCODE_B] && keyStates[SDL_SCANCODE_G]) {
					debugDrawHitboxes = !debugDrawHitboxes;
					cout << "Box2d debugging toggled\n";
				}
			}
		}

		// Set renderer color back to light blue because it will be changed for drawing primitives
		SDL_SetRenderDrawColor(renderer, 181, 227, 255, 255);
		// Also clear the screen
		SDL_RenderClear(renderer);

		// Decide which game loop to do based on the current screen type
		switch (currentScreenType) {
		case screenTypes::MAIN_MENU:
			menuScreenLoop(pendingMouseEvent);
			break;
		case screenTypes::GAME:
			gameScreenLoop(pendingMouseEvent, pendingKeyEvent);
			break;
		case screenTypes::CREDITS:
			creditsScreenLoop(pendingMouseEvent);
			break;
		case screenTypes::INSTRUCTIONS:
			instructionsScreenLoop(pendingMouseEvent);
			break;
		}
	}

	// If the music isn't stopped before exiting SDL_mixer seems to crash
	Mix_HaltMusic();
}

Platformer::Platformer() {
	window = NULL;
	renderer = NULL;
	player = NULL;
	playerTextureXOffset = 96;
	animationFrameIndex = 0;
	playerDirection = 1;
	menuSprites = NULL;
	currentLevel = 1;
	fontHandler = NULL;
	physicsWorld = NULL;
	playerBody = NULL;
	collisionListener = NULL;
	debugDrawHitboxes = true;
	playerJumpStartTime = 0;
	quit = false;
	paused = false;
	displayAreYouSure = false;
	displayAreYouSure_Reason = 0;
	playerDead = false;
	currentScreenType = screenTypes::MAIN_MENU;
	frameCount = 0;
	muted = false;
}

// Frees memory
Platformer::~Platformer() {
	// Delete the player sprite texture
	SDL_DestroyTexture(player);
	player = NULL;

	SDL_DestroyTexture(menuSprites);
	menuSprites = NULL;

	// Destroy window and renderer
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	window = NULL;
	renderer = NULL;

	// Destroy all of the physics objects
	delete physicsWorld;
	delete collisionListener;
	collisionListener = NULL;
	physicsWorld = NULL;
	playerBody = NULL;

	delete fontHandler;
	fontHandler = NULL;

	cout << "\nAverage FPS: " << frameCount / (SDL_GetTicks() / 1000) << endl;
	SDL_Delay(1000);

	// Quit SDL subsystems
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();

	// Pause
	/*string pauseInput;
	cout << "Paused...";
	cin >> pauseInput;*/
}

SDL_Texture* Platformer::loadTexture(const char* filename) {
	// Load the image and then create the texture it
	SDL_Surface* surface = IMG_Load(filename);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

	// We can free the unused surface as we no longer need it because we have a texture
	SDL_FreeSurface(surface);

	return texture;
}

// Checks if any map scrolling is needed based on the players position
void Platformer::checkScrolling() {
	float xPos = (playerBody->GetPosition().x - 0.5) * 32;
	float yPos = SCREEN_HEIGHT - ((playerBody->GetPosition().y + 0.5) * 32);

	// If the distance from the player's left side to the left side of the viewport is less than the minimum margin
	// then we need to calculate how much we need to change the viewport by
	if (xPos - camXOffset < PLAYER_SPRITE_LR_MARGIN)
		camXOffset -= PLAYER_SPRITE_LR_MARGIN - (xPos - camXOffset);

	// If the distance from the player's right side to the right side of the viewport is less than the minimum margin
	// then we need to calculate how much we need to change the viewport by
	else if ((camXOffset + SCREEN_WIDTH) - xPos < PLAYER_SPRITE_LR_MARGIN)
		camXOffset += (xPos + PLAYER_SPRITE_LR_MARGIN) - (camXOffset + SCREEN_WIDTH);

	// If the distance between the top of the player and the top of the viewport is less than the minimum margin
	// then we need to calculate how much we need to change the viewport by
	if (yPos - camYOffset < PLAYER_SPRITE_TOP_MARGIN)
		camYOffset -= PLAYER_SPRITE_TOP_MARGIN - (yPos - camYOffset);

	// If the distance between the bottom of the player and the bottom of the viewport is less than the minimum margin,
	// then we need to calculate how much we need to change the viewport by
	else if ((camYOffset + SCREEN_HEIGHT) - yPos < PLAYER_SPRITE_BOTTOM_MARGIN) {
		float incrementValue = PLAYER_SPRITE_BOTTOM_MARGIN - (camYOffset + SCREEN_HEIGHT - yPos);

		// We don't want to scroll if we are going to go below the bottom of the screen. I might remove this later, but for now we need this little check
		if (camYOffset + incrementValue <= 0)
			camYOffset += incrementValue;
	}
}

// Checks if the given point is inside a rectangle. Utility function
bool Platformer::isPointInRect(int x, int y, int rectX, int rectY, int width, int height) {
	if (x < rectX || x > rectX + width)
		return false;
	if (y < rectY || y > rectY + height)
		return false;

	return true;
}
// Same as above but better
bool Platformer::isPointInButton(int x, int y, Button& button) {
	if (x < button.x || x > button.x + button.width)
		return false;
	if (y < button.y || y > button.y + button.height)
		return false;

	return true;
}

void Platformer::createPhysics() {
	// Clear the contact listener
	collisionListener->clear();
	// We also need to clear the previous world if it existed
	if (physicsWorld != NULL) delete physicsWorld;

	// Create the physics world with some gravity
	physicsWorld = new b2World(b2Vec2(0.0f, -25.0f));
	// The overrides handle drawing and collisions
	physicsWorld->SetContactListener(collisionListener);
	physicsWorld->SetDebugDraw(&debugDrawer);

	// Create a dynamic body for the player
	b2BodyDef playerBodyDef;
	playerBodyDef.type = b2_dynamicBody;
	playerBodyDef.fixedRotation = true;
	playerBodyDef.position.Set(3.5, 2.5);
	playerBody = physicsWorld->CreateBody(&playerBodyDef);

	// Create the players collision shape
	b2PolygonShape collisionShape;
	b2Vec2 collisionShapePoints[] = { b2Vec2(-0.48, 0.48), b2Vec2(0.48, 0.48), b2Vec2(0.48, -0.44), b2Vec2(0.16, -0.48), b2Vec2(-0.16, -0.48), b2Vec2(-0.48, -0.44) };
	collisionShape.Set(collisionShapePoints, 6);

	// Bind physics properties to the player with a fixture
	b2FixtureDef playerFixture;
	playerFixture.shape = &collisionShape;
	playerFixture.density = 1.0;
	playerFixture.friction = 0.01;
	playerFixture.userData = (void*)PLAYER_BODY;
	playerBody->CreateFixture(&playerFixture);

	// This will be a sensor fixture. It will detect if the player is touching the ground
	b2FixtureDef playerSensorFixtureDef;
	collisionShape.SetAsBox(0.38, 0.1, b2Vec2(0, -0.48), 0);
	playerSensorFixtureDef.isSensor = true;
	playerSensorFixtureDef.shape = &collisionShape;
	b2Fixture* playerSensorFixture = playerBody->CreateFixture(&playerSensorFixtureDef);
	playerSensorFixture->SetUserData((void*)PLAYER_SENSOR);

	collisionListener->SetPlayerBody(playerBody);
}

void Platformer::updatePlayerAnimation(bool movingSideways, bool movingVertical) {
	if (collisionListener->playerGroundContacts > 0) {
		if (!movingSideways) {
			playerTextureXOffset = 96;
			animationFrameIndex = 0.0;
		}
		else {
			playerTextureXOffset = 32 * ((int)animationFrameIndex % 3) + 64;
			animationFrameIndex += 1.0/10.0;
			if (animationFrameIndex >= 3) animationFrameIndex = 0.0;
		}
	}

	else if (collisionListener->playerLadderContacts > 0) {
		if (movingVertical) {
			playerTextureXOffset = 32 * ((int)animationFrameIndex % 2);
			animationFrameIndex += 1.0 / 10.0;
			if (animationFrameIndex >= 2) animationFrameIndex = 0.0;
		}
		else {
			playerTextureXOffset = 96;
			animationFrameIndex = 0.0;
		}
	}

	else {
		animationFrameIndex = 0.0;
		playerTextureXOffset = 128;
	}
}