#include "GameLevel.h"

GameLevel::GameLevel() {}

// Loads the map and its tileset. Returns false if the map couldn't be loaded
bool GameLevel::load(int screenWidth, int screenHeight, SDL_Renderer* ren, const char* mapFilename, b2World* world) {
	SCREEN_WIDTH = screenWidth;
	SCREEN_HEIGHT = screenHeight;
	renderer = ren;

	tmx::Map tiledMap;
	// Check for unsuccesful map load
	if (tiledMap.load(mapFilename) == false) {
		std::cout << "Failed to load map" << endl;
		return false;
	}

	// Save the dimensions for rendering
	width = tiledMap.getTileCount().x;
	height = tiledMap.getTileCount().y;

	// Loop through all of the tilesets
	auto& mapTilesets = tiledMap.getTilesets();
	for (auto& tileset : mapTilesets) {
		// Only use this tileset if the tileset is a single image - not supporting collection of images right now
		if (tileset.getImagePath() == "")
			continue;

		// Load the image and then create the texture it
		SDL_Surface* surface = IMG_Load(tileset.getImagePath().c_str());
		// Add the tileset texture to the map
		tilesets.insert(make_pair(tileset.getFirstGID(), make_pair(tileset.getLastGID(), SDL_CreateTextureFromSurface(renderer, surface))));
		// We can free the unused surface as we no longer need it (because we have a texture)
		SDL_FreeSurface(surface);
	}

	// Loop through all of the layers
	auto& mapLayers = tiledMap.getLayers();
	for (auto& layer : mapLayers) {

		// The "collisions" object layer holds the hitboxes for the level
		if (layer->getName() == "collisions" && layer->getType() == tmx::Layer::Type::Object)
			collisionObjects = layer->getLayerAs<tmx::ObjectGroup>().getObjects();

		// We're only looking to add hitboxes to the tiles on the map, so if this layer isn't a tile layer, we'll move on.
		if (layer->getType() != tmx::Layer::Type::Tile)
			continue;

		// Get all of this layer's tiles.
		auto& layerTiles = layer->getLayerAs<tmx::TileLayer>().getTiles();

		// Now we can loop through all of the tiles in this layer
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				uint32_t tileGID = layerTiles[y * width + x].ID;

				// If the GID is zero then its an empty tile so we dont want to do anything with it. We can skip to the next tile
				if (tileGID == 0) continue;

				// This will hold the first GID of the tileset that this tile belongs to. If it changes from -1, then we have found a tileset
				int tset_gid = -1;
				SDL_Rect spriteRect;
				// If we didn't find a valid tileset then skip the tile
				if (!getTileSourceRect(tileGID, &tset_gid, &spriteRect)) continue;

				// Now that everything checks out, we can add this tile to the tiles vector
				Tile tile = { tset_gid, x, y, spriteRect};
				tiles.push_back(tile);
			}
		}
	}

	return true;
}

bool GameLevel::getTileSourceRect(int tileGID, int* tset_gid, SDL_Rect* outputRect) {
	for (auto ts : tilesets) {
		if (tileGID >= ts.first && tileGID <= ts.second.first) {
			*tset_gid = ts.first;
			break;
		}
	}

	// If we didn't find a valid tileset, skip the tile.
	if (*tset_gid == -1) return false;
	tileGID -= *tset_gid;

	// Get the dimensions of the tileset to figure out the source rect for the tile
	int tsWidth = 0;
	SDL_QueryTexture(tilesets[*tset_gid].second, NULL, NULL, &tsWidth, NULL);

	// Since the tile texture is just one tile in the sprite sheet, we need to create a rect to extract it
	*outputRect = { (tileGID % (tsWidth / 32)) * 32, (tileGID / (tsWidth / 32)) * 32, 32, 32 };

	return true;
}

void GameLevel::render(float camXOffset, float camYOffset) {
	for (Tile tile : tiles) {
		// Creating a rectangle for the tiles destination on the screen. Since we have a camera, we need to subtract the camera offset to give a scrolling effect
		SDL_Rect destinationRect = { (int)(tile.x * 32 - camXOffset), (int)(SCREEN_HEIGHT - (height * 32 - tile.y * 32) - camYOffset), 32, 32 };

		// We can skip rendering the tile if it is outside the camera as we wont be seeing it anyway
		if (isTileInRect(&destinationRect) == false)
			// Skip this tile
			continue;

		SDL_RenderCopy(renderer, tilesets[tile.tilesetGID].second, &tile.spriteRect, &destinationRect);
	}

	for (Entity& entity : entities) {
		b2Vec2 entityPos = entity.entityBody->GetPosition();

		// Creating a rectangle for the tiles destination on the screen. Since we have a camera, we need to subtract the camera offset to give a scrolling effect
		SDL_Rect destinationRect = { ((entityPos.x - 0.5) * 32 - camXOffset), (SCREEN_HEIGHT - ((entityPos.y + 0.5) * 32) - camYOffset), 32, 32 };

		// We can skip rendering the tile if it is outside the camera as we wont be seeing it anyway
		if (isTileInRect(&destinationRect) == false)
			// Skip this tile
			continue;

		SDL_RenderCopyEx(renderer, tilesets[entity.tilesetGID].second, &entity.spriteRect, &destinationRect, (double)entity.entityBody->GetAngle() * -180.0 / 3.141592653589793, NULL, SDL_FLIP_NONE);
	}

	for (auto platformIDPair : movingPlatforms) {
		MovingPlatform platform = platformIDPair.second;

		b2Vec2 entityPos = platform.entityBody->GetPosition();

		// Creating a rectangle for the tiles destination on the screen. Since we have a camera, we need to subtract the camera offset to give a scrolling effect
		SDL_Rect destinationRect = { ((entityPos.x - 0.5) * 32 - camXOffset), (SCREEN_HEIGHT - ((entityPos.y + 0.5) * 32) - camYOffset), 32, 32 };

		// We can skip rendering the tile if it is outside the camera as we wont be seeing it anyway
		if (isTileInRect(&destinationRect) == false) {
			// Skip this tile
			//cout << "This moving platform is outside the view rect\n";
			continue;
		}

		//cout << "Rendering moving platform at x=" << destinationRect.x << endl;
		SDL_RenderCopy(renderer, tilesets[platform.tilesetGID].second, &platform.spriteRect, &destinationRect);
	}
}

bool GameLevel::isTileInRect(SDL_Rect* tileRect) {
	// Here we are comparing the borders of the tile to the window borders
	if (tileRect->x + 32 < 0 || tileRect->x > SCREEN_WIDTH || tileRect->y + 32 < 0 || tileRect->y > SCREEN_HEIGHT) {
		// If the sides of the tile are outside the window's sides, then this tile must be outside of the window (obviously)
		return false;
	}

	// Otherwise the tile will be inside the camera, and we want to render it
	return true;
}

GameLevel::~GameLevel() {
	printf("Game level deconstructor called\n");

	// Finally we can loop through each tileset and release the memory taken by the textures
	for (auto tileset : tilesets) {
		printf("Destroying tileset texture\n");
		SDL_DestroyTexture(tileset.second.second);
		tileset.second.second = NULL;
	}
}

void GameLevel::createHitboxes(b2World* world) {
	// Remove any thingies if they existed from the previous level
	movingPlatforms.clear();
	entities.clear();

	for (auto& object : collisionObjects)
	{
		// A map of properties is easier to use than a vector
		unordered_map<string, tmx::Property> objectProperties;
		for (tmx::Property objectProperty : object.getProperties()) {
			auto var1 = objectProperty;
			objectProperties.insert(make_pair(objectProperty.getName(), var1));
		}

		if (object.getType() == "entity" || object.getType() == "mp") {
			createEntity(&object, world, object.getType() == "mp", objectProperties);
			continue;
		}

		b2BodyDef tileBodyDef;
		tileBodyDef.type = b2_staticBody;

		tileBodyDef.position.Set(object.getPosition().x / 32, height - (object.getPosition().y / 32));
		b2Body* tileBody = world->CreateBody(&tileBodyDef);

		const auto& objectPoints = object.getPoints();
		const int pointCount = (int)(objectPoints.size());

		b2Vec2* chainPoints = new b2Vec2[pointCount];
		for (int i = 0; i < pointCount; i++)
			chainPoints[i] = b2Vec2(objectPoints[i].x / 32, -1 * objectPoints[i].y / 32);

		b2FixtureDef fixtureDef;
		b2ChainShape collisionShape;
		// We need a polygon shape for the finish points on the level
		b2PolygonShape polygonShape;

		// Make the collision chape from the points
		collisionShape.CreateLoop(chainPoints, pointCount);
		fixtureDef.shape = &collisionShape;

		// Ladders should be sensors so that there is no collision response
		if (object.getType() == "ladder") {
			fixtureDef.isSensor = true;
			fixtureDef.userData = (void*)LADDER;
		}

		else if (object.getType() == "button") {
			fixtureDef.isSensor = true;

			// We don't want to have platform ids over 100,000. It could be higher, but just stoppig here to be safe. If it goes too high, the
			// collision handler wont be able to get the correct platform id from the box2d user data
			if (objectProperties.count("platformID") > 0 && objectProperties["platformID"].getIntValue() < 100000) {
				int platformUserData = BUTTON * 1000000 + objectProperties["platformID"].getIntValue();
				fixtureDef.userData = (void*)platformUserData;
			}	
		}

		else if(object.getType() == "danger") {
			// If the tile is dangerous, then we need to add some user data so the collision handler knows
			fixtureDef.userData = (void*)DANGEROUS_TILE;
		}

		// Finish points need to be sensors, but they also need to have an ID and a polygon shape
		else if (object.getType() == "finish") {
			fixtureDef.isSensor = true;
			polygonShape.Set(chainPoints, pointCount);
			fixtureDef.shape = &polygonShape;

			int finishPointUserData = FINISH_POINT * 1000000;
			if (objectProperties.count("level") > 0)
				finishPointUserData += objectProperties["level"].getIntValue();
			fixtureDef.userData = (void*)finishPointUserData;
		}

		// Now we bind the shape to the body with a fixture
		tileBody->CreateFixture(&fixtureDef);

		delete[] chainPoints;
		chainPoints = NULL;
	}
}

void GameLevel::createEntity(tmx::Object* entityObject, b2World* world, bool movingPlatform, unordered_map<string, tmx::Property> objectProperties) {
	MPDirections movementType = MPDirections::NOT_SET;
	
	if (movingPlatform) {
		if (objectProperties.count("direction") < 1) return;
		movementType = (MPDirections)objectProperties["direction"].getIntValue();
	}

	/* We want to exit if the object doent have the right properties, which means one of these:
	* The rendering things were not set (tile id and center of platform/entity)
	* The entity is a moving platform but the direction hasn't been set
	* The entity is a moving platform, but the movement boundaries haven't been set, or they don't correspond correctly to the direction of the platform
	* The entity is amoving platform, but one of its velocity properties wasn't specified
	*/
	if (objectProperties.count("tileGID") < 1 || objectProperties.count("centerX") < 1 || objectProperties.count("centerY") < 1 || movingPlatform && (\
		(objectProperties.count("boundaryLeft") < 1 || objectProperties.count("boundaryRight") < 1 || objectProperties.count("horizontalVelocity") < 1) && (movementType == MPDirections::HORIZONTAL || movementType == MPDirections::DIAGONAL) ||
		(objectProperties.count("boundaryTop") < 1 || objectProperties.count("boundaryBottom") < 1 || objectProperties.count("verticalVelocity") < 1) && (movementType == MPDirections::VERTICAL || movementType == MPDirections::DIAGONAL))) {

		// This entity wasn't setup properly in the Tiled editor
		cout << "NNNNNNNNNNNNNNNNNNAaaaaaaaaaah\nMoving platform: " << movingPlatform << endl;
		return;
	}

	// This will hold the first GID of the tileset that this tile belongs to. If it changes from -1, then we have found a tileset
	int tset_gid = -1;
	SDL_Rect spriteRect;
	// If we didn't find a valid tileset then skip the tile
	if (!getTileSourceRect(objectProperties["tileGID"].getIntValue(), &tset_gid, &spriteRect)) return;

	b2BodyDef entityBodyDef;
	if (movingPlatform)
		entityBodyDef.type = b2_kinematicBody;
	else
		entityBodyDef.type = b2_dynamicBody;
	entityBodyDef.position.Set((float)objectProperties["centerX"].getIntValue() / 32, height - (float)objectProperties["centerY"].getIntValue() / 32);
	b2Body* entityBody = world->CreateBody(&entityBodyDef);

	const auto& objectPoints = entityObject->getPoints();
	const int pointCount = (int)(objectPoints.size());

	b2Vec2* chainPoints = new b2Vec2[pointCount];

	for (int i = 0; i < pointCount; i++)
		chainPoints[i] = b2Vec2(objectPoints[i].x / 32 - (entityBodyDef.position.x - (entityObject->getPosition().x / 32)), -1 * objectPoints[i].y / 32 - (entityBodyDef.position.y - (height - (entityObject->getPosition().y / 32))));

	b2PolygonShape collisionShape;
	collisionShape.Set(chainPoints, pointCount);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &collisionShape;
	fixtureDef.density = 1.0;
	fixtureDef.friction = 3.0;
	fixtureDef.userData = movingPlatform ? (void*)MOVING_PLATFORM : (void*)ENTITY;
	entityBody->CreateFixture(&fixtureDef);

	delete[] chainPoints;
	chainPoints = NULL;

	if (movingPlatform) {
		// Need to get the correct direction for the platform
		b2Vec2 direction(0, 0);
		b2Vec2 MPVelocity(0, 0);
		b2Vec2 horizontalMovementBoundaries(0, 0);
		b2Vec2 verticalMovementBoundaries(0, 0);
		if (movementType == MPDirections::HORIZONTAL || movementType == MPDirections::DIAGONAL) {
			direction.x = 1;
			MPVelocity.x = objectProperties["horizontalVelocity"].getFloatValue();
			horizontalMovementBoundaries.x = (float)objectProperties["boundaryLeft"].getIntValue() / 32;
			horizontalMovementBoundaries.y = (float)objectProperties["boundaryRight"].getIntValue() / 32;
		}
		if (movementType == MPDirections::VERTICAL || movementType == MPDirections::DIAGONAL) {
			direction.y = 1;
			MPVelocity.y = objectProperties["verticalVelocity"].getFloatValue();
			verticalMovementBoundaries.x = height - (float)objectProperties["boundaryTop"].getIntValue() / 32;
			verticalMovementBoundaries.y = height - (float)objectProperties["boundaryBottom"].getIntValue() / 32;
		}

		bool usesButton = false;
		if(objectProperties.count("usesButton") > 0)
			usesButton = objectProperties["usesButton"].getBoolValue();

		MovingPlatform movingPlatform = { tset_gid, spriteRect, entityBody, (int)movementType, usesButton, !usesButton, horizontalMovementBoundaries, verticalMovementBoundaries, MPVelocity, direction };

		// Only start the platform if it doesn't use a button
		if (usesButton == false)
			entityBody->SetLinearVelocity(b2Vec2(MPVelocity.x, MPVelocity.y));

		movingPlatforms.insert(make_pair((int)entityObject->getUID(), movingPlatform));
	}
	else {
		Entity entity = { tset_gid, spriteRect, entityBody };
		entities.push_back(entity);
	}
}

void GameLevel::doMovingPlatformLogic(unordered_map<int, int> buttons) {
	for (auto buttonMapElement : buttons) {
		MovingPlatform& platform = movingPlatforms[buttonMapElement.first];

		// To prevent accidental buttons, we can use a simple check
		if (platform.usesButton == false) continue;

		if (buttonMapElement.second < 1) {
			platform.entityBody->SetLinearVelocity(b2Vec2(0, 0));
			platform.active = false;
		}
		else {
			//cout << "Yooooo stepped on button " << buttonMapElement.first << endl;
			// The direction will either be -1 or 1, so multiplying it with the speed gets the platform going in the same direction as before
			platform.entityBody->SetLinearVelocity(b2Vec2(platform.direction.x * platform.speed.x, platform.direction.y * platform.speed.y));
			platform.active = true;
		}
	}

	for (auto& pair : movingPlatforms) {
		MovingPlatform& platform = pair.second;

		// If this platform isn't active, we should skip it. This is because you could get a rare case where the user steps off the button at exactly the right
		// time to make the platform stop at its boundary. Then boundary checking code would then restart the platform, even if the user is off of the button.
		if (platform.active == false) {
			//cout << "Platform not active\n";
			continue;
		}

		b2Vec2 entityPos = platform.entityBody->GetPosition();
		b2Vec2 entityVel = platform.entityBody->GetLinearVelocity();

		// Position of platform is in the center so we need to subtract or add half a meter which is 16px //
		// A diagonal direction means we need to check both horizontal and vertial components

		if (platform.movementType == (int)MPDirections::HORIZONTAL || platform.movementType == (int)MPDirections::DIAGONAL) {
			// Need to start moving right
			if (entityPos.x - 0.5 <= platform.xMovementBoundaries.x) {
				entityVel.x = platform.speed.x;
				platform.direction.x = 1;
			}

			// Need to start moving left
			else if (entityPos.x + 0.5 >= platform.xMovementBoundaries.y) {
				entityVel.x = -1 * platform.speed.x;
				platform.direction.x = -1;
			}
		}

		if (platform.movementType == (int)MPDirections::VERTICAL || platform.movementType == (int)MPDirections::DIAGONAL) {
			// Need to start moving down
			if (entityPos.y + 0.5 >= platform.yMovementBoundaries.x) {
				entityVel.y = -1 * platform.speed.y;
				platform.direction.y = -1;
			}

			// Need to start moving up
			else if (entityPos.y - 0.5 <= platform.yMovementBoundaries.y) {
				entityVel.y = platform.speed.y;
				platform.direction.y = 1;
			}
		}

		// Update the velocity
		platform.entityBody->SetLinearVelocity(entityVel);
	}
}

void GameLevel::dumpMovingPlatformData(bool param, MovingPlatform* mp) {
	MovingPlatform p;
	if (param == true)
		p = *mp;
	else
		p = movingPlatforms[34];

	cout << "Active: " << p.active << endl;
	cout << "Direction: (" << p.direction.x << ", " << p.direction.y << ")\n";
	cout << "Movement type: " << p.movementType << endl;
	cout << "Speed: (" << p.speed.x << ", " << p.speed.y << ")\n";
	cout << "Sprite rect x: " << p.spriteRect.x << endl;
	cout << "Sprite rect y: " << p.spriteRect.y << endl;
	cout << "Tileset GID: " << p.tilesetGID << endl;
	cout << "Uses button: " << p.usesButton << endl;
	cout << "Horizontal movement boundaries: (" << p.xMovementBoundaries.x << ", " << p.xMovementBoundaries.y << ")\n";
	cout << "Vertical movement boundaries: (" << p.yMovementBoundaries.x << ", " << p.yMovementBoundaries.y << ")\n";
}