#pragma once

#include <SDL_mixer.h>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>

using namespace std;

class AudioHandler
{
public:
	AudioHandler();
	~AudioHandler();

	bool loadMusic();

	void playMusic(int musicType);
	void checkForTrackEnd();

	void mute();
	void unmute(int musicType, bool paused);

	const int MAIN_MENU = 0;
	const int GAME = 1;

private:
	int currentMusicType;
	int currentMusicIndex;

	// The int is the number corresponding to the current music type, and its element is a vector holding all of the tracks for that music type
	unordered_map<int, vector<Mix_Music*>> musicMap;
};