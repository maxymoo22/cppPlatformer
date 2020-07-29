#include "Box2dOverrides.h"

CollisionListener::CollisionListener() {
	playerBody = NULL;
}

void CollisionListener::SetPlayerBody(b2Body* body) {
	playerBody = body;
}

// We need a way of resetting everything
void CollisionListener::clear() {
	playerGroundContacts = 0;
	playerDangerContacts = 0;
	playerLadderContacts = 0;
	playerFinishPointContacts = 0;
	playerMPContacts = 0;

	// These 3 need to be reset because they are per-level based
	movingPlatforms.clear();
	buttons.clear();
	playerBody = NULL;
}

void CollisionListener::BeginContact(b2Contact* contact) {
	// These two check if the player touches a moving platform. If so, we need to find the correct body and add it to the vector.
	// It will then be used to get the velocity, which will be added to the players velocity to stop them falling of the platform when not moving
	if ((int)(contact->GetFixtureA()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) == MOVING_PLATFORM) {
		playerMPContacts++;
		movingPlatforms.push_back(contact->GetFixtureB()->GetBody());
	}

	if ((int)(contact->GetFixtureB()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) == MOVING_PLATFORM) {
		playerMPContacts++;
		movingPlatforms.push_back(contact->GetFixtureA()->GetBody());
	}
	/////////////////////////


	// If one of the fixtures is the player's sensor colliding with something, then the player must now be on the ground
	if ((int)(contact->GetFixtureA()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != LADDER && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != FINISH_POINT && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != DANGEROUS_TILE ||
		(int)(contact->GetFixtureB()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != LADDER && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != FINISH_POINT && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != DANGEROUS_TILE) {
		playerGroundContacts++;
		
		// This checks if fixture A is the ground/entity
		if ((int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR)
			entityFixturesUnderfoot.insert(contact->GetFixtureA());
		// Otherwise fixture it will be fixture B
		else
			entityFixturesUnderfoot.insert(contact->GetFixtureB());
	}

	// Checking for collisions with dangerous tiles
	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) == DANGEROUS_TILE && (int)(contact->GetFixtureB()->GetUserData()) != PLAYER_SENSOR ||
			 (int)(contact->GetFixtureB()->GetBody()->GetUserData()) == DANGEROUS_TILE && (int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR) {
		playerDangerContacts++;
	}

	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) == LADDER && (int)(contact->GetFixtureB()->GetUserData()) != PLAYER_SENSOR ||
			 (int)(contact->GetFixtureB()->GetBody()->GetUserData()) == LADDER && (int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR) {
		playerLadderContacts++;
	}

	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) == FINISH_POINT && (int)(contact->GetFixtureB()->GetUserData()) != PLAYER_SENSOR ||
			 (int)(contact->GetFixtureB()->GetBody()->GetUserData()) == FINISH_POINT && (int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR) {
		playerFinishPointContacts++;
	}
	
	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) != PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) / 1000000 == BUTTON) {
		cout << "Contact with button. Platform id: " << (int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000 << endl;
		buttons[(int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000]++;
	}

	else if ((int)(contact->GetFixtureB()->GetBody()->GetUserData()) != PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) / 1000000 == BUTTON) {
		cout << "Contact with button. Platform id: " << (int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000 << endl;
		buttons[(int)(contact->GetFixtureA()->GetBody()->GetUserData()) - 7000000]++;
	}

	// Set the gravity of the player correctly
	if (playerLadderContacts > 0)
		playerBody->SetGravityScale(0);
}

void CollisionListener::EndContact(b2Contact* contact) {
	// These two check if the player leaves a moving platform. If so, we need to find the correct body and add it to the vector.
	// It will then be used to get the velocity, which will be added to the players velocity to stop them falling of the platform when not moving
	if ((int)(contact->GetFixtureA()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) == MOVING_PLATFORM) {
		playerMPContacts--;

		// This searches the vector for the moving platform body and returns the index
		auto index = find(movingPlatforms.begin(), movingPlatforms.end(), contact->GetFixtureB()->GetBody()) - movingPlatforms.begin();
		// This will actually remove the body since we no longer need it
		movingPlatforms.erase(movingPlatforms.begin() + index);
	}

	if ((int)(contact->GetFixtureB()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) == MOVING_PLATFORM) {
		playerMPContacts--;
		
		// This searches the vector for the moving platform body and returns the index
		auto index = find(movingPlatforms.begin(), movingPlatforms.end(), contact->GetFixtureA()->GetBody()) - movingPlatforms.begin();
		// This will actually remove the body since we no longer need it
		movingPlatforms.erase(movingPlatforms.begin() + index);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// If one of the fixtures is the player's sensor breaking contact with something, then the player must now be off of the ground
	if ((int)(contact->GetFixtureA()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != LADDER && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != FINISH_POINT && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) != DANGEROUS_TILE ||
		(int)(contact->GetFixtureB()->GetUserData()) == PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != LADDER && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != FINISH_POINT && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) != DANGEROUS_TILE) {
		playerGroundContacts--;
		
		// This checks if fixture A is the ground/entity
		if ((int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR)
			entityFixturesUnderfoot.erase(contact->GetFixtureA());
		// Otherwise fixture it will be fixture B
		else
			entityFixturesUnderfoot.erase(contact->GetFixtureB());
	}

	// Checking for collisions with dangerous tiles
	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) == DANGEROUS_TILE && (int)(contact->GetFixtureB()->GetUserData()) != PLAYER_SENSOR ||
			(int)(contact->GetFixtureB()->GetBody()->GetUserData()) == DANGEROUS_TILE && (int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR) {
		playerDangerContacts--;
	}

	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) == LADDER && (int)(contact->GetFixtureB()->GetUserData()) != PLAYER_SENSOR ||
		(int)(contact->GetFixtureB()->GetBody()->GetUserData()) == LADDER && (int)(contact->GetFixtureA()->GetUserData()) != PLAYER_SENSOR) {
		playerLadderContacts--;
	}

	else if ((int)(contact->GetFixtureA()->GetBody()->GetUserData()) != PLAYER_SENSOR && (int)(contact->GetFixtureB()->GetBody()->GetUserData()) / 1000000 == BUTTON) {
		cout << "End contact with button. Platform id: " << (int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000 << endl;
		buttons[(int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000]--;
	}

	else if ((int)(contact->GetFixtureB()->GetBody()->GetUserData()) != PLAYER_SENSOR && (int)(contact->GetFixtureA()->GetBody()->GetUserData()) / 1000000 == BUTTON) {
		cout << "End contact with button. Platform id: " << (int)(contact->GetFixtureB()->GetBody()->GetUserData()) - 7000000 << endl;
		buttons[(int)(contact->GetFixtureA()->GetBody()->GetUserData()) - 7000000]--;
	}
	
	// Set the gravity of the player correctly
	if (playerLadderContacts < 1)
		playerBody->SetGravityScale(1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The below code draws the box2d fixtures and bodies. This is very useful for debugging; you can see exactly where everything is
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Box2dDraw::Box2dDraw() {}

Box2dDraw::Box2dDraw(SDL_Renderer* ren, int screenHeight) {
	renderer = ren;
	SCREEN_HEIGHT = screenHeight;
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
		SDL_RenderDrawLine(renderer, vertices[i].x * 32 - camXOffset, SCREEN_HEIGHT - (vertices[i].y * 32) - camYOffset, vertices[i + 1].x * 32 - camXOffset, SCREEN_HEIGHT - (vertices[i + 1].y * 32) - camYOffset);
	}
}

void Box2dDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	for (int i = 0; i < vertexCount - 1; i++) {
		SDL_RenderDrawLine(renderer, vertices[i].x * 32 - camXOffset, SCREEN_HEIGHT - (vertices[i].y * 32) - camYOffset, vertices[i + 1].x * 32 - camXOffset, SCREEN_HEIGHT - (vertices[i + 1].y * 32) - camYOffset);
	}

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
	SDL_RenderDrawLine(renderer, p1.x * 32 - camXOffset, SCREEN_HEIGHT - (p1.y * 32) - camYOffset, p2.x * 32 - camXOffset, SCREEN_HEIGHT - (p2.y * 32) - camYOffset);
}

void Box2dDraw::DrawTransform(const b2Transform& xf) {
	//std::cout << "Drawing transform\n";
	//SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}

void Box2dDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color) {
	//SDL_SetRenderDrawColor(renderer, rand() % 256, rand() % 256, rand() % 256, color.a);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, color.a);
	SDL_RenderDrawPoint(renderer, p.x * 32 - camXOffset, SCREEN_HEIGHT - (p.y * 32) - camYOffset);
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
		SDL_RenderDrawLine(renderer, start.x * 32 - camXOffset, SCREEN_HEIGHT - (start.y * 32) - camYOffset, end.x * 32 - camXOffset, SCREEN_HEIGHT - (end.y * 32) - camYOffset);
	}
}