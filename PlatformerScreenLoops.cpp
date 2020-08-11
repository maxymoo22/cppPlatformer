#include "Platformer.h"

// The main loop for the screen that users see when they first start the game
void Platformer::menuScreenLoop(bool pendingMouseEvent) {
	if (!muted)
		audioHandler.checkForTrackEnd();

	fontHandler->renderFont("heading_font", "Platformer", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4);

	// A rectangle we can use for rendering
	SDL_Rect rectangle;

	//Get mouse position
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);
	vector<Button> buttons = { Button("Play", SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 - 25, 160, 50),
							   Button("Select\nlevel", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 25, 160, 50),
							   Button("Exit to\ndesktop", SCREEN_WIDTH / 2 + 120, SCREEN_HEIGHT / 2 - 25, 160, 50),
							   Button("Your\nmission", SCREEN_WIDTH / 2 - 280, SCREEN_HEIGHT / 2 + 65, 160, 50),
							   Button(muted ? "Unmute" : "Mute", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 65, 160, 50),
							   Button("Multiplayer", SCREEN_WIDTH / 2 + 120, SCREEN_HEIGHT / 2 + 65, 160, 50),
							   Button("Credits", SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 155, 160, 50) };

	// If it's true, then we need to draw the popup. Users can confirm they want to exit and that they didn't press it by accident
	if (displayAreYouSure == true) {
		// Popup body
		rectangle = {SCREEN_WIDTH / 2 - 225, SCREEN_HEIGHT / 4 - 90, 450, 180};
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rectangle);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		fontHandler->renderFont("popup_font", "Are you sure?", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 45);

		// Add the yes and no buttons to the vector
		buttons.push_back(Button("Yes", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 4 + 15, 160, 50));
		buttons.push_back(Button("No", SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 4 + 15, 160, 50));
	}

	for (Button button : buttons) {
		rectangle = { button.x, button.y, button.width, button.height };
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		if (isPointInButton(x, y, button)) {
			if (pendingMouseEvent == true) {
				// Now we need to loop through all of the actions. UGGGGGGGH
				if (button.text == "Yes")
					yesButton();
				else if (button.text == "No")
					noButton();

				else if (!displayAreYouSure) {
					if (button.text == "Play")
						playButton();
					else if (button.text == "Exit to\ndesktop")
						exitButton();
					else if (button.text == "Credits")
						currentScreenType = screenTypes::CREDITS;
					else if (button.text == "Your\nmission")
						currentScreenType = screenTypes::INSTRUCTIONS;
					else if (button.text == "Mute")
						muteButton();
					else if (button.text == "Unmute")
						unmuteButton();
					else if (button.text == "Select\nlevel")
						levelSelectButton();
				}
			}

			// If the mouse button is down then we want to highlight the button
			else if ((mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) && !(button.text != "Yes" && button.text != "No" && displayAreYouSure == true)) {
				SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
				// Adjust it so the highlighitng is a bit smaller than the button outline
				rectangle.x += 5; rectangle.y += 5; rectangle.h -= 10; rectangle.w -= 10;
				SDL_RenderFillRect(renderer, &rectangle);
			}
		}

		fontHandler->renderFont("button_font", button.text, button.x + button.width / 2, button.y + button.height / 2);
	}

	// Show everything on the screen
	SDL_RenderPresent(renderer);
}

// The main loop for the screen where users actually play the game
void Platformer::gameScreenLoop(bool pendingMouseEvent, bool pendingKeyEvent) {
	if(!muted)
		audioHandler.checkForTrackEnd();

	const Uint8* keyStates = SDL_GetKeyboardState(NULL);

	// The escape key pauses and resumes the game
	if (pendingKeyEvent && keyStates[SDL_SCANCODE_ESCAPE] && !displayAreYouSure) {
		if(paused)
			Mix_ResumeMusic();
		else
			Mix_PauseMusic();

		paused = !paused;
	}

	//## ---- DRAWING CODE ---- ##\\

	// Draw the level
	maps[currentLevel].render(camXOffset, camYOffset);
	
	if (!playerDead) {
		b2Vec2 playerPosVector = playerBody->GetPosition();
		//  We need to take the camera offset into account
		SDL_Rect playerRect = { playerPosVector.x * 32 - 16 - camXOffset, SCREEN_HEIGHT - (playerPosVector.y * 32 + 16) - camYOffset, 32, 32 };
		SDL_Rect sourceRect = { playerTextureXOffset, 0, 32, 32 };
		// Draw the player sprite at its position.
		SDL_RenderCopyEx(renderer, player, &sourceRect, &playerRect, 0, NULL, (playerDirection) ? SDL_RendererFlip::SDL_FLIP_NONE : SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
	}
	// If the player is dead then we want to render the death particles
	else {
		for (auto& deathParticle : deathParticles) {
			b2Vec2 p = deathParticle.body->GetPosition();
			SDL_Rect sourceRect = {(deathParticle.colorIndex % 4) * 8, (deathParticle.colorIndex % 4) * 8, 8, 8};
			SDL_Rect destRect = {p.x * 32 - 4 - camXOffset, SCREEN_HEIGHT - (p.y * 32 + 4) - camYOffset, 8, 8};

			SDL_RenderCopyEx(renderer, particleTexture, &sourceRect, &destRect, (double)deathParticle.body->GetAngle() * -180.0 / b2_pi, NULL, SDL_FLIP_NONE);
		}
	}

	if (debugDrawHitboxes == true) {
		// Draw the box2d stuff for debugging
		debugDrawer.updateCameraOffset(camXOffset, camYOffset);
		physicsWorld->DebugDraw();
	}

	vector<Button> buttons;
	SDL_Rect rectangle;

	// If it's true, then we need to draw the popup. Users can confirm they want to exit or restart and that they didn't press the button by accident
	if (displayAreYouSure == true) {
		// Popup body
		rectangle = { SCREEN_WIDTH / 2 - 225, SCREEN_HEIGHT / 4 - 90, 450, 180 };
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rectangle);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		fontHandler->renderFont("popup_font", "Are you sure?", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 45);

		// Add the yes and no buttons to the vector
		buttons = { Button("Yes", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 4 + 15, 160, 50),
					Button("No", SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 4 + 15, 160, 50) };
	}

	else if (paused == true) {
		rectangle = { SCREEN_WIDTH / 2 - 225, SCREEN_HEIGHT / 4 - 95, 450, 250 };

		// The popup body
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rectangle);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		fontHandler->renderFont("popup_font", "Paused", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 40);

		buttons = { Button("Resume", SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 4 + 5, 160, 50),
					Button("Restart\nlevel", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 4 + 5, 160, 50),
					Button("Main menu", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 4 + 75, 160, 50), 
					Button(muted ? "Unmute" : "Mute", SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 4 + 75, 160, 50) };
	}

	else if (playerDead == true) {
		rectangle = {SCREEN_WIDTH / 2 - 225, SCREEN_HEIGHT / 4 - 90, 450, 180};

		// The popup body
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderFillRect(renderer, &rectangle);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		fontHandler->renderFont("popup_font", "You dieieieid!", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 - 45);

		buttons = { Button("Respawn", SCREEN_WIDTH / 2 - 170, SCREEN_HEIGHT / 4 + 15, 160, 50),
					Button("Main menu", SCREEN_WIDTH / 2 + 10, SCREEN_HEIGHT / 4 + 15, 160, 50) };
	}

	// Mouse position
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);

	for (auto& button : buttons) {
		rectangle = { button.x, button.y, button.width, button.height };
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &rectangle);

		if (isPointInButton(x, y, button)) {
			if (pendingMouseEvent == true) {
				if (button.text == "Resume")
					resumeButton();
				else if (button.text == "Restart\nlevel")
					restartLevelButton();
				else if (button.text == "Main menu") {
					displayAreYouSure_Reason = 2;

					// If the player is dead then we dont need to display the are you sure popup
					if (playerDead == true)
						yesButton();
					else
						displayAreYouSure = true;
				}
				else if (button.text == "Mute")
					muteButton();
				else if (button.text == "Unmute")
					unmuteButton();
				else if (button.text == "Yes")
					yesButton();
				else if (button.text == "No")
					noButton();
				else if (button.text == "Respawn")
					respawnButton();
			}

			// If the mouse button is down then we want to highlight the button
			else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
				SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
				// Adjust it so the highlighitng is a bit smaller than the button outline
				rectangle.x += 5; rectangle.y += 5; rectangle.h -= 10; rectangle.w -= 10;
				SDL_RenderFillRect(renderer, &rectangle);
			}
		}

		fontHandler->renderFont("button_font", button.text, button.x + button.width / 2, button.y + button.height / 2);
	}

	// Render everything to the screen
	SDL_RenderPresent(renderer);

	//##------------------------##//


	if (playerDead) {
		// Step the physics forwards
		physicsWorld->Step(1.0 / 80.0, 8, 3);
		return;
	}

	// We only want to draw if the game isn't displaying any popups
	if (paused == true || displayAreYouSure == true) return;

	b2Vec2 velocity = playerBody->GetLinearVelocity();

	// If the player is on a ladder we can turn off gravity
	if (collisionListener->playerLadderContacts > 0) {
		if ((keyStates[SDL_SCANCODE_SPACE] || keyStates[SDL_SCANCODE_UP]))
			playerBody->ApplyLinearImpulseToCenter(b2Vec2(0, 5 - velocity.y), true);
		else if (keyStates[SDL_SCANCODE_DOWN])
			playerBody->ApplyLinearImpulseToCenter(b2Vec2(0, -5 - velocity.y), true);
		else
			playerBody->ApplyLinearImpulseToCenter(b2Vec2(0, -velocity.y), true);
	}

	// If the player pressed space then we can apply a jump impulse, but only if they are on the gorund.
	// We also need to make sure that enough time has passed to stop the player spam jumping.
	else if ((keyStates[SDL_SCANCODE_SPACE] || keyStates[SDL_SCANCODE_UP]) && collisionListener->playerGroundContacts > 0 && SDL_GetTicks() - playerJumpStartTime >= 200) {
		playerBody->ApplyLinearImpulseToCenter(b2Vec2(0.0, 10.0), true);
		playerJumpStartTime = SDL_GetTicks();
	}

	// This stuff is for moving the player
	b2Vec2 leftRightImpulse;
	if (keyStates[SDL_SCANCODE_LEFT] && !keyStates[SDL_SCANCODE_RIGHT]) {
		leftRightImpulse = b2Vec2(-5 - velocity.x, 0);
		playerDirection = 0;
	}
	else if (keyStates[SDL_SCANCODE_RIGHT] && !keyStates[SDL_SCANCODE_LEFT]) {
		leftRightImpulse = b2Vec2(5 - velocity.x, 0);
		playerDirection = 1;
	}
	else
		leftRightImpulse = b2Vec2(-velocity.x, 0);
	playerBody->ApplyLinearImpulseToCenter(leftRightImpulse, true);

	// Once we have moved the player, we can move any entities under the player based on how the player moved
	b2Vec2 locationOfImpulseInWorldCoords = playerBody->GetWorldPoint(b2Vec2(0, -0.6));
	for (auto& underfootFixture : collisionListener->entityFixturesUnderfoot)
		underfootFixture->GetBody()->ApplyLinearImpulse(b2Vec2(-1 * leftRightImpulse.x / 13, 0), locationOfImpulseInWorldCoords, true);

	// Update the moving platforms
	maps[currentLevel].doMovingPlatformLogic(collisionListener->buttons);

	// This is to stop the player sliding off of a moving platform
	if (collisionListener->movingPlatforms.size() > 0)
		// If they are on the platform, we want to get the platforms velocity and add it to the player's so that it moves relative to the platform
		playerBody->ApplyLinearImpulseToCenter(b2Vec2((*collisionListener->movingPlatforms.begin())->GetLinearVelocity().x, 0), true);

	if (keyStates[SDL_SCANCODE_RETURN] && currentLevel == 0 && collisionListener->playerFinishPointContacts > 0) {
		// We need to delete all of the physics for the level and switch to the new level
		currentLevel = collisionListener->levelEntranceNum;
		writeUserData();
		createPhysics();
		maps[currentLevel].createHitboxes(physicsWorld);
	}

	// Step the physics forwards
	physicsWorld->Step(1.0 / 80.0, 8, 3);

	// Check if any map scrolling is needed
	checkScrolling();

	updatePlayerAnimation(keyStates[SDL_SCANCODE_LEFT] != keyStates[SDL_SCANCODE_RIGHT], keyStates[SDL_SCANCODE_UP] != keyStates[SDL_SCANCODE_DOWN]);

	if (collisionListener->playerDangerContacts > 0 || playerBody->GetPosition().y < -40) {
		playerDead = true;
		Mix_PauseMusic();
		Mix_RewindMusic();

		// Dont need particles if the player fell below the map
		if (playerBody->GetPosition().y < -40) return;

		b2Vec2 p = playerBody->GetPosition();
		// We don't need the player anymore
		physicsWorld->DestroyBody(playerBody);
		collisionListener->nullPlayerBody();
		playerBody = NULL;

		srand(SDL_GetTicks());
		for (int i = 0; i < 20; i++) {
			b2BodyDef particleDef;
			particleDef.type = b2_dynamicBody;
			particleDef.position.Set(p.x, p.y);
			b2Body* particleBody = physicsWorld->CreateBody(&particleDef);

			DeathParticle particle = {particleBody, rand() % 16};
			deathParticles.push_back(particle);

			b2PolygonShape particleShape;
			// The hitbox should be a bit smaller than the actual rendered particle since box2d collides with polygon skins instead of the polygon surface
			particleShape.SetAsBox(0.1, 0.1);
			particleBody->CreateFixture(&particleShape, 1.0);

			particleBody->ApplyLinearImpulseToCenter(b2Vec2((rand() % 2) / 1.9, (rand() % 2) / 1.9), true);
		}
	}

	// If the player has reached the end then (obviously) we need to go to the next level
	else if (collisionListener->playerFinishPointContacts > 0 && currentLevel > 0) {
		currentLevel++;
		if (currentLevel == 4)
			currentLevel = 1;

		writeUserData();

		camXOffset = 0;
		camYOffset = 0;

		// We need to delete all of the physics for the level and switch to the new level
		createPhysics();
		maps[currentLevel].createHitboxes(physicsWorld);
	}
}

// CREDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDits
void Platformer::creditsScreenLoop(bool pendingMouseEvent) {
	if (!muted)
		audioHandler.checkForTrackEnd();

	fontHandler->renderFont("heading_font", "CREDITS", SCREEN_WIDTH / 2, 100);
	fontHandler->renderFont("button_font", "Maps:\nOlieboi\n\nMusic:\nRain kelly-austin", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 + 50);

	SDL_Rect rectangle = {20, SCREEN_HEIGHT - 75, 160, 50};
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &rectangle);

	//Get mouse position
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);
	if (x > 20 && x < 180 && y > SCREEN_HEIGHT - 75 && y < SCREEN_HEIGHT - 25) {
		if (pendingMouseEvent == true)
			currentScreenType = screenTypes::MAIN_MENU;

		// If the mouse button is down then we want to highlight the button
		else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
			// Adjust it so the highlighitng is a bit smaller than the button outline
			rectangle.x += 5; rectangle.y += 5; rectangle.h -= 10; rectangle.w -= 10;
			SDL_RenderFillRect(renderer, &rectangle);
		}
	}

	fontHandler->renderFont("button_font", "Main menu", 100, SCREEN_HEIGHT - 50);
	SDL_RenderPresent(renderer);
}

// How to play
void Platformer::instructionsScreenLoop(bool pendingMouseEvent) {
	if (!muted)
		audioHandler.checkForTrackEnd();

	fontHandler->renderFont("heading_font", "how to play", SCREEN_WIDTH / 2, 50);
	fontHandler->renderFont("button_font", "Use either the arrow keys or w/a/s/d to move.\n\
W or UP:     jump/climb up ladder\n\
A or LEFT:              move left\n\
S or DOWN:      climb down ladder\n\
D or RIGHT:            move right\n\
Esc:                 pause/resume\n\
The space key can also be used for jumping and climbing\n\n\n\n\
Dont touch the lava:\n\
or the squish monsters:\n\n\n\n\
When you go through one of these\n\
you will be transported to the next level.\n\n\
good luck!", SCREEN_WIDTH / 2, 350);

	SDL_Rect rectangle = { 20, SCREEN_HEIGHT - 75, 160, 50 };
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderDrawRect(renderer, &rectangle);

	//Get mouse position
	int x, y;
	Uint32 mouseState = SDL_GetMouseState(&x, &y);
	if (x > 20 && x < 180 && y > SCREEN_HEIGHT - 75 && y < SCREEN_HEIGHT - 25) {
		if (pendingMouseEvent == true)
			currentScreenType = screenTypes::MAIN_MENU;

		// If the mouse button is down then we want to highlight the button
		else if (mouseState & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
			// Adjust it so the highlighitng is a bit smaller than the button outline
			rectangle.x += 5; rectangle.y += 5; rectangle.h -= 10; rectangle.w -= 10;
			SDL_RenderFillRect(renderer, &rectangle);
		}
	}

	fontHandler->renderFont("button_font", "Main menu", 100, SCREEN_HEIGHT - 50);

	// These lines render a 64x64 block of lava next to the 'dont touch the lava' line
	SDL_Rect sourceRectangle = {64, 0, 32, 64};
	rectangle = {SCREEN_WIDTH / 2 + 160, 315, 32, 64};
	SDL_RenderCopy(renderer, menuSprites, &sourceRectangle, &rectangle);
	rectangle.x += 32;
	SDL_RenderCopy(renderer, menuSprites, &sourceRectangle, &rectangle);

	// This renders the squish monster
	sourceRectangle = {0, 64, 32, 32};
	rectangle = { SCREEN_WIDTH / 2 + 180, 370, 32, 32 };
	SDL_RenderCopy(renderer, menuSprites, &sourceRectangle, &rectangle);

	// Finally, this renders the level end portal
	sourceRectangle = {0, 0, 64, 64};
	rectangle = { SCREEN_WIDTH / 2 + 250, 430, 64, 64 };
	SDL_RenderCopy(renderer, menuSprites, &sourceRectangle, &rectangle);

	SDL_RenderPresent(renderer);
}