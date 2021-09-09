#ifndef GAME_INFO
#define GAME_INFO
class Context;
struct GameInfo {
	const char* title;
	const char* description;
	const uint16_t* icon;
	Context* (*launch)(Display& display);
};
#endif

#include <Arduino.h>
#include <Display/Display.h>
#include <SPIFFS.h>
#include "bitmaps/invaderz_icon.hpp"
#include "src/SpaceInvaders.h"

const GameInfo InvaderzInfo {
		"Invaderz",
		"A remake of the classic space invaders.",
		[]() -> fs::File { return SPIFFS.open("/MainMenu/invaderz.raw"); },
		[](Display& display) -> Context* { return new SpaceInvaders::SpaceInvaders(display); }
};