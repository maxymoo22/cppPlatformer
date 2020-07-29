#include "AudioHandler.h"

AudioHandler::AudioHandler() {
	currentMusicType = MAIN_MENU;
	currentMusicIndex = 0;
}

AudioHandler::~AudioHandler() {
	// Looping through every music pointer of every screen/music type that we loaded
	for (int musicType = 0; musicType < 2; musicType++) {
		for (int i = 0; i < musicMap[musicType].size(); i++) {
			// We need to free the memory bcs otherwise mem leak :(
			Mix_FreeMusic(musicMap[musicType][i]);
		}
	}
}

bool AudioHandler::loadMusic() {
	unordered_map<int, vector<string>> musicFilenames = { {0, { "resources/sounds/menus.mp3", "resources/sounds/ex1.mp3" }}, {1, { "resources/sounds/background.mp3" }} };

	// Looping through every filename of every screen/music type
	for (int musicType = 0; musicType < 2; musicType++) {
		for (int i = 0; i < musicFilenames[musicType].size(); i++) {
			// And loading it
			Mix_Music* musicPtr = Mix_LoadMUS(musicFilenames[musicType][i].c_str());
			if (musicPtr == NULL) return false;

			// Now we can add it to the map for later use
			musicMap[musicType].push_back(musicPtr);
		}
	}

	return true;
}

void AudioHandler::playMusic(int musicType) {
	// Play music can be called to change the music. For example, user clicks the play button and this is called to change the music from menu to background
	// Thats why we need to compare the supplied music type, because if it is different then the game is changing states.
	// This means we need to restart the the audio loop for the current state.
	if (musicType != currentMusicType)
		currentMusicIndex = 0;
	currentMusicType = musicType;

	// Mix_PlayMusic handily halts any previously playing music for us
	Mix_PlayMusic(musicMap[currentMusicType][currentMusicIndex], 1);
}

void AudioHandler::checkForTrackEnd() {
	// The only time this function will detect a stop in the music is when the track ends and should move on to the next one
	if (!Mix_PlayingMusic()) {
		// Go to the next track, but make sure we don't go to far
		currentMusicIndex++;
		if (currentMusicIndex == musicMap[currentMusicType].size())
			currentMusicIndex = 0;

		// Start playing this new music
		playMusic(currentMusicType);
	}
}

void AudioHandler::mute() {
	currentMusicIndex = 0;
	Mix_HaltMusic();
}

void AudioHandler::unmute(int musicType, bool paused) {
	currentMusicType = musicType;
	playMusic(currentMusicType);

	// If the pause menu is open then we want to pause the music
	if(paused)
		Mix_PauseMusic();
}