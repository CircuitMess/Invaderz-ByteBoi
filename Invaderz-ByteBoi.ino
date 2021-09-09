/*
	Invaderz
	Copyright (C) 2020 CircuitMess
	original game:
	Invaders by Yoda Zhang
	http://www.yodasvideoarcade.com/images/gamebuino-invaders.zip
	Ported to MAKERphone by CircuitMess, then ported to ByteBoy.
*/
//----------------------------------------------------------------------------
// define variables && constants
//----------------------------------------------------------------------------    
#include <Arduino.h>
#include "src/SpaceInvaders.h"
#include <CircuitOS.h>
#include <Input/I2cExpander.h>
#include <Input/InputI2C.h>
#include <Loop/LoopManager.h>
#include <SPIFFS.h>
#include <ByteBoi.h>

//#include "Nibble.hpp"
#include <Audio/Piezo.h>

SpaceInvaders::SpaceInvaders* game;

void setup() {
	Serial.begin(115200);
	ByteBoi.begin();
	ByteBoi.getDisplay()->commit();
	LoopManager::addListener(Input::getInstance());
	game=new SpaceInvaders::SpaceInvaders(ByteBoi.getDisplay());
	LoopManager::addListener(game);
	game->unpack();
	game->start();
}

void loop() {
	LoopManager::loop();
	yield();
}