#pragma once

#include <box2d.h>
#include <SDL.h>
#include <iostream>
#include <stdlib.h>  // For random numbers
#include <vector>
#include <unordered_map>
#include <set>

#define PLAYER_BODY 1
#define PLAYER_SENSOR 2
#define DANGEROUS_TILE 3
#define LADDER 4
#define FINISH_POINT 5
#define MOVING_PLATFORM 6
#define BUTTON 7
#define ENTITY 8

using namespace std;

class CollisionListener : public b2ContactListener {
public:
	CollisionListener();
	void SetPlayerBody(b2Body* body);
	void clear();

	void BeginContact(b2Contact* contact);
	void EndContact(b2Contact* contact);

	// This is bigger than 0 if the player is on the ground. We can use it to check if the player can jump or not by checking if there are any ground contacts
	int playerGroundContacts = 0;
	// Same thing as above, but for the things that kill the player
	int playerDangerContacts = 0;
	// For ladders
	int playerLadderContacts = 0;
	// If the player is at the end of the level
	int playerFinishPointContacts = 0;

	// For moving platforms
	set<b2Body*> movingPlatforms;

	// This will hold all of the entities that the player is standing on. It is needed because we want to apply impulses to the entities when the player moves.
	// This will also hold the ground fixtures but they won't react to forces anyway
	set<b2Fixture*> entityFixturesUnderfoot;

	// This map will be passed to the level for use in deciding if a platform should be moved or not. The first element is the ID of the platform that
	// the button controls. The second element is the number of contacts currently on that button.
	unordered_map<int, int> buttons;

private:
	b2Body* playerBody;
};

class Box2dDraw : public b2Draw {
public:
	Box2dDraw();
	Box2dDraw(SDL_Renderer* ren, int screenHeight);
	void updateCameraOffset(float x, float y);

	void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
	void DrawCircle(const b2Vec2& center, float radius, const b2Color& color);
	void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color);
	void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DrawTransform(const b2Transform& xf);
	void DrawPoint(const b2Vec2& p, float size, const b2Color& color);

private:
	SDL_Renderer* renderer;
	int SCREEN_HEIGHT;
	float camXOffset;
	float camYOffset;

	void renderCircle(const b2Vec2& center, float radius, unsigned int sides);
};