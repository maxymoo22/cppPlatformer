#include "Platformer.h"

#define EXIT_GAME 1
#define GO_TO_MAIN_MENU 2
#define RESTART_LEVEL 3

void Platformer::playButton() {
	currentScreenType = screenTypes::GAME;
	if (!muted)
		audioHandler.playMusic(audioHandler.GAME);
}

void Platformer::resumeButton() {
	paused = false;
	Mix_ResumeMusic();
}

void Platformer::exitButton() {
	displayAreYouSure = true;
	displayAreYouSure_Reason = EXIT_GAME;
}

void Platformer::yesButton() {
	if (displayAreYouSure_Reason == EXIT_GAME) {
		quit = true;
		return;
	}

	// Otherwise the user will be going to the main menu or restarting the level, so we can reset stuff
	paused = false;
	displayAreYouSure = false;
	playerDead = false;

	// Maybe need this?? idk its only 2 lines so whatever
	camXOffset = 0;
	camYOffset = 0;

	Mix_RewindMusic();
	Mix_ResumeMusic();

	// Finally, if the player is going to the main menu we can switch the screen and music type
	if (displayAreYouSure_Reason == GO_TO_MAIN_MENU) {
		currentScreenType = screenTypes::MAIN_MENU;
		// Set the current level back to what it was before we entered level selection. This only needs to be done if we are coming back from level selection
		if(currentLevel == 0)
			currentLevel = naturalLevel;

		if (!muted)
			audioHandler.playMusic(audioHandler.MAIN_MENU);
	}

	// This will clear the physics for this world and setup the new stuff for next time. Even though we aren't switching level we still do this bcs it resets everything safely
	createPhysics();
	maps[currentLevel].createHitboxes(physicsWorld);
}

void Platformer::noButton() {
	displayAreYouSure = false;
}

void Platformer::mainMenuButton() {
	displayAreYouSure = true;
}

void Platformer::restartLevelButton() {
	displayAreYouSure = true;
	displayAreYouSure_Reason = RESTART_LEVEL;
}

void Platformer::muteButton() {
	muted = true;
	audioHandler.mute();
	writeUserData();
}

void Platformer::unmuteButton() {
	muted = false;
	audioHandler.unmute((int)currentScreenType, paused);
	writeUserData();
}

void Platformer::respawnButton() {
	playerDead = false;
	// This will clear the physics for this world and setup the new stuff for next time. Even though we aren't switching level we still do this bcs it resets everything safely
	createPhysics();
	maps[currentLevel].createHitboxes(physicsWorld);
	Mix_ResumeMusic();
}

void Platformer::levelSelectButton() {
	currentScreenType = screenTypes::GAME;
	naturalLevel = currentLevel;
	currentLevel = 0;
	createPhysics();
	maps[currentLevel].createHitboxes(physicsWorld);
	if (!muted)
		audioHandler.playMusic(audioHandler.GAME);
}