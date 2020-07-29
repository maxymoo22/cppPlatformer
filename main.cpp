#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC

#include "Platformer.h"

int main(int argc, char* args[]) {
	Platformer platformer;

	bool result = platformer.init();
	if (result == false) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Couldn't initialize the game. Please check the logs for more info", platformer.window);
		return 0;
	}

	result = platformer.loadAssets();
	if (result == false) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Couldn't load the game. Please check the logs for more info", platformer.window);
		return 0;
	}

	platformer.loop();

	_CrtDumpMemoryLeaks();

	return 0;
}