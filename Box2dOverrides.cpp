#include "Box2dOverrides.h"

CollisionListener::CollisionListener() {
	playerBody = NULL;
	levelEntranceNum = -1;
}

void CollisionListener::SetPlayerBody(b2Body* body) { playerBody = body; }
void CollisionListener::nullPlayerBody() { playerBody = NULL; }

// We need a way of resetting everything
void CollisionListener::clear() {
	playerGroundContacts = 0;
	playerDangerContacts = 0;
	playerLadderContacts = 0;
	playerFinishPointContacts = 0;
	levelEntranceNum = -1;

	// These 4 need to be reset because they are per-level based
	movingPlatforms.clear();
	buttons.clear();
	entityFixturesUnderfoot.clear();
	playerBody = NULL;
}

void CollisionListener::BeginContact(b2Contact* contact) {
	// Store the type of fixture. There are 8 possible values defined in Box2dOverrides.h Need to do a cast to (size_t) because of issues with
	// the building in android studio. See https://stackoverflow.com/a/27044460/10018988
	int fixtureAData = (int)(size_t)contact->GetFixtureA()->GetUserData();
	int fixtureBData = (int)(size_t)contact->GetFixtureB()->GetUserData();

	// We need to know what moving platforms the player is on so we can add to the velocity. This stops the player from sliding off
	if (fixtureAData == PLAYER_SENSOR && fixtureBData == MOVING_PLATFORM)
		movingPlatforms.insert(contact->GetFixtureB()->GetBody());
	else if (fixtureAData == MOVING_PLATFORM && fixtureBData == PLAYER_SENSOR)
		movingPlatforms.insert(contact->GetFixtureA()->GetBody());

	// The basics, checks for collisions with ladders and things that kill the player
	if (fixtureAData == LADDER && fixtureBData == PLAYER_BODY || fixtureBData == LADDER && fixtureAData == PLAYER_BODY)
		playerLadderContacts++;
	else if (fixtureAData == DANGEROUS_TILE && fixtureBData == PLAYER_BODY || fixtureBData == DANGEROUS_TILE && fixtureAData == PLAYER_BODY)
		playerDangerContacts++;

	// End of level
	else if (fixtureAData / 1000000 == FINISH_POINT && fixtureBData == PLAYER_BODY) {
		playerFinishPointContacts++;
		// Store the finish point that the player is currently touching. This will be used to decide which level they go to
		levelEntranceNum = fixtureAData - FINISH_POINT * 1000000;
	}
	else if (fixtureBData / 1000000 == FINISH_POINT && fixtureAData == PLAYER_BODY) {
		playerFinishPointContacts++;
		// Store the finish point that the player is currently touching. This will be used to decide which level they go to
		levelEntranceNum = fixtureBData - FINISH_POINT * 1000000;
	}

	// As long as it's not the player's foot sensor, if something touches a button we need to know so we can activate the platform it's linked to
	else if (fixtureAData != PLAYER_SENSOR && fixtureBData / 1000000 == BUTTON) {
		buttons[fixtureBData - BUTTON * 1000000]++;
		cout << "Stepped on button\n";
	}
	else if (fixtureAData / 1000000 == BUTTON && fixtureBData != PLAYER_SENSOR) {
		buttons[fixtureAData - BUTTON * 1000000]++;
		cout << "Stepped on button\n";
	}

	// Finally, we need to know when the player is on the ground. This stops them from jumping in mid air and flying around
	if (fixtureAData == PLAYER_SENSOR && fixtureBData != LADDER && fixtureBData != DANGEROUS_TILE && fixtureBData < 100000 ||
		fixtureBData == PLAYER_SENSOR && fixtureAData != LADDER && fixtureAData != DANGEROUS_TILE && fixtureAData < 100000) {
		playerGroundContacts++;

		// If fixture A is an entity, then we need to add it to the entity list. This will then be used to apply a kickback to it when the player walks
		if (fixtureAData == ENTITY)
			entityFixturesUnderfoot.insert(contact->GetFixtureA());
		// Same thing if the entity is fixture B instead
		else if (fixtureBData == ENTITY)
			entityFixturesUnderfoot.insert(contact->GetFixtureB());
	}

	// If the player is on a ladder then they shouldn't have gravity
	if (playerLadderContacts > 0 && playerBody != NULL)
		playerBody->SetGravityScale(0);
}

void CollisionListener::EndContact(b2Contact* contact) {
	// Store the type of fixture. There are 8 possible values defined in Box2dOverrides.h Need to do a cast to (size_t) because of issues with
	// the building in android studio. See https://stackoverflow.com/a/27044460/10018988
	int fixtureAData = (int)(size_t)contact->GetFixtureA()->GetUserData();
	int fixtureBData = (int)(size_t)contact->GetFixtureB()->GetUserData();

	// We need to know what moving platforms the player is on so we can add to the velocity. This stops the player from sliding off
	if (fixtureAData == PLAYER_SENSOR && fixtureBData == MOVING_PLATFORM)
		movingPlatforms.erase(contact->GetFixtureB()->GetBody());
	else if (fixtureAData == MOVING_PLATFORM && fixtureBData == PLAYER_SENSOR)
		movingPlatforms.erase(contact->GetFixtureA()->GetBody());

	// All of the basics, checks for collisions with ladders, things that kill the player and the end of the level
	if (fixtureAData == LADDER && fixtureBData == PLAYER_BODY || fixtureBData == LADDER && fixtureAData == PLAYER_BODY)
		playerLadderContacts--;
	else if (fixtureAData == DANGEROUS_TILE && fixtureBData == PLAYER_BODY || fixtureBData == DANGEROUS_TILE && fixtureAData == PLAYER_BODY)
		playerDangerContacts--;
	else if (fixtureAData == FINISH_POINT && fixtureBData == PLAYER_BODY || fixtureBData == FINISH_POINT && fixtureAData == PLAYER_BODY)
		playerFinishPointContacts--;

	// As long as it's not the player's foot sensor, if something touches a button we need to know so we can activate the platform it's linked to
	else if (fixtureAData != PLAYER_SENSOR && fixtureBData / 1000000 == BUTTON)
		buttons[fixtureBData - BUTTON * 1000000]--;
	else if (fixtureAData / 1000000 == BUTTON && fixtureBData != PLAYER_SENSOR)
		buttons[fixtureAData - BUTTON * 1000000]--;

	// Finally, we need to know when the player is on the ground. This stops them from jumping in mid air and flying around
	if (fixtureAData == PLAYER_SENSOR && fixtureBData != LADDER && fixtureBData != DANGEROUS_TILE && fixtureBData < 100000 ||
		fixtureBData == PLAYER_SENSOR && fixtureAData != LADDER && fixtureAData != DANGEROUS_TILE && fixtureAData < 100000) {
		playerGroundContacts--;

		// If fixture A is an entity, then we need to add it to the entity list. This will then be used to apply a kickback to it when the player walks
		if (fixtureAData == ENTITY)
			entityFixturesUnderfoot.erase(contact->GetFixtureA());
		// Same thing if the entity is fixture B instead
		else if (fixtureBData == ENTITY)
			entityFixturesUnderfoot.erase(contact->GetFixtureB());
	}

	// If the player is no longer on a ladder then they should have the normal gravity
	if (playerLadderContacts < 1 && playerBody != NULL)
		playerBody->SetGravityScale(1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The below code draws the box2d fixtures and bodies. This is very useful for debugging; you can see exactly where everything is
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Box2dDraw::Box2dDraw() {}

Box2dDraw::Box2dDraw(SDL_Renderer* ren, int screenHeight, int tileSize) {
	renderer = ren;
	SCREEN_HEIGHT = screenHeight;
	TILE_SIZE = tileSize;
	camXOffset = 0;
	camYOffset = 0;

	srand(SDL_GetTicks());
}

void Box2dDraw::updateCameraOffset(float x, float y) {
	camXOffset = x;
	camYOffset = y;
}

void Box2dDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);

	for (int i = 0; i < vertexCount - 1; i++) {
		SDL_RenderDrawLine(renderer, vertices[i].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[i].y * TILE_SIZE) - camYOffset, vertices[i + 1].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[i + 1].y * TILE_SIZE) - camYOffset);
	}
	SDL_RenderDrawLine(renderer, vertices[vertexCount - 1].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[vertexCount - 1].y * TILE_SIZE) - camYOffset, vertices[0].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[0].y * TILE_SIZE) - camYOffset);
}

void Box2dDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	for (int i = 0; i < vertexCount - 1; i++) {
		SDL_RenderDrawLine(renderer, vertices[i].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[i].y * TILE_SIZE) - camYOffset, vertices[i + 1].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[i + 1].y * TILE_SIZE) - camYOffset);
	}
	SDL_RenderDrawLine(renderer, vertices[vertexCount - 1].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[vertexCount - 1].y * TILE_SIZE) - camYOffset, vertices[0].x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (vertices[0].y * TILE_SIZE) - camYOffset);

	/*Sint16* xVertices = new Sint16[vertexCount];
	Sint16* yVertices = new Sint16[vertexCount];

	for (int i = 0; i < vertexCount; i++) {
		xVertices[i] = vertices[i].x;
		yVertices[i] = vertices[i].y;
	}

	aapolygonRGBA(renderer, xVertices, yVertices, vertexCount, color.r, color.g, color.b, color.a);*/
}

void Box2dDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	DrawPoint(center, radius, color);
}

void Box2dDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	//DrawPoint(center, radius, color);
	renderCircle(center, radius, 60);
}

void Box2dDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	SDL_RenderDrawLine(renderer, p1.x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (p1.y * TILE_SIZE) - camYOffset, p2.x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (p2.y * TILE_SIZE) - camYOffset);
}

void Box2dDraw::DrawTransform(const b2Transform& xf) {
	//std::cout << "Drawing transform\n";
	//SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Box2dDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	SDL_RenderDrawPoint(renderer, p.x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (p.y * TILE_SIZE) - camYOffset);
}

void Box2dDraw::renderCircle(const b2Vec2& center, float radius, unsigned int sides)
{
	const double _2PI = 3.1415926535898;

	if (sides == 0)
	{
		sides = _2PI * radius / 2;
	}

	float d_a = _2PI / sides, angle = d_a;

	b2Vec2 start, end;
	end.x = radius;
	end.y = 0.0f;
	end = end + center;
	for (int i = 0; i != sides; i++)
	{
		start = end;
		end.x = cos(angle) * radius;
		end.y = sin(angle) * radius;
		end = end + center;
		angle += d_a;
		SDL_RenderDrawLine(renderer, start.x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (start.y * TILE_SIZE) - camYOffset, end.x * TILE_SIZE - camXOffset, SCREEN_HEIGHT - (end.y * TILE_SIZE) - camYOffset);
	}
}